#include "shiplua/native/NativeHooks.h"
#include "shiplua/native/NativeDiagnostics.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace ShipLua {
namespace {
bool ValidName(const char* name) {
    if (!name || !*name) return false;
    const auto length = std::strlen(name);
    return length <= SHIP_NATIVE_MAX_HOOK_NAME;
}

bool SingleMode(uint32_t mode) {
    return mode == SHIP_NATIVE_HOOK_OBSERVE || mode == SHIP_NATIVE_HOOK_TRANSFORM ||
           mode == SHIP_NATIVE_HOOK_REPLACE;
}

const char* ModeName(uint32_t mode) {
    switch (mode) {
        case SHIP_NATIVE_HOOK_OBSERVE: return "observe";
        case SHIP_NATIVE_HOOK_TRANSFORM: return "transform";
        case SHIP_NATIVE_HOOK_REPLACE: return "replace";
        default: return "?";
    }
}

struct OriginalFrame {
    ShipNativeHookOriginalFn original;
    void* context;
    bool called;
};

ShipNativeStatus RunOriginal(const ShipNativeHookCall& call, ShipNativeHookOriginalFn original,
                             void* context) noexcept {
    if (!original) return SHIP_NATIVE_OK;
    ShipNativeHookCall forwarded = call;
    forwarded.mode = 0;
    forwarded.phase = 0;
    forwarded.call_original = nullptr;
    forwarded.original_context = context;
    try {
        return original(&forwarded);
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus SHIP_NATIVE_CALL CallOriginal(const ShipNativeHookCall* call) {
    if (!call || !call->original_context) return SHIP_NATIVE_INVALID_ARGUMENT;
    auto* frame = static_cast<OriginalFrame*>(call->original_context);
    if (frame->called) return SHIP_NATIVE_FAILURE;
    frame->called = true;
    return RunOriginal(*call, frame->original, frame->context);
}

ShipNativeStatus Invoke(const std::string& owner, ShipNativeHookFn callback, void* user,
                        const ShipNativeHookCall& call) noexcept {
    const NativeActivityScope activity(owner.c_str(), "hook", call.point);
    try {
        return callback(user, &call);
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}
} // namespace

NativeHookRegistry::NativeHookRegistry(Logger logger) : mLogger(std::move(logger)) {}

void NativeHookRegistry::Log(const std::string& message) const {
    if (!mLogger) return;
    try { mLogger(message); } catch (...) { /* Logger não pode derrubar o despacho. */ }
}

void NativeHookRegistry::Notify(uint64_t point) {
    const auto named = mPoints.find(point);
    if (named != mPoints.end()) named->second.changed = false;
    try {
        const auto found = mListeners.find(point);
        if (found == mListeners.end()) return;
        found->second(point); // sem copiar std::function: o ouvinte é só leitura
    } catch (...) {
        try {
            Log("ouvinte do ponto '" + (named != mPoints.end() ? named->second.name : std::to_string(point)) +
                "' falhou");
        } catch (...) { /* Construção e emissão do diagnóstico não impedem a retirada. */ }
    }
}

ShipNativeStatus NativeHookRegistry::DeclarePoint(std::string_view owner, const ShipNativeHookPointSpec& spec,
                                                  uint64_t* point) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    if (!point || spec.size < sizeof(ShipNativeHookPointSpec) || !ValidName(spec.point) || !spec.version ||
        spec.payload_size > SHIP_NATIVE_MAX_BYTES || !spec.modes ||
        (spec.modes & ~(SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM | SHIP_NATIVE_HOOK_REPLACE))) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    if (FindPoint(spec.point, spec.version)) return SHIP_NATIVE_INVALID_ARGUMENT;
    std::size_t active = 0;
    for (const auto& entry : mPoints) active += entry.second.removed ? 0 : 1;
    if (active >= kMaxPoints) return SHIP_NATIVE_LIMIT;
    try {
        Point created;
        created.owner = owner;
        created.name = spec.point;
        created.version = spec.version;
        created.payloadSize = spec.payload_size;
        created.modes = spec.modes;
        const auto id = mNextPoint++;
        mPoints.emplace(id, std::move(created));
        *point = id;
        return SHIP_NATIVE_OK;
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}

uint64_t NativeHookRegistry::FindPoint(std::string_view name, uint32_t version) const noexcept {
    for (const auto& entry : mPoints) {
        if (!entry.second.removed && entry.second.version == version && entry.second.name == name) {
            return entry.first;
        }
    }
    return 0;
}

bool NativeHookRegistry::IsOwnerOf(std::string_view owner, uint64_t point) const noexcept {
    const auto found = mPoints.find(point);
    return found != mPoints.end() && !found->second.removed && found->second.owner == owner;
}

bool NativeHookRegistry::HasHooks(uint64_t point) const noexcept {
    const auto found = mPoints.find(point);
    return found != mPoints.end() && !found->second.removed && found->second.live != 0;
}

void NativeHookRegistry::Insert(Point& point, Hook hook) {
    const auto position = std::find_if(point.hooks.begin(), point.hooks.end(), [&hook](const Hook& existing) {
        return existing.priority < hook.priority;
    });
    point.hooks.insert(position, std::move(hook));
    ++point.live;
}

ShipNativeStatus NativeHookRegistry::Register(std::string_view owner, const ShipNativeHookSpec& spec,
                                              uint64_t* handle) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    if (!handle || spec.size < sizeof(ShipNativeHookSpec) || !ValidName(spec.point) || !spec.version ||
        !spec.callback || !SingleMode(spec.mode) ||
        (spec.mode == SHIP_NATIVE_HOOK_OBSERVE ? spec.phase > SHIP_NATIVE_HOOK_AFTER : spec.phase != 0)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    const auto pointId = FindPoint(spec.point, spec.version);
    if (!pointId) return SHIP_NATIVE_UNSUPPORTED;
    auto& point = mPoints.at(pointId);
    if (!(point.modes & spec.mode) || spec.minimum_payload_size > point.payloadSize) {
        return SHIP_NATIVE_UNSUPPORTED;
    }
    std::size_t owned = 0;
    for (const auto& entry : mPoints) {
        for (const auto& hook : entry.second.hooks) owned += !hook.removed && hook.owner == owner;
    }
    for (const auto& pending : mPending) owned += pending.second.owner == owner;
    if (owned >= kMaxHooksPerOwner) return SHIP_NATIVE_LIMIT;
    if (spec.mode == SHIP_NATIVE_HOOK_REPLACE) {
        auto replaced = [](const Hook& hook) {
            return hook.mode == SHIP_NATIVE_HOOK_REPLACE && hook.enabled && !hook.removed;
        };
        const Hook* current = nullptr;
        for (const auto& hook : point.hooks) if (replaced(hook)) current = &hook;
        for (const auto& pending : mPending) {
            if (pending.first == pointId && replaced(pending.second)) current = &pending.second;
        }
        if (current) {
            Log("hook replace recusado em '" + point.name + "' para '" + std::string(owner) + "': ja pertence a '" +
                current->owner + "'");
            return SHIP_NATIVE_LIMIT;
        }
    }
    try {
        Hook hook;
        hook.handle = mNextHook++;
        hook.owner = owner;
        hook.mode = spec.mode;
        hook.phase = spec.phase;
        hook.priority = spec.priority;
        hook.callback = spec.callback;
        hook.user = spec.user;
        const auto id = hook.handle;
        mHookPoints.emplace(id, pointId);
        const bool live = mDepth == 0;
        if (live) {
            Insert(point, std::move(hook));
        } else {
            mPending.emplace_back(pointId, std::move(hook));
        }
        *handle = id;
        if (live) Notify(pointId);
        return SHIP_NATIVE_OK;
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
}

ShipNativeStatus NativeHookRegistry::Unregister(std::string_view owner, uint64_t handle) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto mapped = mHookPoints.find(handle);
    if (mapped == mHookPoints.end()) return SHIP_NATIVE_INVALID_ARGUMENT;
    const auto pointId = mapped->second;
    auto& point = mPoints.at(pointId);
    for (auto pending = mPending.begin(); pending != mPending.end(); ++pending) {
        if (pending->second.handle != handle) continue;
        if (pending->second.owner != owner) return SHIP_NATIVE_INVALID_ARGUMENT;
        mPending.erase(pending);
        mHookPoints.erase(mapped);
        return SHIP_NATIVE_OK;
    }
    for (auto hook = point.hooks.begin(); hook != point.hooks.end(); ++hook) {
        if (hook->handle != handle || hook->removed) continue;
        if (hook->owner != owner) return SHIP_NATIVE_INVALID_ARGUMENT;
        const bool wasLive = hook->enabled;
        if (wasLive) --point.live;
        mHookPoints.erase(mapped);
        if (mDepth > 0) {
            hook->enabled = false;
            hook->removed = true;
            mDirty = true;
        } else {
            point.hooks.erase(hook);
        }
        if (wasLive) Notify(pointId);
        return SHIP_NATIVE_OK;
    }
    return SHIP_NATIVE_INVALID_ARGUMENT;
}

void NativeHookRegistry::RemoveOwner(std::string_view owner) {
    if (!OnThread()) return;
    for (auto& [id, point] : mPoints) {
        const bool dropPoint = !point.removed && !owner.empty() && point.owner == owner;
        if (dropPoint) point.changed = true;
        for (auto& hook : point.hooks) {
            if (hook.removed || (!dropPoint && hook.owner != owner)) continue;
            if (dropPoint && hook.owner != owner) {
                try {
                    Log("hook de '" + hook.owner + "' removido: ponto '" + point.name + "' saiu com '" +
                        std::string(owner) + "'");
                } catch (...) { /* O diagnóstico não interrompe o unload. */ }
            }
            if (hook.enabled) {
                --point.live;
                point.changed = true;
            }
            hook.enabled = false;
            hook.removed = true;
            mHookPoints.erase(hook.handle);
        }
        if (dropPoint) {
            point.removed = true;
            point.dropListener = true;
        }
    }
    mPending.erase(std::remove_if(mPending.begin(), mPending.end(), [&](const auto& pending) {
        const auto& point = mPoints.at(pending.first);
        const bool drop = pending.second.owner == owner || point.removed;
        if (drop) mHookPoints.erase(pending.second.handle);
        return drop;
    }), mPending.end());
    for (auto& [id, point] : mPoints) {
        if (point.changed) Notify(id);
        if (point.dropListener) {
            mListeners.erase(id);
            point.dropListener = false;
        }
    }
    mDirty = true;
    if (mDepth == 0) Compact();
}

void NativeHookRegistry::Compact() {
    mDirty = false;
    for (auto entry = mPoints.begin(); entry != mPoints.end();) {
        auto& hooks = entry->second.hooks;
        hooks.erase(std::remove_if(hooks.begin(), hooks.end(), [](const Hook& hook) { return hook.removed; }),
                    hooks.end());
        if (entry->second.removed) {
            entry = mPoints.erase(entry);
        } else {
            ++entry;
        }
    }
    auto pending = std::move(mPending);
    mPending.clear();
    for (auto& item : pending) {
        const auto point = mPoints.find(item.first);
        if (point == mPoints.end()) {
            mHookPoints.erase(item.second.handle);
            continue;
        }
        try {
            Insert(point->second, std::move(item.second));
            point->second.changed = true;
        } catch (...) {
            mHookPoints.erase(item.second.handle);
            try {
                Log("hook pendente descartado por falta de memoria em '" + point->second.name + "'");
            } catch (...) { /* Não interromper os avisos dos outros pontos. */ }
        }
    }
    for (auto& [id, point] : mPoints) {
        if (point.changed) Notify(id);
    }
}

void NativeHookRegistry::DisableHook(Point& point, Hook& hook) {
    if (!hook.enabled) return;
    hook.enabled = false;
    --point.live;
    point.changed = true;
    mDirty = true;
    try {
        Log(std::string("hook ") + ModeName(hook.mode) + " de '" + hook.owner + "' em '" + point.name +
            "' desativado apos " + std::to_string(kMaxFailures) + " falhas seguidas");
    } catch (...) { /* O aviso no fim do despacho independe do log. */ }
}

// Retorna true quando o hook não concluiu (payload restaurado / original roda).
// UNSUPPORTED é recusa voluntária e não conta como falha.
bool NativeHookRegistry::RecordFailure(Point& point, Hook& hook, ShipNativeStatus status) {
    if (status == SHIP_NATIVE_OK) {
        hook.failures = 0;
        return false;
    }
    if (status != SHIP_NATIVE_UNSUPPORTED && ++hook.failures >= kMaxFailures) DisableHook(point, hook);
    return true;
}

ShipNativeStatus NativeHookRegistry::Dispatch(uint64_t pointId, void* payload, uint32_t payloadSize,
                                              ShipNativeHookOriginalFn original, void* originalContext) noexcept {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto found = mPoints.find(pointId);
    if ((!payload && payloadSize) || (found != mPoints.end() && payloadSize != found->second.payloadSize)) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    ShipNativeHookCall call{};
    call.size = sizeof(ShipNativeHookCall);
    call.payload_size = payloadSize;
    call.payload = payload;
    if (found == mPoints.end() || found->second.removed || found->second.live == 0) {
        if (found != mPoints.end()) {
            call.point = found->second.name.c_str();
            call.point_version = found->second.version;
        }
        return RunOriginal(call, original, originalContext);
    }

    // std::map preserva a referência; hooks só são apagados com mDepth == 0.
    auto& point = found->second;
    call.point = point.name.c_str();
    call.point_version = point.version;
    ++mDepth;
    const auto count = point.hooks.size();
    auto observe = [&](uint32_t phase) {
        for (std::size_t index = 0; index < count; ++index) {
            auto& hook = point.hooks[index];
            if (!hook.enabled || hook.mode != SHIP_NATIVE_HOOK_OBSERVE || hook.phase != phase) continue;
            call.mode = SHIP_NATIVE_HOOK_OBSERVE;
            call.phase = phase;
            RecordFailure(point, hook, Invoke(hook.owner, hook.callback, hook.user, call));
        }
    };

    observe(SHIP_NATIVE_HOOK_BEFORE);

    std::array<uint8_t, 256> small{};
    std::vector<uint8_t> large;
    uint8_t* snapshot = small.data();
    for (std::size_t index = 0; index < count; ++index) {
        auto& hook = point.hooks[index];
        if (!hook.enabled || hook.mode != SHIP_NATIVE_HOOK_TRANSFORM) continue;
        if (payloadSize > small.size() && large.empty()) {
            try {
                large.resize(payloadSize);
                snapshot = large.data();
            } catch (...) {
                Log("sem memoria para snapshot do transform em '" + point.name + "'");
                break;
            }
        }
        if (payloadSize) std::memcpy(snapshot, payload, payloadSize);
        call.mode = SHIP_NATIVE_HOOK_TRANSFORM;
        call.phase = 0;
        if (RecordFailure(point, hook, Invoke(hook.owner, hook.callback, hook.user, call)) && payloadSize) {
            std::memcpy(payload, snapshot, payloadSize);
        }
    }

    Hook* replace = nullptr;
    for (std::size_t index = 0; index < count && !replace; ++index) {
        auto& hook = point.hooks[index];
        if (hook.enabled && hook.mode == SHIP_NATIVE_HOOK_REPLACE) replace = &hook;
    }
    ShipNativeStatus status;
    if (replace) {
        OriginalFrame frame{original, originalContext, false};
        call.mode = SHIP_NATIVE_HOOK_REPLACE;
        call.phase = 0;
        call.call_original = CallOriginal;
        call.original_context = &frame;
        status = Invoke(replace->owner, replace->callback, replace->user, call);
        call.call_original = nullptr;
        call.original_context = nullptr;
        if (RecordFailure(point, *replace, status) && !frame.called) {
            status = RunOriginal(call, original, originalContext);
        }
    } else {
        status = RunOriginal(call, original, originalContext);
    }

    observe(SHIP_NATIVE_HOOK_AFTER);

    if (--mDepth == 0 && (mDirty || !mPending.empty())) {
        try { Compact(); } catch (...) { mDirty = true; }
    }
    return status;
}

std::size_t NativeHookRegistry::HookCount() const noexcept {
    std::size_t total = mPending.size();
    for (const auto& entry : mPoints) {
        for (const auto& hook : entry.second.hooks) total += hook.removed ? 0 : 1;
    }
    return total;
}

std::vector<NativeHookInfo> NativeHookRegistry::Describe() const {
    std::vector<NativeHookInfo> result;
    auto add = [&result](const std::string& pointName, const Hook& hook) {
        result.push_back({hook.handle, hook.owner, pointName, hook.mode, hook.phase, hook.priority, hook.failures,
                          hook.enabled});
    };
    for (const auto& entry : mPoints) {
        if (entry.second.removed) continue;
        for (const auto& hook : entry.second.hooks) if (!hook.removed) add(entry.second.name, hook);
    }
    for (const auto& pending : mPending) add(mPoints.at(pending.first).name, pending.second);
    return result;
}

ShipNativeStatus NativeHookRegistry::SetPointListener(std::string_view owner, uint64_t point,
                                                      PointListener listener) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto found = mPoints.find(point);
    if (found == mPoints.end() || found->second.removed || found->second.owner != owner) {
        return SHIP_NATIVE_INVALID_ARGUMENT;
    }
    try {
        if (listener) {
            mListeners[point] = std::move(listener);
        } else {
            mListeners.erase(point);
        }
    } catch (...) {
        return SHIP_NATIVE_FAILURE;
    }
    return SHIP_NATIVE_OK;
}

std::vector<NativeHookEntry> NativeHookRegistry::Snapshot(uint64_t point) const {
    std::vector<NativeHookEntry> result;
    if (!OnThread()) return result;
    const auto found = mPoints.find(point);
    if (found == mPoints.end() || found->second.removed) return result;
    for (const auto& hook : found->second.hooks) {
        if (!hook.enabled || hook.removed) continue;
        result.push_back({hook.handle, hook.owner, hook.mode, hook.phase, hook.priority, hook.callback, hook.user});
    }
    return result;
}

ShipNativeStatus NativeHookRegistry::Disable(std::string_view pointOwner, uint64_t handle, std::string_view reason) {
    if (!OnThread()) return SHIP_NATIVE_FAILURE;
    const auto mapped = mHookPoints.find(handle);
    if (mapped == mHookPoints.end()) return SHIP_NATIVE_INVALID_ARGUMENT;
    const auto pointId = mapped->second;
    auto& point = mPoints.at(pointId);
    if (point.owner != pointOwner) return SHIP_NATIVE_INVALID_ARGUMENT;
    for (auto& hook : point.hooks) {
        if (hook.handle != handle || hook.removed) continue;
        if (!hook.enabled) return SHIP_NATIVE_OK;
        hook.enabled = false;
        --point.live;
        try {
            Log(std::string("hook ") + ModeName(hook.mode) + " de '" + hook.owner + "' em '" + point.name +
                "' desativado: " + std::string(reason));
        } catch (...) { /* Notify roda mesmo se o diagnóstico falhar. */ }
        Notify(pointId);
        return SHIP_NATIVE_OK;
    }
    return SHIP_NATIVE_INVALID_ARGUMENT; // pendente: ainda não está na lista
}

} // namespace ShipLua
