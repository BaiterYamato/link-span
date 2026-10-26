#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "shiplua/attributes/AttributeTypes.h"
#include "shiplua/runtime/Logger.h"
#include "shiplua/runtime/Result.h"

namespace ShipLua {

// false = acabou a vaga de itens desta consulta (AttributeLimits::kMaxActiveItems).
using AttributeEmit = std::function<bool(std::string_view item, AttributeWhen when)>;
// false = a fonte falhou; ela é pulada nesta consulta.
using AttributeSource = std::function<bool(const AttributeEmit& emit)>;

struct AttributeBreakdownEntry {
    std::string modifier;
    std::string item; // vazio para modificador sem alvo
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
};

struct AttributeModifierView {
    std::string id;
    std::string attribute;
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
    AttributeWhen when = AttributeWhen::Equipped;
    std::string via; // tag do alvo, sem '#'; vazio quando o alvo é o próprio item
};

// Registro da RFC 0030. Não é thread-safe: tudo roda na thread do jogo.
class AttributeRegistry {
  public:
    AttributeRegistry() = default;
    AttributeRegistry(const AttributeRegistry&) = delete;
    AttributeRegistry& operator=(const AttributeRegistry&) = delete;

    // Falhas de fonte vão para este logger, uma vez por fonte e sessão.
    void SetLogger(Logger logger);

    // Declarações de um mod. A primeira chamada fixa a posição do mod na ordem de carga; uma nova chamada para o
    // mesmo dono troca as declarações e mantém a posição. `dependencies` = ids de que o mod depende, direta ou
    // transitivamente. Devolve só diagnósticos ainda não devolvidos (ciclos, remoções).
    std::vector<AttributeDiagnostic> AddMod(const std::string& owner, AttributeDeclarations declarations,
                                            std::set<std::string> dependencies);
    // Tira declarações, modificadores de runtime e fontes do dono. Dono desconhecido: nada.
    std::vector<AttributeDiagnostic> RemoveMod(const std::string& owner);
    bool HasMod(const std::string& owner) const;
    // Avisos de estado: modificador para atributo não declarado, alvo #tag sem membros. Ordem determinística.
    std::vector<AttributeDiagnostic> Audit() const;

    // Nome 1..128 bytes de id válido. Nome repetido ou mais de 8 fontes: ResourceLimit.
    Result<std::uint64_t> RegisterSource(const std::string& owner, const std::string& name, AttributeSource source);
    // Handle de outro dono ou inexistente: InvalidHandle.
    Result<void> UnregisterSource(const std::string& owner, std::uint64_t handle);

    // Id fora do prefixo do dono: PermissionDenied. Id repetido (arquivo ou runtime, qualquer mod): InvalidState.
    // Campo inválido ou valor não finito: InvalidArgument. Mais de 1024 modificadores do dono: ResourceLimit.
    Result<void> AddRuntimeModifier(const std::string& owner, AttributeModifier modifier);
    // Só o modificador de runtime do próprio dono.
    bool RemoveRuntimeModifier(const std::string& owner, const std::string& id);

    // Atributo não declarado: InvalidArgument "unknown attribute". Chamada de dentro de uma fonte: InvalidState
    // "reentrant call".
    Result<double> Value(const std::string& attribute);
    Result<std::vector<AttributeBreakdownEntry>> Breakdown(const std::string& attribute);
    std::vector<AttributeModifierView> ModifiersOf(const std::string& item) const;
    bool HasTag(const std::string& item, const std::string& tag) const;
    std::vector<std::string> TagMembers(const std::string& tag) const; // ordem crescente

  private:
    struct State;
    std::shared_ptr<State> mState;
};

} // namespace ShipLua
