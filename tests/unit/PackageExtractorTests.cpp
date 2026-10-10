#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#ifdef _WIN32
#include <thread>
#endif

#include <miniz.h>

#include "shiplua/host/ModHost.h"
#include "shiplua/manifest/PackageExtractor.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

struct Entry {
    std::string name;
    std::string contents;
};

bool WritePackage(const std::filesystem::path& path, const std::vector<Entry>& entries) {
    mz_zip_archive archive{};
    if (!mz_zip_writer_init_file(&archive, path.string().c_str(), 0)) {
        return false;
    }
    bool ok = true;
    for (const auto& entry : entries) {
        if (!mz_zip_writer_add_mem(&archive, entry.name.c_str(), entry.contents.data(),
                                   entry.contents.size(), MZ_BEST_COMPRESSION)) {
            ok = false;
            break;
        }
    }
    ok = mz_zip_writer_finalize_archive(&archive) != 0 && ok;
    mz_zip_writer_end(&archive);
    return ok;
}

std::vector<Entry> ValidEntries() {
    return {
        {"manifest.toml",
         "id = \"example.package\"\nname = \"Package\"\nversion = \"0.1.0\"\n"
         "api = \">=0.1 <0.2\"\nentrypoint = \"scripts/main.lua\"\n"},
        {"scripts/main.lua", "package_loaded = true\n"},
        {"assets/", ""},
    };
}

void MarkFirstEntryAsSymlink(const std::filesystem::path& path) {
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
    for (std::size_t i = 0; i + 46 <= bytes.size(); ++i) {
        if (bytes[i] == 0x50 && bytes[i + 1] == 0x4b && bytes[i + 2] == 0x01 &&
            bytes[i + 3] == 0x02) {
            bytes[i + 5] = 3; // Unix creator system.
            const std::uint32_t attributes = 0120777U << 16U;
            for (int byte = 0; byte < 4; ++byte) {
                bytes[i + 38 + byte] = static_cast<unsigned char>(attributes >> (byte * 8));
            }
            file.clear();
            file.seekp(0);
            file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            return;
        }
    }
}

void ReplaceBytes(const std::filesystem::path& path, const std::string& from,
                  const std::string& to) {
    if (from.size() != to.size()) {
        return;
    }
    std::ifstream input(path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
    for (std::size_t i = 0; i + from.size() <= bytes.size(); ++i) {
        if (std::equal(from.begin(), from.end(), bytes.begin() + i)) {
            std::copy(to.begin(), to.end(), bytes.begin() + i);
            i += from.size() - 1;
        }
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

void TestValidExtractionAndHostLifecycle(const std::filesystem::path& root) {
    const auto package = root / "valid.shipmod";
    const auto extracted = root / "extracted";
    Check(WritePackage(package, ValidEntries()), "valid package fixture should be created");
    auto result = ShipLua::ExtractShipmod(package, extracted);
    Check(result.isOk(), "valid package should extract: " + result.message);
    Check(std::filesystem::is_regular_file(extracted / "manifest.toml"),
          "manifest should be extracted");
    Check(std::filesystem::is_regular_file(extracted / "scripts" / "main.lua"),
          "nested entrypoint should be extracted");
    Check(std::filesystem::is_directory(extracted / "assets"),
          "explicit ZIP directory should be extracted");

    const auto cache = root / "cache";
    ShipLua::ModHost host;
    auto loaded = host.LoadModFromPackage(package.string(), cache.string());
    Check(loaded.isOk(), "ModHost should load a package: " + loaded.message);
    Check(host.IsLoaded("example.package"), "package manifest id should be registered");
    Check(host.UnloadMod("example.package").isOk(), "package mod should unload");
    std::size_t cacheChildren = 0;
    if (std::filesystem::exists(cache)) {
        cacheChildren = static_cast<std::size_t>(
            std::distance(std::filesystem::directory_iterator(cache),
                          std::filesystem::directory_iterator()));
    }
    Check(cacheChildren == 0, "unload should remove the owned extraction directory");
}

void TestUnsafePathsAreTransactional(const std::filesystem::path& root) {
    const std::vector<std::string> unsafeNames = {"../escape.lua", "/absolute.lua", "C:/drive.lua"};
    for (std::size_t i = 0; i < unsafeNames.size(); ++i) {
        const auto package = root / ("unsafe-" + std::to_string(i) + ".shipmod");
        const auto destination = root / ("unsafe-output-" + std::to_string(i));
        const std::string archiveName =
            unsafeNames[i].front() == '/' ? "x" + unsafeNames[i].substr(1) : unsafeNames[i];
        Check(WritePackage(package, {{archiveName, "bad"}}),
              "unsafe fixture should be created: " + unsafeNames[i]);
        if (archiveName != unsafeNames[i]) {
            ReplaceBytes(package, archiveName, unsafeNames[i]);
        }
        const auto result = ShipLua::ExtractShipmod(package, destination);
        Check(!result.isOk() && result.code == ShipLua::ErrorCode::PermissionDenied,
              "unsafe package path should be rejected: " + unsafeNames[i] + " (" + result.message + ")");
        Check(!std::filesystem::exists(destination), "failed extraction must not create destination");
    }
    Check(!std::filesystem::exists(root.parent_path() / "escape.lua"),
          "traversal must not write outside extraction root");
}

void TestSymlinkAndLimits(const std::filesystem::path& root) {
    const auto symlinkPackage = root / "symlink.shipmod";
    Check(WritePackage(symlinkPackage, {{"link", "target"}}), "symlink fixture should be created");
    MarkFirstEntryAsSymlink(symlinkPackage);
    const auto symlinkResult = ShipLua::ExtractShipmod(symlinkPackage, root / "symlink-output");
    Check(!symlinkResult.isOk() && symlinkResult.code == ShipLua::ErrorCode::PermissionDenied,
          "ZIP symlink should be rejected");

    const auto largePackage = root / "large.shipmod";
    Check(WritePackage(largePackage, {{"large.bin", std::string(64, 'x')}}),
          "large fixture should be created");
    ShipLua::PackageLimits limits;
    limits.maxFileBytes = 32;
    limits.maxTotalBytes = 32;
    const auto limitResult = ShipLua::ExtractShipmod(largePackage, root / "large-output", limits);
    Check(!limitResult.isOk() && limitResult.code == ShipLua::ErrorCode::ResourceLimit,
          "oversized entry should be rejected before extraction");
}

void TestAmbiguousPaths(const std::filesystem::path& root) {
    const auto collisionPackage = root / "collision.shipmod";
    Check(WritePackage(collisionPackage, {{"Data.lua", "a"}, {"data.lua", "b"}}),
          "case-collision fixture should be created");
    const auto collisionResult =
        ShipLua::ExtractShipmod(collisionPackage, root / "collision-output");
    Check(!collisionResult.isOk() && collisionResult.code == ShipLua::ErrorCode::InvalidArgument,
          "portable case collision should be rejected");

    const auto hierarchyPackage = root / "hierarchy.shipmod";
    Check(WritePackage(hierarchyPackage, {{"scripts", "file"}, {"scripts/main.lua", "nested"}}),
          "hierarchy-collision fixture should be created");
    const auto hierarchyResult =
        ShipLua::ExtractShipmod(hierarchyPackage, root / "hierarchy-output");
    Check(!hierarchyResult.isOk() && hierarchyResult.code == ShipLua::ErrorCode::InvalidArgument,
          "file/directory hierarchy collision should be rejected");
}

void TestHostFailureCleansExtraction(const std::filesystem::path& root) {
    const auto package = root / "broken-mod.shipmod";
    const auto cache = root / "broken-cache";
    Check(WritePackage(package,
                       {{"manifest.toml",
                         "id = \"example.broken\"\nname = \"Broken\"\nversion = \"0.1.0\"\n"
                         "api = \">=0.1 <0.2\"\nentrypoint = \"main.lua\"\n"},
                        {"main.lua", "error('expected failure')\n"}}),
          "broken package fixture should be created");

    ShipLua::ModHost host;
    const auto result = host.LoadModFromPackage(package.string(), cache.string());
    Check(!result.isOk() && result.code == ShipLua::ErrorCode::ScriptFailure,
          "script failure should reject package load");
    Check(host.Count() == 0, "failed package must not remain registered");
    std::size_t cacheChildren = 0;
    if (std::filesystem::exists(cache)) {
        cacheChildren = static_cast<std::size_t>(
            std::distance(std::filesystem::directory_iterator(cache),
                          std::filesystem::directory_iterator()));
    }
    Check(cacheChildren == 0, "failed package load must remove extracted staging data");
}

#ifdef _WIN32
// An antivirus scan holding a freshly extracted DLL made the commit fail with "access
// denied" and the mod was rejected at boot. The commit must wait for the file instead.
void TestCommitWaitsForBusyFile(const std::filesystem::path& root) {
    const auto staging = root / "busy-staging";
    const auto destination = root / "busy-output";
    std::filesystem::create_directories(staging);
    { std::ofstream(staging / "provider.dll") << "dll"; }
    std::ifstream holder(staging / "provider.dll");
    Check(holder.is_open(), "busy fixture should open its file");

    std::error_code probeError;
    std::filesystem::rename(staging, root / "busy-probe", probeError);
    if (!probeError) {
        std::filesystem::rename(root / "busy-probe", staging, probeError);
        holder.close();
        std::cout << "skip: this volume renames a directory that holds an open file\n";
        return;
    }

    std::thread release([&holder]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        holder.close();
    });
    const auto result = ShipLua::CommitExtractedDirectory(staging, destination);
    release.join();
    Check(result.isOk(), "commit should succeed once the file is released: " + result.message);
    Check(std::filesystem::is_regular_file(destination / "provider.dll"),
          "committed directory should hold the extracted file");
    Check(!std::filesystem::exists(staging), "staging directory should be gone after the commit");
}
#endif

void TestCommitKeepsExistingDestination(const std::filesystem::path& root) {
    const auto staging = root / "conflict-staging";
    const auto destination = root / "conflict-output";
    std::filesystem::create_directories(staging);
    std::filesystem::create_directories(destination / "existing");
    const auto started = std::chrono::steady_clock::now();
    const auto result = ShipLua::CommitExtractedDirectory(staging, destination);
    const auto elapsed = std::chrono::steady_clock::now() - started;
    Check(!result.isOk(), "an existing destination must not be replaced");
    Check(elapsed < std::chrono::milliseconds(1500),
          "an existing destination should fail without waiting for the retries");
    Check(std::filesystem::is_directory(destination / "existing"),
          "the existing destination must be preserved");
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() / "shiplua-package-tests";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    TestValidExtractionAndHostLifecycle(root);
    TestUnsafePathsAreTransactional(root);
    TestSymlinkAndLimits(root);
    TestAmbiguousPaths(root);
    TestHostFailureCleansExtraction(root);
#ifdef _WIN32
    TestCommitWaitsForBusyFile(root);
#endif
    TestCommitKeepsExistingDestination(root);

    std::filesystem::remove_all(root);
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All package extraction checks passed\n";
    return 0;
}
