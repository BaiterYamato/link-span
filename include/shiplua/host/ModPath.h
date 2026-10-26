#pragma once

#include <filesystem>
#include <string>

#include "shiplua/runtime/Result.h"

namespace ShipLua {

// Caminho relativo dentro da pasta do mod, com a regra do entrypoint: sem raiz, sem ".." e sem link simbólico no
// caminho. PermissionDenied quando escapa da pasta; InvalidArgument quando não dá para inspecionar.
Result<std::filesystem::path> ResolveModFile(const std::filesystem::path& modDir, const std::string& relative);

} // namespace ShipLua
