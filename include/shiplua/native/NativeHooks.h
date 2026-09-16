#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "shiplua/native/ship_native_abi.h"

namespace ShipLua {

struct NativeHookInfo {
    uint64_t handle = 0;
    std::string owner;
    std::string point;
    uint32_t mode = 0;
    uint32_t phase = 0;
    int32_t priority = 0;
    uint32_t failures = 0;
    bool enabled = true;
};

// Registro de hooks compartilhado por todos os providers de um host. Não é
// thread-safe: declaração, registro e despacho ocorrem na thread que o criou.
// Registrar ou remover durante um despacho é permitido e só vale no próximo.
class NativeHookRegistry {
  public:
    using Logger = std::function<void(const std::string&)>;
    static constexpr uint32_t kMaxFailures = 3;
    static constexpr std::size_t kMaxPoints = 1024;
    static constexpr std::size_t kMaxHooksPerOwner = 1024;

    explicit NativeHookRegistry(Logger logger = {});
    NativeHookRegistry(const NativeHookRegistry&) = delete;
    NativeHookRegistry& operator=(const NativeHookRegistry&) = delete;

    // owner vazio = host. Retorna OK e o id do ponto em `point`.
    ShipNativeStatus DeclarePoint(std::string_view owner, const ShipNativeHookPointSpec& spec,
                                  uint64_t* point);
    ShipNativeStatus Register(std::string_view owner, const ShipNativeHookSpec& spec, uint64_t* handle);
    ShipNativeStatus Unregister(std::string_view owner, uint64_t handle);
    // Remove hooks e pontos do dono. Hooks de outros donos em pontos removidos
    // também saem, com diagnóstico.
    void RemoveOwner(std::string_view owner);

    uint64_t FindPoint(std::string_view name, uint32_t version) const noexcept;
    bool IsOwnerOf(std::string_view owner, uint64_t point) const noexcept;
    // Barato: true quando existe hook habilitado no ponto.
    bool HasHooks(uint64_t point) const noexcept;
    ShipNativeStatus Dispatch(uint64_t point, void* payload, uint32_t payloadSize,
                              ShipNativeHookOriginalFn original, void* originalContext) noexcept;

    std::size_t HookCount() const noexcept;
    std::vector<NativeHookInfo> Describe() const;

  private:
    struct Hook {
        uint64_t handle = 0;
        std::string owner;
        uint32_t mode = 0;
        uint32_t phase = 0;
        int32_t priority = 0;
        ShipNativeHookFn callback = nullptr;
        void* user = nullptr;
        uint32_t failures = 0;
        bool enabled = true;
        bool removed = false;
    };
    struct Point {
        std::string owner;
        std::string name;
        uint32_t version = 0;
        uint32_t payloadSize = 0;
        uint32_t modes = 0;
        std::vector<Hook> hooks; // ordenados por prioridade desc e registro
        std::size_t live = 0;
        bool removed = false;
    };

    bool OnThread() const noexcept { return mThread == std::this_thread::get_id(); }
    void Log(const std::string& message) const;
    void Insert(Point& point, Hook hook);
    void Compact();
    bool RecordFailure(Point& point, Hook& hook, ShipNativeStatus status);
    void DisableHook(Point& point, Hook& hook);

    Logger mLogger;
    std::thread::id mThread = std::this_thread::get_id();
    std::map<uint64_t, Point> mPoints;
    std::unordered_map<uint64_t, uint64_t> mHookPoints; // handle -> ponto
    std::vector<std::pair<uint64_t, Hook>> mPending;    // registrados durante despacho
    uint64_t mNextPoint = 1;
    uint64_t mNextHook = 1;
    int mDepth = 0;
    bool mDirty = false;
};

} // namespace ShipLua
