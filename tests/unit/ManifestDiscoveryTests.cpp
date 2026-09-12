#include <miniz.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "shiplua/manifest/ManifestParser.h"
#include "shiplua/manifest/ModDiscovery.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void TestManifestFixtures() {
    const std::filesystem::path fixtures =
        std::filesystem::path(SHIPLUA_TEST_SOURCE_DIR) / "fixtures" / "manifest";

    Check(ShipLua::ParseManifestFile((fixtures / "valid-minimal.toml").string()).isOk(),
          "minimal fixture should be valid");
    Check(ShipLua::ParseManifestFile((fixtures / "valid-complete.toml").string()).isOk(),
          "complete fixture should be valid");

    const auto missing =
        ShipLua::ParseManifestFile((fixtures / "invalid-missing-name.toml").string());
    Check(!missing.isOk() && missing.message.find("name") != std::string::npos,
          "missing-name fixture should fail with a readable field name");

    const auto syntax = ShipLua::ParseManifestFile((fixtures / "invalid-syntax.toml").string());
    Check(!syntax.isOk() && syntax.message.find("line") != std::string::npos,
          "syntax fixture should fail with a source location");

    const auto version = ShipLua::ParseManifestFile((fixtures / "invalid-version.toml").string());
    Check(!version.isOk() && version.message.find("version") != std::string::npos,
          "invalid-version fixture should fail with a readable field name");
}

void TestManifestVersionRanges() {
    const std::string prefix =
        "id = \"example.ranges\"\nname = \"Ranges\"\nversion = \"1.0.0\"\n"
        "entrypoint = \"main.lua\"\n";

    const auto invalidApi = ShipLua::ParseManifestString(prefix + "api = \"^1.0\"\n");
    Check(!invalidApi.isOk() && invalidApi.message.find("api") != std::string::npos,
          "invalid API range should name its manifest field");

    const auto invalidHost = ShipLua::ParseManifestString(
        prefix + "api = \">=0.1 <1.0\"\n[host]\nshipwright = \">=bad\"\n");
    Check(!invalidHost.isOk() && invalidHost.message.find("host.shipwright") != std::string::npos,
          "invalid host range should name its manifest field");

    const auto invalidDependency = ShipLua::ParseManifestString(
        prefix + "api = \">=0.1 <1.0\"\n[dependencies]\n\"example.core\" = \"||\"\n");
    Check(!invalidDependency.isOk() && invalidDependency.message.find("example.core") != std::string::npos,
          "invalid dependency range should name its dependency");

    const auto emptyId = ShipLua::ParseManifestString(
        "id = \"\"\nname = \"Empty\"\nversion = \"1.0.0\"\n"
        "api = \">=0.1 <1.0\"\nentrypoint = \"main.lua\"\n");
    Check(!emptyId.isOk() && emptyId.message.find("id") != std::string::npos,
          "empty required field should be rejected");
}

void WriteZip(const std::filesystem::path& path, const char* entry) {
    mz_zip_archive archive{};
    Check(mz_zip_writer_init_file(&archive, path.string().c_str(), 0) != 0, "create ZIP");
    const bool directory = std::string(entry).back() == '/';
    Check(mz_zip_writer_add_mem(&archive, entry, directory ? nullptr : "invalid manifest",
                                directory ? 0 : 16, 0) != 0, "write ZIP");
    Check(mz_zip_writer_finalize_archive(&archive) != 0, "finalize ZIP");
    mz_zip_writer_end(&archive);
}

void TestDiscovery() {
    const auto root = std::filesystem::temp_directory_path() / "shiplua-discovery-test";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "b-directory-mod");
    std::filesystem::create_directories(root / "ignored-directory");
    {
        std::ofstream manifest(root / "b-directory-mod" / "manifest.toml");
        manifest << "id = \"example.directory\"\n";
        std::ofstream fake(root / "fake.shipmod", std::ios::binary);
        fake << "not a zip";
    }
    WriteZip(root / "a-package.shipmod", "manifest.toml");
    WriteZip(root / "c-package.zip", "manifest.toml");
    WriteZip(root / "d-package.ZIP", "manifest.toml");
    WriteZip(root / "ordinary.zip", "notes.txt");
    WriteZip(root / "nested.zip", "my-mod/manifest.toml");
    WriteZip(root / "wrong-case.zip", "Manifest.toml");
    WriteZip(root / "directory-manifest.zip", "manifest.toml/");
    WriteZip(root / "wrong-extension.rar", "manifest.toml");
    { std::ofstream truncated(root / "truncated.zip"); truncated << "PK\003\004"; }

    const auto result = ShipLua::DiscoverMods(root);
    Check(result.isOk(), "valid mods root should scan");
    if (result.isOk()) {
        Check(result.value->size() == 4, "only directory and root-manifest ZIPs should appear");
        if (result.value->size() == 4) {
            Check((*result.value)[0].kind == ShipLua::ModSourceKind::Package,
                  "results should be deterministically path-sorted");
            Check((*result.value)[1].kind == ShipLua::ModSourceKind::Directory,
                  "directory candidate should be identified");
            Check((*result.value)[2].path.filename() == "c-package.zip" &&
                  (*result.value)[3].path.filename() == "d-package.ZIP",
                  "ZIP extension ignores case; manifest content is validated later");
        }
    }

    std::filesystem::remove_all(root);
}

void TestMissingRoot() {
    const auto missing = std::filesystem::temp_directory_path() / "shiplua-root-does-not-exist";
    std::filesystem::remove_all(missing);
    const auto result = ShipLua::DiscoverMods(missing);
    Check(!result.isOk(), "missing mods root should fail");
    Check(result.code == ShipLua::ErrorCode::InvalidArgument,
          "missing root should report invalid argument");
}

} // namespace

int main() {
    TestManifestFixtures();
    TestManifestVersionRanges();
    TestDiscovery();
    TestMissingRoot();

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All manifest fixture and discovery checks passed\n";
    return 0;
}
