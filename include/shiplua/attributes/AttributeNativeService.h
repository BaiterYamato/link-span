#pragma once

#include <functional>
#include <memory>
#include <string_view>

#include "shiplua/attributes/AttributeRegistry.h"
#include "shiplua/native/NativeProvider.h"

namespace ShipLua {

// linkspan.attributes v1. Cada mod recebe a própria tabela, cujo `context` identifica o dono; as tabelas vivem
// enquanto o serviço viver. O ModHost monta o descritor do mod na carga do provider (ForMod), sem campo novo em
// NativeHostService: include/shiplua/native entra no layout id do host.
struct AttributeNativeService {
    std::function<const void*(std::string_view owner)> resolve;

    NativeHostService ForMod(std::string_view owner) const;
};

AttributeNativeService MakeAttributeNativeService(std::shared_ptr<AttributeRegistry> registry);

} // namespace ShipLua
