#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace ShipLua {

// A extensão identifica candidatos; o manifesto identifica o mod.
inline bool HasModPackageExtension(const std::filesystem::path& path) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".zip" || extension == ".shipmod";
}

// Inspeciona o ZIP sem extrair: exige um arquivo manifest.toml exatamente na raiz.
// Não valida o conteúdo TOML; recusa arquivos simbólicos.
bool HasModPackageManifest(const std::filesystem::path& path);

} // namespace ShipLua
