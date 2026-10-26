// Gerado por tools/generate_cpp_api.py. Não edite manualmente.
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace ShipLua::Generated {

inline constexpr std::string_view kApiVersion = "0.5.0";
inline constexpr std::uint32_t kSchemaVersion = 1;

enum class GameId {
    Oot,
    Mm,
};

using Subscription = std::uint64_t;

using TimerHandle = std::uint64_t;

struct EventOptions {
    std::optional<std::int64_t> priority;
};

struct ActorHandle {
    std::string kind;
    std::int64_t slot;
    std::int64_t generation;
    std::int64_t scene_generation;
};

struct ActorPosition {
    double x;
    double y;
    double z;
};

struct ActorRotation {
    double x;
    double y;
    double z;
};

struct ActorSpawnOptions {
    ActorPosition position;
    std::optional<ActorRotation> rotation;
};

struct OperationError {
    std::string code;
    std::string message;
};

struct ActorSnapshot {
    ActorHandle handle;
    std::int64_t actor_id;
    std::int64_t category;
    std::optional<std::int64_t> params;
};

struct HotkeyOptions {
    std::optional<std::string> default_;
    std::optional<std::string> label;
};

struct GameState {
    std::string mode;
    std::optional<std::int64_t> save_slot;
};

struct HudIconOptions {
    std::optional<std::int64_t> alpha;
};

struct AttributeBreakdownEntry {
    std::string modifier;
    std::optional<std::string> item;
    std::string op;
    double value;
};

struct AttributeModifierView {
    std::string id;
    std::string attribute;
    std::string op;
    double value;
    std::string when;
    std::optional<std::string> via;
};

struct AttributeModifier {
    std::string id;
    std::string attribute;
    std::string op;
    double value;
    std::optional<std::string> target;
    std::optional<std::string> when;
};

enum class ApiError {
    Unsupported,
    InvalidArgument,
    InvalidHandle,
    InvalidState,
    PermissionDenied,
    ResourceLimit,
    HostFailure,
    ScriptFailure,
};

enum class EventKind { Observe, Filter, Transform, Consume };

struct FieldBinding {
    std::string_view name;
    std::string_view type;
    bool required;
};

enum class FunctionId {
    ShipGameId,
    ShipGameHostVersion,
    ShipGameState,
    ShipRuntimeVersion,
    ShipApiVersion,
    ShipCapabilitiesHas,
    ShipCapabilitiesList,
    ShipEventsOn,
    ShipEventsOff,
    ShipHooksResult,
    ShipHotkeysRegister,
    ShipActorSpawn,
    ShipActorDestroy,
    ShipActorExists,
    ShipActorGet,
    ShipActorSetMaxHealth,
    ShipWorldTravel,
    ShipMmPlayerJump,
    ShipMmSpawnDog,
    ShipMmPlayerSetSwordSkin,
    ShipOotPlayerJump,
    ShipOotPlayerSetMask,
    ShipPlayerSetSpeedMultiplier,
    ShipPlayerGet,
    ShipPlayerSet,
    ShipOotPlayerAttachModel,
    ShipOotPlayerSetDamageImmunity,
    ShipOotPlayerSetWeight,
    ShipOotPlayerSetRollBlocked,
    ShipOotPlayerSetBody,
    ShipOotPlayerGetBody,
    ShipOotPlayerPlayBodyAnimation,
    ShipOotPlayerSetBodySegment,
    ShipOotPlayerSetHeldItemModel,
    ShipOotCutsceneStart,
    ShipOotCutsceneStop,
    ShipOotCutsceneIsActive,
    ShipOotEnvGet,
    ShipLogDebug,
    ShipLogInfo,
    ShipLogWarn,
    ShipLogError,
    ShipTimerAfter,
    ShipTimerEvery,
    ShipTimerCancel,
    ShipStorageGet,
    ShipStorageSet,
    ShipStorageDelete,
    ShipStorageClear,
    ShipStorageSharedGet,
    ShipStorageSharedSet,
    ShipStorageSharedDelete,
    ShipStorageSharedClear,
    ShipSaveAvailable,
    ShipSaveGet,
    ShipSaveSet,
    ShipSaveDelete,
    ShipAudioPlaySfx,
    ShipAudioPlayFanfare,
    ShipHudDrawRect,
    ShipHudDrawText,
    ShipHudDrawRing,
    ShipHudDrawIcon,
    ShipMenuRegister,
    ShipAttributesValue,
    ShipAttributesBreakdown,
    ShipAttributesModifiersOf,
    ShipAttributesHasTag,
    ShipAttributesTagMembers,
    ShipAttributesAddModifier,
    ShipAttributesRemoveModifier,
    ShipNativeHas,
    ShipNativeCall,
};

struct FunctionBinding {
    FunctionId id;
    std::string_view name;
    std::string_view version;
    std::string_view stability;
    std::string_view returnType;
    std::string_view errorMode;
    std::string_view errorType;
    std::string_view availability;
    std::string_view capability;
    std::span<const FieldBinding> arguments;
    std::span<const std::string_view> errors;
};

inline constexpr std::array<FieldBinding, 0> kShipGameIdArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipGameIdErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipGameHostVersionArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipGameHostVersionErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipGameStateArguments{{
}};
inline constexpr std::array<std::string_view, 1> kShipGameStateErrors{{
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 0> kShipRuntimeVersionArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipRuntimeVersionErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipApiVersionArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipApiVersionErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipCapabilitiesHasArguments{{
    {"name", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipCapabilitiesHasErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 0> kShipCapabilitiesListArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipCapabilitiesListErrors{{
}};
inline constexpr std::array<FieldBinding, 3> kShipEventsOnArguments{{
    {"event", "string", true},
    {"options_or_callback", "any", true},
    {"callback", "callback", false},
}};
inline constexpr std::array<std::string_view, 2> kShipEventsOnErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipEventsOffArguments{{
    {"subscription", "subscription", true},
}};
inline constexpr std::array<std::string_view, 1> kShipEventsOffErrors{{
    "invalid_handle",
}};
inline constexpr std::array<FieldBinding, 1> kShipHooksResultArguments{{
    {"value", "any", true},
}};
inline constexpr std::array<std::string_view, 1> kShipHooksResultErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 3> kShipHotkeysRegisterArguments{{
    {"id", "string", true},
    {"options", "hotkey_options", false},
    {"callback", "callback", true},
}};
inline constexpr std::array<std::string_view, 2> kShipHotkeysRegisterErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipActorSpawnArguments{{
    {"actor_type", "string", true},
    {"options", "actor_spawn_options", true},
}};
inline constexpr std::array<std::string_view, 6> kShipActorSpawnErrors{{
    "invalid_argument",
    "unsupported",
    "permission_denied",
    "invalid_state",
    "resource_limit",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 1> kShipActorDestroyArguments{{
    {"handle", "actor_handle", true},
}};
inline constexpr std::array<std::string_view, 5> kShipActorDestroyErrors{{
    "invalid_argument",
    "unsupported",
    "permission_denied",
    "invalid_handle",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 1> kShipActorExistsArguments{{
    {"handle", "actor_handle", true},
}};
inline constexpr std::array<std::string_view, 4> kShipActorExistsErrors{{
    "invalid_argument",
    "unsupported",
    "permission_denied",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 2> kShipActorGetArguments{{
    {"handle", "actor_handle", true},
    {"field", "string", true},
}};
inline constexpr std::array<std::string_view, 4> kShipActorGetErrors{{
    "invalid_argument",
    "unsupported",
    "permission_denied",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 2> kShipActorSetMaxHealthArguments{{
    {"handle", "actor_handle", true},
    {"max_health", "integer", true},
}};
inline constexpr std::array<std::string_view, 5> kShipActorSetMaxHealthErrors{{
    "invalid_argument",
    "unsupported",
    "permission_denied",
    "invalid_state",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 2> kShipWorldTravelArguments{{
    {"world", "game_id", true},
    {"destination", "string", true},
}};
inline constexpr std::array<std::string_view, 4> kShipWorldTravelErrors{{
    "invalid_argument",
    "unsupported",
    "invalid_state",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 0> kShipMmPlayerJumpArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipMmPlayerJumpErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipMmSpawnDogArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipMmSpawnDogErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipMmPlayerSetSwordSkinArguments{{
    {"skin", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipMmPlayerSetSwordSkinErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipOotPlayerJumpArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerJumpErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipOotPlayerSetMaskArguments{{
    {"mask", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetMaskErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipPlayerSetSpeedMultiplierArguments{{
    {"factor", "number", true},
}};
inline constexpr std::array<std::string_view, 0> kShipPlayerSetSpeedMultiplierErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipPlayerGetArguments{{
    {"field", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipPlayerGetErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipPlayerSetArguments{{
    {"field", "string", true},
    {"value", "number", true},
}};
inline constexpr std::array<std::string_view, 0> kShipPlayerSetErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipOotPlayerAttachModelArguments{{
    {"slot", "string", true},
    {"path", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerAttachModelErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipOotPlayerSetDamageImmunityArguments{{
    {"kind", "string", true},
    {"enabled", "boolean", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetDamageImmunityErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipOotPlayerSetWeightArguments{{
    {"weight", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetWeightErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipOotPlayerSetRollBlockedArguments{{
    {"blocked", "boolean", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetRollBlockedErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipOotPlayerSetBodyArguments{{
    {"spec", "any", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetBodyErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipOotPlayerGetBodyArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerGetBodyErrors{{
}};
inline constexpr std::array<FieldBinding, 3> kShipOotPlayerPlayBodyAnimationArguments{{
    {"name", "string", true},
    {"mode", "string", false},
    {"speed", "number", false},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerPlayBodyAnimationErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipOotPlayerSetBodySegmentArguments{{
    {"segment", "integer", true},
    {"path", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetBodySegmentErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipOotPlayerSetHeldItemModelArguments{{
    {"slot", "string", true},
    {"path", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotPlayerSetHeldItemModelErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipOotCutsceneStartArguments{{
    {"frames", "integer", true},
    {"options", "any", false},
}};
inline constexpr std::array<std::string_view, 2> kShipOotCutsceneStartErrors{{
    "invalid_argument",
    "invalid_state",
}};
inline constexpr std::array<FieldBinding, 0> kShipOotCutsceneStopArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipOotCutsceneStopErrors{{
}};
inline constexpr std::array<FieldBinding, 0> kShipOotCutsceneIsActiveArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipOotCutsceneIsActiveErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipOotEnvGetArguments{{
    {"field", "string", true},
}};
inline constexpr std::array<std::string_view, 0> kShipOotEnvGetErrors{{
}};
inline constexpr std::array<FieldBinding, 1> kShipLogDebugArguments{{
    {"message", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipLogDebugErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 1> kShipLogInfoArguments{{
    {"message", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipLogInfoErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 1> kShipLogWarnArguments{{
    {"message", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipLogWarnErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 1> kShipLogErrorArguments{{
    {"message", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipLogErrorErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 2> kShipTimerAfterArguments{{
    {"frames", "integer", true},
    {"callback", "callback", true},
}};
inline constexpr std::array<std::string_view, 3> kShipTimerAfterErrors{{
    "invalid_argument",
    "resource_limit",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipTimerEveryArguments{{
    {"frames", "integer", true},
    {"callback", "callback", true},
}};
inline constexpr std::array<std::string_view, 3> kShipTimerEveryErrors{{
    "invalid_argument",
    "resource_limit",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipTimerCancelArguments{{
    {"handle", "timer_handle", true},
}};
inline constexpr std::array<std::string_view, 2> kShipTimerCancelErrors{{
    "invalid_argument",
    "invalid_handle",
}};
inline constexpr std::array<FieldBinding, 2> kShipStorageGetArguments{{
    {"key", "string", true},
    {"default", "any", false},
}};
inline constexpr std::array<std::string_view, 2> kShipStorageGetErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipStorageSetArguments{{
    {"key", "string", true},
    {"value", "any", true},
}};
inline constexpr std::array<std::string_view, 3> kShipStorageSetErrors{{
    "invalid_argument",
    "resource_limit",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipStorageDeleteArguments{{
    {"key", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipStorageDeleteErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 0> kShipStorageClearArguments{{
}};
inline constexpr std::array<std::string_view, 1> kShipStorageClearErrors{{
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipStorageSharedGetArguments{{
    {"key", "string", true},
    {"default", "any", false},
}};
inline constexpr std::array<std::string_view, 2> kShipStorageSharedGetErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipStorageSharedSetArguments{{
    {"key", "string", true},
    {"value", "any", true},
}};
inline constexpr std::array<std::string_view, 3> kShipStorageSharedSetErrors{{
    "invalid_argument",
    "resource_limit",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipStorageSharedDeleteArguments{{
    {"key", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipStorageSharedDeleteErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 0> kShipStorageSharedClearArguments{{
}};
inline constexpr std::array<std::string_view, 1> kShipStorageSharedClearErrors{{
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 0> kShipSaveAvailableArguments{{
}};
inline constexpr std::array<std::string_view, 0> kShipSaveAvailableErrors{{
}};
inline constexpr std::array<FieldBinding, 2> kShipSaveGetArguments{{
    {"key", "string", true},
    {"default", "any", false},
}};
inline constexpr std::array<std::string_view, 3> kShipSaveGetErrors{{
    "invalid_argument",
    "invalid_state",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipSaveSetArguments{{
    {"key", "string", true},
    {"value", "any", true},
}};
inline constexpr std::array<std::string_view, 4> kShipSaveSetErrors{{
    "invalid_argument",
    "invalid_state",
    "resource_limit",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipSaveDeleteArguments{{
    {"key", "string", true},
}};
inline constexpr std::array<std::string_view, 3> kShipSaveDeleteErrors{{
    "invalid_argument",
    "invalid_state",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAudioPlaySfxArguments{{
    {"id", "integer", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAudioPlaySfxErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAudioPlayFanfareArguments{{
    {"id", "integer", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAudioPlayFanfareErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 8> kShipHudDrawRectArguments{{
    {"x", "integer", true},
    {"y", "integer", true},
    {"w", "integer", true},
    {"h", "integer", true},
    {"r", "integer", false},
    {"g", "integer", false},
    {"b", "integer", false},
    {"a", "integer", false},
}};
inline constexpr std::array<std::string_view, 2> kShipHudDrawRectErrors{{
    "invalid_argument",
    "invalid_state",
}};
inline constexpr std::array<FieldBinding, 8> kShipHudDrawTextArguments{{
    {"text", "string", true},
    {"x", "integer", true},
    {"y", "integer", true},
    {"r", "integer", false},
    {"g", "integer", false},
    {"b", "integer", false},
    {"a", "integer", false},
    {"scale", "number", false},
}};
inline constexpr std::array<std::string_view, 2> kShipHudDrawTextErrors{{
    "invalid_argument",
    "invalid_state",
}};
inline constexpr std::array<FieldBinding, 9> kShipHudDrawRingArguments{{
    {"cx", "number", true},
    {"cy", "number", true},
    {"radius", "number", true},
    {"thickness", "number", false},
    {"fraction", "number", false},
    {"r", "integer", false},
    {"g", "integer", false},
    {"b", "integer", false},
    {"a", "integer", false},
}};
inline constexpr std::array<std::string_view, 2> kShipHudDrawRingErrors{{
    "invalid_argument",
    "invalid_state",
}};
inline constexpr std::array<FieldBinding, 6> kShipHudDrawIconArguments{{
    {"path", "string", true},
    {"x", "integer", true},
    {"y", "integer", true},
    {"w", "integer", true},
    {"h", "integer", true},
    {"options", "hud_icon_options", false},
}};
inline constexpr std::array<std::string_view, 3> kShipHudDrawIconErrors{{
    "invalid_argument",
    "invalid_state",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipMenuRegisterArguments{{
    {"page", "any", true},
}};
inline constexpr std::array<std::string_view, 4> kShipMenuRegisterErrors{{
    "unsupported",
    "invalid_argument",
    "resource_limit",
    "host_failure",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesValueArguments{{
    {"attribute", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesValueErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesBreakdownArguments{{
    {"attribute", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesBreakdownErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesModifiersOfArguments{{
    {"item", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesModifiersOfErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 2> kShipAttributesHasTagArguments{{
    {"item", "string", true},
    {"tag", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesHasTagErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesTagMembersArguments{{
    {"tag", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesTagMembersErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesAddModifierArguments{{
    {"modifier", "attribute_modifier", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesAddModifierErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipAttributesRemoveModifierArguments{{
    {"id", "string", true},
}};
inline constexpr std::array<std::string_view, 2> kShipAttributesRemoveModifierErrors{{
    "invalid_argument",
    "unsupported",
}};
inline constexpr std::array<FieldBinding, 1> kShipNativeHasArguments{{
    {"name", "string", true},
}};
inline constexpr std::array<std::string_view, 1> kShipNativeHasErrors{{
    "invalid_argument",
}};
inline constexpr std::array<FieldBinding, 2> kShipNativeCallArguments{{
    {"name", "string", true},
    {"request", "string", true},
}};
inline constexpr std::array<std::string_view, 6> kShipNativeCallErrors{{
    "unsupported",
    "invalid_argument",
    "invalid_handle",
    "invalid_state",
    "resource_limit",
    "host_failure",
}};

inline constexpr std::array<FunctionBinding, 73> kFunctions{{
    {FunctionId::ShipGameId, "ship.game.id", "0.1.0", "stable", "game_id", "raise", {}, "common", {}, kShipGameIdArguments, kShipGameIdErrors},
    {FunctionId::ShipGameHostVersion, "ship.game.host_version", "0.1.0", "stable", "string", "raise", {}, "common", {}, kShipGameHostVersionArguments, kShipGameHostVersionErrors},
    {FunctionId::ShipGameState, "ship.game.state", "0.4.0", "experimental", "game_state", "raise", {}, "oot", "game.state", kShipGameStateArguments, kShipGameStateErrors},
    {FunctionId::ShipRuntimeVersion, "ship.runtime.version", "0.1.0", "stable", "string", "raise", {}, "common", {}, kShipRuntimeVersionArguments, kShipRuntimeVersionErrors},
    {FunctionId::ShipApiVersion, "ship.api.version", "0.1.0", "stable", "string", "raise", {}, "common", {}, kShipApiVersionArguments, kShipApiVersionErrors},
    {FunctionId::ShipCapabilitiesHas, "ship.capabilities.has", "0.1.0", "stable", "boolean", "raise", {}, "common", {}, kShipCapabilitiesHasArguments, kShipCapabilitiesHasErrors},
    {FunctionId::ShipCapabilitiesList, "ship.capabilities.list", "0.1.0", "stable", "array<string>", "raise", {}, "common", {}, kShipCapabilitiesListArguments, kShipCapabilitiesListErrors},
    {FunctionId::ShipEventsOn, "ship.events.on", "0.1.0", "stable", "subscription", "raise", {}, "common", {}, kShipEventsOnArguments, kShipEventsOnErrors},
    {FunctionId::ShipEventsOff, "ship.events.off", "0.1.0", "stable", "boolean", "raise", {}, "common", {}, kShipEventsOffArguments, kShipEventsOffErrors},
    {FunctionId::ShipHooksResult, "ship.hooks.result", "0.4.0", "experimental", "boolean", "raise", {}, "common", {}, kShipHooksResultArguments, kShipHooksResultErrors},
    {FunctionId::ShipHotkeysRegister, "ship.hotkeys.register", "0.2.0", "preview", "boolean", "raise", {}, "common", {}, kShipHotkeysRegisterArguments, kShipHotkeysRegisterErrors},
    {FunctionId::ShipActorSpawn, "ship.actor.spawn", "0.4.0", "experimental", "actor_handle", "return", "operation_error", "common", "actor.spawn", kShipActorSpawnArguments, kShipActorSpawnErrors},
    {FunctionId::ShipActorDestroy, "ship.actor.destroy", "0.4.0", "experimental", "boolean", "return", "operation_error", "common", "actor.destroy", kShipActorDestroyArguments, kShipActorDestroyErrors},
    {FunctionId::ShipActorExists, "ship.actor.exists", "0.4.0", "experimental", "boolean", "return", "operation_error", "common", "actor.exists", kShipActorExistsArguments, kShipActorExistsErrors},
    {FunctionId::ShipActorGet, "ship.actor.get", "0.5.0", "experimental", "any", "return", "operation_error", "common", "actor.observe", kShipActorGetArguments, kShipActorGetErrors},
    {FunctionId::ShipActorSetMaxHealth, "ship.actor.set_max_health", "0.5.0", "experimental", "boolean", "return", "operation_error", "common", "actor.health", kShipActorSetMaxHealthArguments, kShipActorSetMaxHealthErrors},
    {FunctionId::ShipWorldTravel, "ship.world.travel", "0.3.0", "experimental", "boolean", "raise", {}, "common", "world.travel", kShipWorldTravelArguments, kShipWorldTravelErrors},
    {FunctionId::ShipMmPlayerJump, "ship.mm.player.jump", "0.2.0", "experimental", "boolean", "raise", {}, "mm", "mm.player.jump", kShipMmPlayerJumpArguments, kShipMmPlayerJumpErrors},
    {FunctionId::ShipMmSpawnDog, "ship.mm.spawn_dog", "0.3.0", "experimental", "boolean", "raise", {}, "mm", "mm.spawn_dog", kShipMmSpawnDogArguments, kShipMmSpawnDogErrors},
    {FunctionId::ShipMmPlayerSetSwordSkin, "ship.mm.player.set_sword_skin", "0.4.0", "experimental", "boolean", "raise", {}, "mm", "mm.player.sword_skin", kShipMmPlayerSetSwordSkinArguments, kShipMmPlayerSetSwordSkinErrors},
    {FunctionId::ShipOotPlayerJump, "ship.oot.player.jump", "0.3.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.jump", kShipOotPlayerJumpArguments, kShipOotPlayerJumpErrors},
    {FunctionId::ShipOotPlayerSetMask, "ship.oot.player.set_mask", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.mask", kShipOotPlayerSetMaskArguments, kShipOotPlayerSetMaskErrors},
    {FunctionId::ShipPlayerSetSpeedMultiplier, "ship.player.set_speed_multiplier", "0.4.0", "experimental", "boolean", "raise", {}, "common", "player.speed", kShipPlayerSetSpeedMultiplierArguments, kShipPlayerSetSpeedMultiplierErrors},
    {FunctionId::ShipPlayerGet, "ship.player.get", "0.4.0", "experimental", "any", "raise", {}, "oot", "player.fields", kShipPlayerGetArguments, kShipPlayerGetErrors},
    {FunctionId::ShipPlayerSet, "ship.player.set", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "player.fields", kShipPlayerSetArguments, kShipPlayerSetErrors},
    {FunctionId::ShipOotPlayerAttachModel, "ship.oot.player.attach_model", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.attach_model", kShipOotPlayerAttachModelArguments, kShipOotPlayerAttachModelErrors},
    {FunctionId::ShipOotPlayerSetDamageImmunity, "ship.oot.player.set_damage_immunity", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.immunity", kShipOotPlayerSetDamageImmunityArguments, kShipOotPlayerSetDamageImmunityErrors},
    {FunctionId::ShipOotPlayerSetWeight, "ship.oot.player.set_weight", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.weight", kShipOotPlayerSetWeightArguments, kShipOotPlayerSetWeightErrors},
    {FunctionId::ShipOotPlayerSetRollBlocked, "ship.oot.player.set_roll_blocked", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.roll", kShipOotPlayerSetRollBlockedArguments, kShipOotPlayerSetRollBlockedErrors},
    {FunctionId::ShipOotPlayerSetBody, "ship.oot.player.set_body", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.custom_body", kShipOotPlayerSetBodyArguments, kShipOotPlayerSetBodyErrors},
    {FunctionId::ShipOotPlayerGetBody, "ship.oot.player.get_body", "0.4.0", "experimental", "any", "raise", {}, "oot", "oot.player.custom_body", kShipOotPlayerGetBodyArguments, kShipOotPlayerGetBodyErrors},
    {FunctionId::ShipOotPlayerPlayBodyAnimation, "ship.oot.player.play_body_animation", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.custom_body", kShipOotPlayerPlayBodyAnimationArguments, kShipOotPlayerPlayBodyAnimationErrors},
    {FunctionId::ShipOotPlayerSetBodySegment, "ship.oot.player.set_body_segment", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.custom_body", kShipOotPlayerSetBodySegmentArguments, kShipOotPlayerSetBodySegmentErrors},
    {FunctionId::ShipOotPlayerSetHeldItemModel, "ship.oot.player.set_held_item_model", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.player.held_item_model", kShipOotPlayerSetHeldItemModelArguments, kShipOotPlayerSetHeldItemModelErrors},
    {FunctionId::ShipOotCutsceneStart, "ship.oot.cutscene.start", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.cutscene", kShipOotCutsceneStartArguments, kShipOotCutsceneStartErrors},
    {FunctionId::ShipOotCutsceneStop, "ship.oot.cutscene.stop", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.cutscene", kShipOotCutsceneStopArguments, kShipOotCutsceneStopErrors},
    {FunctionId::ShipOotCutsceneIsActive, "ship.oot.cutscene.is_active", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "oot.cutscene", kShipOotCutsceneIsActiveArguments, kShipOotCutsceneIsActiveErrors},
    {FunctionId::ShipOotEnvGet, "ship.oot.env.get", "0.4.0", "experimental", "any", "raise", {}, "oot", "oot.env", kShipOotEnvGetArguments, kShipOotEnvGetErrors},
    {FunctionId::ShipLogDebug, "ship.log.debug", "0.1.0", "stable", "nil", "raise", {}, "common", {}, kShipLogDebugArguments, kShipLogDebugErrors},
    {FunctionId::ShipLogInfo, "ship.log.info", "0.1.0", "stable", "nil", "raise", {}, "common", {}, kShipLogInfoArguments, kShipLogInfoErrors},
    {FunctionId::ShipLogWarn, "ship.log.warn", "0.1.0", "stable", "nil", "raise", {}, "common", {}, kShipLogWarnArguments, kShipLogWarnErrors},
    {FunctionId::ShipLogError, "ship.log.error", "0.1.0", "stable", "nil", "raise", {}, "common", {}, kShipLogErrorArguments, kShipLogErrorErrors},
    {FunctionId::ShipTimerAfter, "ship.timer.after", "0.3.0", "experimental", "timer_handle", "raise", {}, "common", "core.timers", kShipTimerAfterArguments, kShipTimerAfterErrors},
    {FunctionId::ShipTimerEvery, "ship.timer.every", "0.3.0", "experimental", "timer_handle", "raise", {}, "common", "core.timers", kShipTimerEveryArguments, kShipTimerEveryErrors},
    {FunctionId::ShipTimerCancel, "ship.timer.cancel", "0.3.0", "experimental", "boolean", "raise", {}, "common", "core.timers", kShipTimerCancelArguments, kShipTimerCancelErrors},
    {FunctionId::ShipStorageGet, "ship.storage.get", "0.3.0", "experimental", "any", "raise", {}, "common", "core.storage", kShipStorageGetArguments, kShipStorageGetErrors},
    {FunctionId::ShipStorageSet, "ship.storage.set", "0.3.0", "experimental", "boolean", "raise", {}, "common", "core.storage", kShipStorageSetArguments, kShipStorageSetErrors},
    {FunctionId::ShipStorageDelete, "ship.storage.delete", "0.3.0", "experimental", "boolean", "raise", {}, "common", "core.storage", kShipStorageDeleteArguments, kShipStorageDeleteErrors},
    {FunctionId::ShipStorageClear, "ship.storage.clear", "0.3.0", "experimental", "integer", "raise", {}, "common", "core.storage", kShipStorageClearArguments, kShipStorageClearErrors},
    {FunctionId::ShipStorageSharedGet, "ship.storage.shared.get", "0.4.0", "experimental", "any", "raise", {}, "common", "core.storage.shared", kShipStorageSharedGetArguments, kShipStorageSharedGetErrors},
    {FunctionId::ShipStorageSharedSet, "ship.storage.shared.set", "0.4.0", "experimental", "boolean", "raise", {}, "common", "core.storage.shared", kShipStorageSharedSetArguments, kShipStorageSharedSetErrors},
    {FunctionId::ShipStorageSharedDelete, "ship.storage.shared.delete", "0.4.0", "experimental", "boolean", "raise", {}, "common", "core.storage.shared", kShipStorageSharedDeleteArguments, kShipStorageSharedDeleteErrors},
    {FunctionId::ShipStorageSharedClear, "ship.storage.shared.clear", "0.4.0", "experimental", "integer", "raise", {}, "common", "core.storage.shared", kShipStorageSharedClearArguments, kShipStorageSharedClearErrors},
    {FunctionId::ShipSaveAvailable, "ship.save.available", "0.5.0", "experimental", "boolean", "raise", {}, "common", "core.save", kShipSaveAvailableArguments, kShipSaveAvailableErrors},
    {FunctionId::ShipSaveGet, "ship.save.get", "0.5.0", "experimental", "any", "return", "operation_error", "common", "core.save", kShipSaveGetArguments, kShipSaveGetErrors},
    {FunctionId::ShipSaveSet, "ship.save.set", "0.5.0", "experimental", "boolean", "return", "operation_error", "common", "core.save", kShipSaveSetArguments, kShipSaveSetErrors},
    {FunctionId::ShipSaveDelete, "ship.save.delete", "0.5.0", "experimental", "boolean", "return", "operation_error", "common", "core.save", kShipSaveDeleteArguments, kShipSaveDeleteErrors},
    {FunctionId::ShipAudioPlaySfx, "ship.audio.play_sfx", "0.5.0", "experimental", "boolean", "raise", {}, "common", "audio.play", kShipAudioPlaySfxArguments, kShipAudioPlaySfxErrors},
    {FunctionId::ShipAudioPlayFanfare, "ship.audio.play_fanfare", "0.5.0", "experimental", "boolean", "raise", {}, "common", "audio.play", kShipAudioPlayFanfareArguments, kShipAudioPlayFanfareErrors},
    {FunctionId::ShipHudDrawRect, "ship.hud.draw_rect", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "hud.draw", kShipHudDrawRectArguments, kShipHudDrawRectErrors},
    {FunctionId::ShipHudDrawText, "ship.hud.draw_text", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "hud.draw", kShipHudDrawTextArguments, kShipHudDrawTextErrors},
    {FunctionId::ShipHudDrawRing, "ship.hud.draw_ring", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "hud.draw", kShipHudDrawRingArguments, kShipHudDrawRingErrors},
    {FunctionId::ShipHudDrawIcon, "ship.hud.draw_icon", "0.4.0", "experimental", "boolean", "raise", {}, "oot", "hud.icons", kShipHudDrawIconArguments, kShipHudDrawIconErrors},
    {FunctionId::ShipMenuRegister, "ship.menu.register", "0.5.0", "preview", "boolean", "raise", {}, "common", "core.menu", kShipMenuRegisterArguments, kShipMenuRegisterErrors},
    {FunctionId::ShipAttributesValue, "ship.attributes.value", "0.5.0", "experimental", "number", "raise", {}, "common", "core.attributes", kShipAttributesValueArguments, kShipAttributesValueErrors},
    {FunctionId::ShipAttributesBreakdown, "ship.attributes.breakdown", "0.5.0", "experimental", "array<attribute_breakdown_entry>", "raise", {}, "common", "core.attributes", kShipAttributesBreakdownArguments, kShipAttributesBreakdownErrors},
    {FunctionId::ShipAttributesModifiersOf, "ship.attributes.modifiers_of", "0.5.0", "experimental", "array<attribute_modifier_view>", "raise", {}, "common", "core.attributes", kShipAttributesModifiersOfArguments, kShipAttributesModifiersOfErrors},
    {FunctionId::ShipAttributesHasTag, "ship.attributes.has_tag", "0.5.0", "experimental", "boolean", "raise", {}, "common", "core.attributes", kShipAttributesHasTagArguments, kShipAttributesHasTagErrors},
    {FunctionId::ShipAttributesTagMembers, "ship.attributes.tag_members", "0.5.0", "experimental", "array<string>", "raise", {}, "common", "core.attributes", kShipAttributesTagMembersArguments, kShipAttributesTagMembersErrors},
    {FunctionId::ShipAttributesAddModifier, "ship.attributes.add_modifier", "0.5.0", "experimental", "boolean", "raise", {}, "common", "core.attributes", kShipAttributesAddModifierArguments, kShipAttributesAddModifierErrors},
    {FunctionId::ShipAttributesRemoveModifier, "ship.attributes.remove_modifier", "0.5.0", "experimental", "boolean", "raise", {}, "common", "core.attributes", kShipAttributesRemoveModifierArguments, kShipAttributesRemoveModifierErrors},
    {FunctionId::ShipNativeHas, "ship.native.has", "0.5.0", "experimental", "boolean", "raise", {}, "common", {}, kShipNativeHasArguments, kShipNativeHasErrors},
    {FunctionId::ShipNativeCall, "ship.native.call", "0.5.0", "experimental", "string", "return", "operation_error", "common", {}, kShipNativeCallArguments, kShipNativeCallErrors},
}};

struct EventBinding {
    std::string_view name;
    EventKind kind;
    std::string_view phase;
    bool cancellable;
    bool supportsOot;
    bool supportsMm;
    std::string_view capability;
    std::span<const FieldBinding> payload;
};

inline constexpr std::array<FieldBinding, 4> kGameReadyPayload{{
    {"game_id", "game_id", true},
    {"host_version", "string", true},
    {"runtime_version", "string", true},
    {"api_version", "string", true},
}};
inline constexpr std::array<FieldBinding, 1> kGameFramePayload{{
    {"frame", "integer", true},
}};
inline constexpr std::array<FieldBinding, 0> kGameShutdownPayload{{
}};
inline constexpr std::array<FieldBinding, 1> kSceneEnterPayload{{
    {"scene_id", "integer", true},
}};
inline constexpr std::array<FieldBinding, 1> kActorInitPayload{{
    {"actor", "actor_snapshot", true},
}};
inline constexpr std::array<FieldBinding, 1> kActorUpdatePayload{{
    {"actor", "actor_snapshot", true},
}};
inline constexpr std::array<FieldBinding, 1> kActorDestroyPayload{{
    {"handle", "actor_handle", true},
}};
inline constexpr std::array<FieldBinding, 1> kSaveLoadedPayload{{
    {"slot", "integer", true},
}};
inline constexpr std::array<FieldBinding, 1> kSaveSavingPayload{{
    {"slot", "integer", true},
}};
inline constexpr std::array<FieldBinding, 1> kTextOpenPayload{{
    {"text_id", "integer", true},
}};
inline constexpr std::array<FieldBinding, 2> kAudioSequenceStartedPayload{{
    {"player_index", "integer", true},
    {"sequence_id", "integer", true},
}};
inline constexpr std::array<FieldBinding, 2> kInputHotkeyPayload{{
    {"action", "string", true},
    {"key", "string", true},
}};
inline constexpr std::array<FieldBinding, 3> kInputActionPayload{{
    {"action", "string", true},
    {"pressed", "boolean", true},
    {"source", "string", true},
}};
inline constexpr std::array<FieldBinding, 1> kHookOotPlayerSpeedRunPayload{{
    {"speed", "number", true},
}};
inline constexpr std::array<FieldBinding, 0> kHookOotPlayerFallDamagePayload{{
}};
inline constexpr std::array<FieldBinding, 2> kHookOotItemReceivePayload{{
    {"item_id", "integer", true},
    {"get_item_id", "integer", true},
}};
inline constexpr std::array<FieldBinding, 1> kHookOotPlayerHealthChangePayload{{
    {"amount", "integer", true},
}};
inline constexpr std::array<FieldBinding, 0> kHookOotPlayerBonkPayload{{
}};
inline constexpr std::array<FieldBinding, 6> kHookOotEnemyDefeatPayload{{
    {"actor", "actor_snapshot", true},
    {"actor_id", "integer", true},
    {"category", "integer", true},
    {"pos_x", "number", true},
    {"pos_y", "number", true},
    {"pos_z", "number", true},
}};
inline constexpr std::array<FieldBinding, 14> kHookOotActorDamagePayload{{
    {"target_id", "integer", true},
    {"target_category", "integer", true},
    {"attacker_id", "integer", true},
    {"attacker_category", "integer", true},
    {"attacker_is_player", "boolean", true},
    {"dmg_flags", "integer", true},
    {"damage_effect", "integer", true},
    {"base_damage", "number", true},
    {"damage", "number", true},
    {"target", "actor_snapshot", true},
    {"attacker", "actor_snapshot", false},
    {"target_health", "integer", true},
    {"target_max_health", "integer", true},
    {"target_extended", "boolean", true},
}};
inline constexpr std::array<FieldBinding, 5> kHookOotActorDamagedPayload{{
    {"target", "actor_snapshot", true},
    {"amount", "integer", true},
    {"health", "integer", true},
    {"max_health", "integer", true},
    {"extended", "boolean", true},
}};
inline constexpr std::array<FieldBinding, 2> kHookOotItemGivePayload{{
    {"item_id", "integer", true},
    {"get_item_id", "integer", true},
}};
inline constexpr std::array<FieldBinding, 0> kHookOotHudDrawPayload{{
}};
inline constexpr std::array<FieldBinding, 1> kHookOotPlayerFirstPersonControlPayload{{
    {"held_item_action", "integer", true},
}};
inline constexpr std::array<FieldBinding, 2> kHookOotPlayerArrowTypeSelectPayload{{
    {"magic_arrow_type", "integer", true},
    {"arrow_type", "integer", true},
}};
inline constexpr std::array<FieldBinding, 17> kHookOotPlayerBodyAnimSelectPayload{{
    {"speed", "number", true},
    {"on_ground", "boolean", true},
    {"rolling", "boolean", true},
    {"roll_charge", "integer", true},
    {"roll_phase", "string", true},
    {"falling", "boolean", true},
    {"landing", "boolean", true},
    {"climbing", "boolean", true},
    {"climb_direction", "string", true},
    {"climb_step", "integer", true},
    {"climb_starting", "boolean", true},
    {"door_opening", "boolean", true},
    {"door_direction", "string", true},
    {"chest_opening", "boolean", true},
    {"instrument", "boolean", true},
    {"attacking", "boolean", true},
    {"attack_animation", "integer", true},
}};
inline constexpr std::array<FieldBinding, 1> kHookMmPlayerSpeedWalkPayload{{
    {"speed", "number", true},
}};
inline constexpr std::array<FieldBinding, 0> kHookMmPlayerGoronRollConsumeMagicPayload{{
}};
inline constexpr std::array<FieldBinding, 0> kHookMmPlayerGoronRollDisableSpikeModePayload{{
}};
inline constexpr std::array<FieldBinding, 0> kHookMmPlayerGoronRollIncreaseSpikeLevelPayload{{
}};
inline constexpr std::array<FieldBinding, 1> kHookMmItemGivePayload{{
    {"item", "integer", true},
}};
inline constexpr std::array<FieldBinding, 5> kHookMmEnemyDefeatPayload{{
    {"actor_id", "integer", true},
    {"category", "integer", true},
    {"pos_x", "number", true},
    {"pos_y", "number", true},
    {"pos_z", "number", true},
}};
inline constexpr std::array<FieldBinding, 1> kHookMmItemShouldGivePayload{{
    {"item", "integer", true},
}};

inline constexpr std::array<EventBinding, 33> kEvents{{
    {"game.ready", EventKind::Observe, "mvp", false, true, true, {}, kGameReadyPayload},
    {"game.frame", EventKind::Observe, "mvp", false, true, true, {}, kGameFramePayload},
    {"game.shutdown", EventKind::Observe, "mvp", false, true, true, {}, kGameShutdownPayload},
    {"scene.enter", EventKind::Observe, "host_bridge", false, true, true, "scene.events", kSceneEnterPayload},
    {"actor.init", EventKind::Observe, "host_bridge", false, true, true, "actor.events", kActorInitPayload},
    {"actor.update", EventKind::Observe, "host_bridge", false, true, true, "actor.events", kActorUpdatePayload},
    {"actor.destroy", EventKind::Observe, "host_bridge", false, true, true, "actor.events", kActorDestroyPayload},
    {"save.loaded", EventKind::Observe, "host_bridge", false, true, true, "save.events", kSaveLoadedPayload},
    {"save.saving", EventKind::Observe, "host_bridge", false, true, false, "save.events", kSaveSavingPayload},
    {"text.open", EventKind::Observe, "host_bridge", false, true, true, "text.events", kTextOpenPayload},
    {"audio.sequence_started", EventKind::Observe, "host_bridge", false, true, true, "audio.sequence.events", kAudioSequenceStartedPayload},
    {"input.hotkey", EventKind::Observe, "host_bridge", false, true, true, {}, kInputHotkeyPayload},
    {"input.action", EventKind::Transform, "host_bridge", false, true, false, "input.actions", kInputActionPayload},
    {"hook.oot.player.speed.run", EventKind::Transform, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerSpeedRunPayload},
    {"hook.oot.player.fall_damage", EventKind::Transform, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerFallDamagePayload},
    {"hook.oot.item.receive", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotItemReceivePayload},
    {"hook.oot.player.health_change", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerHealthChangePayload},
    {"hook.oot.player.bonk", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerBonkPayload},
    {"hook.oot.enemy.defeat", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotEnemyDefeatPayload},
    {"hook.oot.actor.damage", EventKind::Transform, "hook_bridge", false, true, false, "hooks.bridge", kHookOotActorDamagePayload},
    {"hook.oot.actor.damaged", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotActorDamagedPayload},
    {"hook.oot.item.give", EventKind::Transform, "hook_bridge", false, true, false, "hooks.bridge", kHookOotItemGivePayload},
    {"hook.oot.hud.draw", EventKind::Observe, "hook_bridge", false, true, false, "hud.draw", kHookOotHudDrawPayload},
    {"hook.oot.player.first_person_control", EventKind::Observe, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerFirstPersonControlPayload},
    {"hook.oot.player.arrow_type_select", EventKind::Transform, "hook_bridge", false, true, false, "hooks.bridge", kHookOotPlayerArrowTypeSelectPayload},
    {"hook.oot.player.body_anim_select", EventKind::Transform, "hook_bridge", false, true, false, "oot.player.custom_body", kHookOotPlayerBodyAnimSelectPayload},
    {"hook.mm.player.speed.walk", EventKind::Transform, "hook_bridge", false, false, true, "hooks.bridge", kHookMmPlayerSpeedWalkPayload},
    {"hook.mm.player.goron_roll.consume_magic", EventKind::Transform, "hook_bridge", false, false, true, "hooks.bridge", kHookMmPlayerGoronRollConsumeMagicPayload},
    {"hook.mm.player.goron_roll.disable_spike_mode", EventKind::Transform, "hook_bridge", false, false, true, "hooks.bridge", kHookMmPlayerGoronRollDisableSpikeModePayload},
    {"hook.mm.player.goron_roll.increase_spike_level", EventKind::Transform, "hook_bridge", false, false, true, "hooks.bridge", kHookMmPlayerGoronRollIncreaseSpikeLevelPayload},
    {"hook.mm.item.give", EventKind::Observe, "hook_bridge", false, false, true, "hooks.bridge", kHookMmItemGivePayload},
    {"hook.mm.enemy.defeat", EventKind::Observe, "hook_bridge", false, false, true, "hooks.bridge", kHookMmEnemyDefeatPayload},
    {"hook.mm.item.should_give", EventKind::Transform, "hook_bridge", false, false, true, "hooks.bridge", kHookMmItemShouldGivePayload},
}};

struct CapabilityBinding {
    std::string_view name;
    std::string_view status;
    bool supportsOot;
    bool supportsMm;
};

inline constexpr std::array<CapabilityBinding, 49> kCapabilities{{
    {"core.events", "contract", true, true},
    {"hooks.bridge", "contract", true, true},
    {"core.timers", "contract", true, true},
    {"core.input", "contract", true, true},
    {"core.menu", "contract", true, false},
    {"core.attributes", "contract", true, true},
    {"core.modules", "contract", true, true},
    {"core.save", "contract", true, false},
    {"input.actions", "contract", true, false},
    {"game.state", "contract", true, false},
    {"core.storage", "contract", true, true},
    {"core.storage.shared", "contract", true, true},
    {"oot.cutscene", "contract", true, false},
    {"oot.env", "contract", true, false},
    {"hud.draw", "contract", true, false},
    {"hud.icons", "contract", true, false},
    {"scene.events", "contract", true, true},
    {"actor.events", "contract", true, true},
    {"actor.spawn", "contract", true, true},
    {"actor.destroy", "contract", true, true},
    {"actor.exists", "contract", true, true},
    {"actor.observe", "contract", true, false},
    {"actor.health", "contract", true, false},
    {"audio.play", "contract", true, false},
    {"save.events", "contract", true, true},
    {"text.events", "contract", true, true},
    {"audio.sequence.events", "contract", true, true},
    {"world.travel", "contract", true, true},
    {"mm.room.events", "planned", false, true},
    {"mm.cycle", "planned", false, true},
    {"mm.owl_save", "planned", false, true},
    {"mm.clock", "planned", false, true},
    {"mm.player.jump", "contract", false, true},
    {"mm.spawn_dog", "contract", false, true},
    {"mm.player.sword_skin", "contract", false, true},
    {"oot.player.jump", "contract", true, false},
    {"oot.player.mask", "contract", true, false},
    {"player.speed", "contract", true, true},
    {"player.fields", "contract", true, false},
    {"oot.player.attach_model", "contract", true, false},
    {"mod.assets", "contract", true, false},
    {"oot.player.immunity", "contract", true, false},
    {"oot.player.weight", "contract", true, false},
    {"oot.player.roll", "contract", true, false},
    {"oot.player.custom_body", "contract", true, false},
    {"oot.player.held_item_model", "contract", true, false},
    {"oot.ocarina", "planned", true, false},
    {"oot.dungeon_keys", "planned", true, false},
    {"oot.equipment", "planned", true, false},
}};

} // namespace ShipLua::Generated
