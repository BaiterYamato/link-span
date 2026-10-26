#pragma once

#include <memory>

#include "shiplua/attributes/AttributeRegistry.h"
#include "shiplua/native/NativeProvider.h"

namespace ShipLua {

// linkspan.attributes v1. Cada provider recebe, por NativeHostService::resolve, uma tabela cujo `context`
// identifica o mod; as tabelas vivem enquanto o serviço viver.
NativeHostService MakeAttributeNativeService(std::shared_ptr<AttributeRegistry> registry);

} // namespace ShipLua
