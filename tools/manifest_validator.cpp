#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "shiplua/manifest/ManifestParser.h"
#include "shiplua/manifest/PackageExtractor.h"
#include "shiplua/manifest/PackageFormat.h"

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

std::string MensagemParaAutor(ShipLua::ErrorCode code, const std::string& detalhe) {
    const auto missing = detalhe.find("missing required field");
    if (missing != std::string::npos) {
        const auto fields = detalhe.find('\'', missing);
        return "missing required field" +
               (fields == std::string::npos ? std::string{} : ": " + detalhe.substr(fields));
    }
    const auto invalidField = detalhe.find("invalid field '");
    if (invalidField != std::string::npos) {
        const auto begin = invalidField + 15;
        const auto end = detalhe.find('\'', begin);
        return "invalid field '" + detalhe.substr(begin, end - begin) + "'";
    }
    const auto dependency = detalhe.find("invalid dependency range for '");
    if (dependency != std::string::npos) {
        const auto begin = dependency + 30;
        const auto end = detalhe.find('\'', begin);
        return "invalid range for dependency '" + detalhe.substr(begin, end - begin) + "'";
    }
    if (detalhe.find("TOML parse error") != std::string::npos) {
        const auto line = detalhe.find("line ");
        const auto column = detalhe.find("column ");
        if (line != std::string::npos && column != std::string::npos) {
            const auto lineEnd = detalhe.find(',', line);
            const auto columnEnd = detalhe.find(':', column);
            return "TOML syntax error at line " +
                   detalhe.substr(line + 5, lineEnd - line - 5) + ", column " +
                   detalhe.substr(column + 7, columnEnd - column - 7);
        }
        return "TOML syntax error";
    }
    switch (code) {
        case ShipLua::ErrorCode::PermissionDenied:
            return "the package contains an unsafe or forbidden path";
        case ShipLua::ErrorCode::ResourceLimit:
            return "the package exceeds the safety limits";
        case ShipLua::ErrorCode::HostFailure:
            return "system failure while reading or preparing the package";
        default:
            return "invalid structure or content";
    }
}

ShipLua::Result<ShipLua::Manifest> ValidarEntrada(const std::filesystem::path& entrada) {
    std::error_code error;
    const auto status = std::filesystem::symlink_status(entrada, error);
    if (error || std::filesystem::is_symlink(status)) {
        return ShipLua::Result<ShipLua::Manifest>::err(
            ShipLua::ErrorCode::PermissionDenied, "path is missing, unreadable or a symbolic link");
    }
    if (std::filesystem::is_directory(status)) {
        return ShipLua::ParseManifestFile((entrada / "manifest.toml").string());
    }
    if (!std::filesystem::is_regular_file(status)) {
        return ShipLua::Result<ShipLua::Manifest>::err(
            ShipLua::ErrorCode::InvalidArgument, "path is neither a regular file nor a folder");
    }
    if (!ShipLua::HasModPackageExtension(entrada)) {
        return ShipLua::ParseManifestFile(entrada.string());
    }
    if (!ShipLua::HasModPackageManifest(entrada)) {
        return ShipLua::Result<ShipLua::Manifest>::err(
            ShipLua::ErrorCode::InvalidArgument, "invalid ZIP or no regular manifest.toml at its root");
    }

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    TempDirectory temp(std::filesystem::temp_directory_path() /
                       ("shiplua-validator-" + std::to_string(stamp)));
    const auto extracted = temp.Path() / "conteudo";
    auto extraction = ShipLua::ExtractShipmod(entrada, extracted);
    if (!extraction.isOk()) {
        return ShipLua::Result<ShipLua::Manifest>::err(extraction.code, extraction.message);
    }
    return ShipLua::ParseManifestFile((extracted / "manifest.toml").string());
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: shiplua_manifest_validator <manifest|folder|package.zip|package.shipmod> [...]\n";
        return 2;
    }

    bool todosValidos = true;
    for (int i = 1; i < argc; ++i) {
        const std::filesystem::path entrada(argv[i]);
        auto resultado = ValidarEntrada(entrada);
        if (resultado.isOk()) {
            std::cout << "Valid: " << entrada.string() << " — id=" << resultado.value->id
                      << ", version=" << resultado.value->version << '\n';
        } else {
            todosValidos = false;
            std::cerr << "Invalid: " << entrada.string() << " — "
                      << MensagemParaAutor(resultado.code, resultado.message) << '\n';
        }
    }
    return todosValidos ? 0 : 1;
}
