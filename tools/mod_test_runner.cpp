#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "shiplua/manifest/PackageExtractor.h"
#include "shiplua/manifest/PackageFormat.h"
#include "shiplua/testing/ModTestRunner.h"

namespace {

class TempDirectory {
  public:
    explicit TempDirectory(std::filesystem::path path) : mPath(std::move(path)) {}
    ~TempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(mPath, ignored);
    }
    const std::filesystem::path& Path() const { return mPath; }

  private:
    std::filesystem::path mPath;
};

void PrintUsage() {
    std::cerr << "Usage: shiplua_mod_test_runner <mod-folder|package.zip|package.shipmod> "
                 "[--game oot|mm] [--capability <name>]...\n"
                 "\n"
                 "Runs the mod's tests (<mod>/tests/*.lua) in the mock runtime, without the game or a ROM.\n"
                 "Exit code: 0 = all passed; 1 = failures or errors; 2 = invalid usage.\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return 2;
    }

    ShipLua::MockHostOptions options;
    std::filesystem::path input;
    bool haveInput = false;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--game") {
            if (i + 1 >= argc) {
                PrintUsage();
                return 2;
            }
            options.gameId = argv[++i];
            if (options.gameId != "oot" && options.gameId != "mm") {
                std::cerr << "--game must be 'oot' or 'mm'\n";
                return 2;
            }
        } else if (argument == "--capability") {
            if (i + 1 >= argc) {
                PrintUsage();
                return 2;
            }
            options.extraCapabilities.emplace_back(argv[++i]);
        } else if (!argument.empty() && argument.front() == '-') {
            std::cerr << "Unknown option: " << argument << '\n';
            PrintUsage();
            return 2;
        } else if (!haveInput) {
            input = argument;
            haveInput = true;
        } else {
            std::cerr << "Only one mod path is accepted per run.\n";
            PrintUsage();
            return 2;
        }
    }
    if (!haveInput) {
        PrintUsage();
        return 2;
    }

    std::filesystem::path modDir = input;
    std::unique_ptr<TempDirectory> temporary;
    std::error_code error;
    const auto status = std::filesystem::symlink_status(input, error);
    if (error || std::filesystem::is_symlink(status)) {
        std::cerr << "Path is missing, unreadable or a symbolic link: " << input.string() << '\n';
        return 2;
    }
    if (std::filesystem::is_regular_file(status)) {
        if (!ShipLua::HasModPackageExtension(input)) {
            std::cerr << "Input file is not a .zip or .shipmod package: " << input.string() << '\n';
            return 2;
        }
        if (!ShipLua::HasModPackageManifest(input)) {
            std::cerr << "Invalid ZIP or no regular manifest.toml at its root: " << input.string() << '\n';
            return 1;
        }
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        temporary = std::make_unique<TempDirectory>(
            std::filesystem::temp_directory_path() /
            ("shiplua-mod-test-" + std::to_string(stamp)));
        const std::filesystem::path extracted = temporary->Path() / "conteudo";
        const auto extraction = ShipLua::ExtractShipmod(input, extracted);
        if (!extraction.isOk()) {
            std::cerr << "Failed to extract the package: " << extraction.message << '\n';
            return 1;
        }
        modDir = extracted;
    } else if (!std::filesystem::is_directory(status)) {
        std::cerr << "Path is neither a folder nor a regular file: " << input.string() << '\n';
        return 2;
    }

    ShipLua::ModTestRunner runner(std::move(options));
    const auto report = runner.Run(modDir);
    if (!report.isOk()) {
        std::cerr << "Failed to run the tests: " << report.message << '\n';
        return 1;
    }
    const ShipLua::ModTestReport& result = *report.value;

    if (result.noTestsFound) {
        std::cout << "No tests found in " << (modDir / "tests").string()
                  << "; nothing to run.\n";
        return 0;
    }

    std::cout << "Mod: " << (result.modId.empty() ? "(unknown)" : result.modId) << '\n';
    for (const ShipLua::ModTestCaseResult& test : result.tests) {
        if (test.ok) {
            std::cout << "[ok] " << test.file << " :: " << test.test << '\n';
        } else {
            std::cout << "[fail] " << test.file << " :: " << test.test;
            if (!test.message.empty()) {
                const auto newline = test.message.find('\n');
                std::cout << " — " << test.message.substr(0, newline);
            }
            std::cout << '\n';
        }
    }
    for (const std::string& infraError : result.errors) {
        std::cout << "[error] " << infraError << '\n';
    }

    const std::size_t passed = result.PassedCount();
    const std::size_t failed = result.FailedCount();
    std::cout << "Summary: " << passed << " passed, " << failed << " failed ("
              << result.tests.size() << " test(s) in " << result.files << " file(s))\n";
    if (!result.errors.empty()) {
        std::cout << "Infrastructure errors: " << result.errors.size() << '\n';
    }
    return failed == 0 && result.errors.empty() ? 0 : 1;
}
