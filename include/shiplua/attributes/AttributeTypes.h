#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace ShipLua {

enum class AttributeOp : std::uint8_t { Add = 0, MultiplyBase = 1, MultiplyTotal = 2 };
enum class AttributeWhen : std::uint8_t { Equipped = 0, Owned = 1 };

struct AttributeDefinition {
    std::string id;
    double base = 0.0;
    double minimum = std::numeric_limits<double>::lowest();
    double maximum = std::numeric_limits<double>::max();
    std::string name;
};

struct AttributeModifier {
    std::string id;
    std::string attribute;
    std::string target; // item, "#tag" ou vazio (vale no jogador; só em runtime)
    AttributeOp op = AttributeOp::Add;
    double value = 0.0;
    AttributeWhen when = AttributeWhen::Equipped;
};

// Conteúdo aceito de um attributes.toml.
struct AttributeDeclarations {
    std::vector<AttributeDefinition> attributes;
    std::vector<AttributeModifier> modifiers;
    std::vector<std::string> removes;                     // ids de modificador
    std::map<std::string, std::vector<std::string>> tags; // tag sem '#' -> itens ou "#tag"
};

enum class AttributeDiagnosticLevel { Info, Warning, Error };

struct AttributeDiagnostic {
    AttributeDiagnosticLevel level = AttributeDiagnosticLevel::Error;
    std::string message;
};

// Anexo B da RFC 0030.
struct AttributeLimits {
    static constexpr std::size_t kMaxFileBytes = 256 * 1024;
    static constexpr std::size_t kMaxAttributesPerMod = 256;
    static constexpr std::size_t kMaxModifiersPerMod = 1024; // arquivo + runtime
    static constexpr std::size_t kMaxTagMembersPerMod = 1024;
    static constexpr std::size_t kMaxTagDepth = 8;
    static constexpr std::size_t kMaxSources = 8;
    static constexpr std::size_t kMaxActiveItems = 256;
    static constexpr std::size_t kMaxIdBytes = 128;
};

// 1..128 bytes de [A-Za-z0-9_.-], sem ponto no começo, no fim ou repetido.
bool IsValidAttributeId(std::string_view id) noexcept;
// Válido e começa com "<owner>.", com pelo menos um caractere depois.
bool IsOwnedAttributeId(std::string_view id, std::string_view owner) noexcept;
// "#" seguido de id válido, ou id válido.
bool IsValidAttributeTarget(std::string_view target) noexcept;
const char* AttributeOpName(AttributeOp op) noexcept;       // "add", "multiply_base", "multiply_total"
const char* AttributeWhenName(AttributeWhen when) noexcept; // "equipped", "owned"
bool ParseAttributeOp(std::string_view text, AttributeOp& op) noexcept;
bool ParseAttributeWhen(std::string_view text, AttributeWhen& when) noexcept;

} // namespace ShipLua
