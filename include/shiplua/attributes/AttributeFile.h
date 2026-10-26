#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "shiplua/attributes/AttributeTypes.h"

namespace ShipLua {

struct AttributeFileResult {
    AttributeDeclarations declarations;
    std::vector<AttributeDiagnostic> diagnostics;
    // false: sintaxe, esquema ou tamanho; o arquivo inteiro é ignorado e `declarations` fica vazio.
    bool accepted = false;
};

// Lê o attributes.toml do mod `owner`. Nunca lança.
AttributeFileResult ParseAttributeFile(std::string_view text, const std::string& owner);

} // namespace ShipLua
