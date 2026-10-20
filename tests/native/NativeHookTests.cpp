#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "shiplua/native/NativeHooks.h"

using namespace ShipLua;
namespace {
int failures = 0;
void Check(bool ok, const std::string& message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

struct Payload { int32_t value; };

struct Probe {
    std::vector<std::string>* trace = nullptr;
    std::string name;
    ShipNativeStatus status = SHIP_NATIVE_OK;
    int32_t add = 0;
    bool callOriginal = false;
    int calls = 0;
    NativeHookRegistry* registry = nullptr;
    std::function<void(Probe&)> during;
};

ShipNativeStatus SHIP_NATIVE_CALL Callback(void* user, const ShipNativeHookCall* call) {
    auto& probe = *static_cast<Probe*>(user);
    ++probe.calls;
    if (probe.trace) probe.trace->push_back(probe.name);
    if (probe.during) probe.during(probe);
    auto* payload = static_cast<Payload*>(call->payload);
    payload->value += probe.add;
    if (probe.callOriginal && call->call_original) {
        const auto status = call->call_original(call);
        if (status != SHIP_NATIVE_OK) return status;
        Check(call->call_original(call) == SHIP_NATIVE_FAILURE, "original only once per replace");
    }
    return probe.status;
}

struct Original {
    std::vector<std::string>* trace;
    int calls = 0;
    int32_t seen = 0;
};

ShipNativeStatus SHIP_NATIVE_CALL RunOriginal(const ShipNativeHookCall* call) {
    auto& original = *static_cast<Original*>(call->original_context);
    ++original.calls;
    original.seen = static_cast<Payload*>(call->payload)->value;
    if (original.trace) original.trace->push_back("original");
    static_cast<Payload*>(call->payload)->value *= 2;
    return SHIP_NATIVE_OK;
}

ShipNativeHookPointSpec PointSpec(const char* name = "test.point", uint32_t modes = 7) {
    return {sizeof(ShipNativeHookPointSpec), name, 1, sizeof(Payload), modes};
}

ShipNativeHookSpec HookSpec(Probe& probe, uint32_t mode, uint32_t phase = 0, int32_t priority = 0,
                            const char* name = "test.point") {
    return {sizeof(ShipNativeHookSpec), name, 1, sizeof(Payload), mode, phase, priority, Callback, &probe};
}
} // namespace

int main() {
    std::vector<std::string> logs;
    auto logger = [&logs](const std::string& text) { logs.push_back(text); };

    { // Declaração e validação.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        Check(registry.DeclarePoint("", PointSpec(), &point) == SHIP_NATIVE_OK && point, "declare");
        uint64_t duplicate = 0;
        Check(registry.DeclarePoint("other", PointSpec(), &duplicate) == SHIP_NATIVE_INVALID_ARGUMENT,
              "duplicate point name/version");
        auto bad = PointSpec();
        bad.modes = 8;
        Check(registry.DeclarePoint("", bad, &duplicate) == SHIP_NATIVE_INVALID_ARGUMENT, "unknown mode bits");
        bad = PointSpec("test.big");
        bad.payload_size = SHIP_NATIVE_MAX_BYTES + 1;
        Check(registry.DeclarePoint("", bad, &duplicate) == SHIP_NATIVE_INVALID_ARGUMENT, "payload limit");
        Check(registry.FindPoint("test.point", 1) == point && !registry.FindPoint("test.point", 2), "find by version");

        Probe probe;
        uint64_t handle = 0;
        Check(registry.Register("mod", HookSpec(probe, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "missing"), &handle) ==
                  SHIP_NATIVE_UNSUPPORTED, "missing point");
        auto spec = HookSpec(probe, SHIP_NATIVE_HOOK_OBSERVE);
        spec.minimum_payload_size = sizeof(Payload) + 1;
        Check(registry.Register("mod", spec, &handle) == SHIP_NATIVE_UNSUPPORTED, "payload too small for hook");
        Check(registry.Register("mod", HookSpec(probe, 3), &handle) == SHIP_NATIVE_INVALID_ARGUMENT,
              "mode must be single");
        Check(registry.Register("mod", HookSpec(probe, SHIP_NATIVE_HOOK_TRANSFORM, 1), &handle) ==
                  SHIP_NATIVE_INVALID_ARGUMENT, "phase only for observe");
        uint64_t observeOnly = 0;
        Check(registry.DeclarePoint("", PointSpec("test.observe", SHIP_NATIVE_HOOK_OBSERVE), &observeOnly) ==
                  SHIP_NATIVE_OK, "declare observe-only");
        Check(registry.Register("mod", HookSpec(probe, SHIP_NATIVE_HOOK_REPLACE, 0, 0, "test.observe"), &handle) ==
                  SHIP_NATIVE_UNSUPPORTED, "mode not accepted by point");
        Check(!registry.HasHooks(point), "no hooks yet");

        Original original{nullptr};
        Payload payload{3};
        Check(registry.Dispatch(point, &payload, sizeof(payload), RunOriginal, &original) == SHIP_NATIVE_OK &&
                  payload.value == 6 && original.calls == 1, "dispatch without hooks runs original");
        Check(registry.Dispatch(point, &payload, sizeof(payload) + 1, RunOriginal, &original) ==
                  SHIP_NATIVE_INVALID_ARGUMENT, "payload size must match point");

        std::thread([&] {
            Check(registry.Register("mod", HookSpec(probe, SHIP_NATIVE_HOOK_OBSERVE), &handle) == SHIP_NATIVE_FAILURE,
                  "foreign thread register");
            Check(registry.Dispatch(point, &payload, sizeof(payload), RunOriginal, &original) == SHIP_NATIVE_FAILURE,
                  "foreign thread dispatch");
        }).join();
    }

    { // Ordem completa: observe before, transform por prioridade, replace, observe after.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("", PointSpec(), &point);
        std::vector<std::string> trace;
        Probe before{&trace, "before"}, after{&trace, "after"};
        Probe low{&trace, "low", SHIP_NATIVE_OK, 1}, high{&trace, "high", SHIP_NATIVE_OK, 10};
        Probe tie{&trace, "tie", SHIP_NATIVE_OK, 100};
        Probe replace{&trace, "replace", SHIP_NATIVE_OK, 1000, true};
        uint64_t h = 0;
        Check(registry.Register("a", HookSpec(after, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER), &h) ==
                  SHIP_NATIVE_OK, "register after");
        registry.Register("a", HookSpec(low, SHIP_NATIVE_HOOK_TRANSFORM, 0, 1), &h);
        registry.Register("b", HookSpec(high, SHIP_NATIVE_HOOK_TRANSFORM, 0, 5), &h);
        registry.Register("c", HookSpec(tie, SHIP_NATIVE_HOOK_TRANSFORM, 0, 1), &h);
        registry.Register("b", HookSpec(before, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_BEFORE), &h);
        uint64_t replaceHandle = 0;
        Check(registry.Register("d", HookSpec(replace, SHIP_NATIVE_HOOK_REPLACE), &replaceHandle) == SHIP_NATIVE_OK,
              "register replace");
        Probe second{&trace, "second"};
        logs.clear();
        Check(registry.Register("e", HookSpec(second, SHIP_NATIVE_HOOK_REPLACE), &h) == SHIP_NATIVE_LIMIT &&
                  !logs.empty() && logs.back().find("'d'") != std::string::npos,
              "second replace refused naming owner");
        Check(registry.HookCount() == 6 && registry.HasHooks(point), "hook count");

        Original original{&trace};
        Payload payload{0};
        Check(registry.Dispatch(point, &payload, sizeof(payload), RunOriginal, &original) == SHIP_NATIVE_OK,
              "full dispatch status");
        const std::vector<std::string> expected{"before", "high", "low", "tie", "replace", "original", "after"};
        Check(trace == expected, "hook order");
        Check(original.seen == 1111 && payload.value == 2222, "payload chain through replace and original");

        Check(registry.Unregister("x", replaceHandle) == SHIP_NATIVE_INVALID_ARGUMENT, "only owner unregisters");
        Check(registry.Unregister("d", replaceHandle) == SHIP_NATIVE_OK, "owner unregisters");
        Check(registry.Unregister("d", replaceHandle) == SHIP_NATIVE_INVALID_ARGUMENT, "double unregister");
        Check(registry.Register("e", HookSpec(second, SHIP_NATIVE_HOOK_REPLACE), &h) == SHIP_NATIVE_OK,
              "replace free after unregister");
    }

    { // Falhas: transform restaurado, UNSUPPORTED não conta, 3 falhas desativam, replace cai no original.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("", PointSpec(), &point);
        Probe broken{nullptr, "broken", SHIP_NATIVE_FAILURE, 50};
        Probe declines{nullptr, "declines", SHIP_NATIVE_UNSUPPORTED, 7};
        Probe replace{nullptr, "replace", SHIP_NATIVE_FAILURE, 0};
        uint64_t h = 0;
        registry.Register("a", HookSpec(broken, SHIP_NATIVE_HOOK_TRANSFORM), &h);
        registry.Register("b", HookSpec(declines, SHIP_NATIVE_HOOK_TRANSFORM), &h);
        registry.Register("c", HookSpec(replace, SHIP_NATIVE_HOOK_REPLACE), &h);
        Original original{nullptr};
        for (int i = 0; i < 5; ++i) {
            Payload payload{1};
            const auto status = registry.Dispatch(point, &payload, sizeof(payload), RunOriginal, &original);
            Check(status == SHIP_NATIVE_OK && payload.value == 2, "failed transforms restored and original runs");
        }
        Check(broken.calls == 3 && replace.calls == 3, "hooks disabled after three failures");
        Check(declines.calls == 5, "UNSUPPORTED never disables");
        Check(original.calls == 5, "replace failure falls back to original");
        bool sawDisable = false;
        for (const auto& line : logs) sawDisable = sawDisable || line.find("desativado") != std::string::npos;
        Check(sawDisable, "disable is logged");
        const auto info = registry.Describe();
        Check(info.size() == 3, "disabled hooks still described");
    }

    { // Mutação durante despacho só vale no próximo; despacho aninhado.
        NativeHookRegistry registry(logger);
        uint64_t point = 0, inner = 0;
        registry.DeclarePoint("", PointSpec(), &point);
        registry.DeclarePoint("", PointSpec("test.inner"), &inner);
        Probe late{nullptr, "late", SHIP_NATIVE_OK, 5};
        uint64_t lateHandle = 0, selfHandle = 0;
        Probe adder{nullptr, "adder"};
        adder.during = [&](Probe&) {
            if (!lateHandle) {
                Check(registry.Register("m", HookSpec(late, SHIP_NATIVE_HOOK_TRANSFORM, 0, -1), &lateHandle) ==
                          SHIP_NATIVE_OK, "register during dispatch");
            }
            Payload nested{0};
            registry.Dispatch(inner, &nested, sizeof(nested), nullptr, nullptr);
        };
        Probe self{nullptr, "self", SHIP_NATIVE_OK, 1};
        self.during = [&](Probe&) {
            Check(registry.Unregister("m", selfHandle) == SHIP_NATIVE_OK, "unregister self during dispatch");
        };
        uint64_t adderHandle = 0;
        registry.Register("m", HookSpec(adder, SHIP_NATIVE_HOOK_OBSERVE), &adderHandle);
        registry.Register("m", HookSpec(self, SHIP_NATIVE_HOOK_TRANSFORM), &selfHandle);
        Probe innerProbe{nullptr, "inner"};
        uint64_t h = 0;
        registry.Register("m", HookSpec(innerProbe, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.inner"), &h);

        Payload payload{0};
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        Check(payload.value == 1 && late.calls == 0, "late hook not called in same dispatch");
        Check(innerProbe.calls == 1, "nested dispatch runs");
        payload.value = 0;
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        Check(payload.value == 5 && late.calls == 1 && self.calls == 1, "late active and self removed next time");
        Check(registry.Unregister("m", adderHandle) == SHIP_NATIVE_OK, "unregister adder");
    }

    { // Remoção por dono leva pontos e hooks alheios nesses pontos.
        NativeHookRegistry registry(logger);
        uint64_t hostPoint = 0, corePoint = 0;
        registry.DeclarePoint("", PointSpec(), &hostPoint);
        registry.DeclarePoint("core", PointSpec("core.point"), &corePoint);
        Check(registry.IsOwnerOf("core", corePoint) && !registry.IsOwnerOf("mod", corePoint), "point owner");
        Probe a{nullptr, "a"}, b{nullptr, "b"}, c{nullptr, "c"};
        uint64_t h = 0;
        registry.Register("mod", HookSpec(a, SHIP_NATIVE_HOOK_OBSERVE), &h);
        registry.Register("mod", HookSpec(b, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "core.point"), &h);
        registry.Register("core", HookSpec(c, SHIP_NATIVE_HOOK_OBSERVE), &h);
        logs.clear();
        registry.RemoveOwner("core");
        Check(!registry.FindPoint("core.point", 1) && registry.HookCount() == 1, "core point and its hooks removed");
        Check(!logs.empty() && logs.front().find("'mod'") != std::string::npos, "foreign hook removal logged");
        Check(registry.Unregister("mod", h) == SHIP_NATIVE_INVALID_ARGUMENT, "stale handle after removal");
        Payload payload{0};
        Check(registry.Dispatch(corePoint, &payload, sizeof(payload), nullptr, nullptr) == SHIP_NATIVE_OK,
              "dispatch on removed point is a no-op");
        registry.RemoveOwner("mod");
        Check(registry.HookCount() == 0 && registry.FindPoint("test.point", 1) == hostPoint,
              "host point survives mod removal");
    }

    { // Ponto com despacho próprio: ouvinte, cópia da lista e desativação pelo dono do ponto.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("", PointSpec("test.own", SHIP_NATIVE_HOOK_OBSERVE | SHIP_NATIVE_HOOK_TRANSFORM), &point);
        std::vector<uint64_t> notified;
        auto listener = [&notified](uint64_t id) { notified.push_back(id); };
        Check(registry.SetPointListener("mod", point, listener) == SHIP_NATIVE_INVALID_ARGUMENT,
              "listener only for the point owner");
        Check(registry.SetPointListener("", 999, listener) == SHIP_NATIVE_INVALID_ARGUMENT, "listener on unknown point");
        Check(registry.SetPointListener("", point, listener) == SHIP_NATIVE_OK, "listener set");

        Probe low{nullptr, "low"}, high{nullptr, "high"};
        uint64_t lowHandle = 0, highHandle = 0;
        registry.Register("mod", HookSpec(low, SHIP_NATIVE_HOOK_TRANSFORM, 0, 1, "test.own"), &lowHandle);
        registry.Register("mod", HookSpec(high, SHIP_NATIVE_HOOK_OBSERVE, SHIP_NATIVE_HOOK_AFTER, 5, "test.own"),
                          &highHandle);
        Check(notified == std::vector<uint64_t>{point, point}, "register notifies once per hook");
        const auto entries = registry.Snapshot(point);
        Check(entries.size() == 2 && entries[0].handle == highHandle && entries[1].handle == lowHandle,
              "snapshot keeps dispatch order");
        Check(entries.size() == 2 && entries[0].owner == "mod" && entries[0].mode == SHIP_NATIVE_HOOK_OBSERVE &&
                  entries[0].phase == SHIP_NATIVE_HOOK_AFTER && entries[0].priority == 5 &&
                  entries[0].callback == Callback && entries[0].user == &high,
              "snapshot copies the hook fields");

        notified.clear();
        logs.clear();
        Check(registry.Disable("mod", lowHandle, "teste") == SHIP_NATIVE_INVALID_ARGUMENT,
              "only the point owner disables");
        Check(registry.Disable("", 12345, "teste") == SHIP_NATIVE_INVALID_ARGUMENT, "unknown handle");
        Check(registry.Disable("", lowHandle, "falhou fora do Dispatch") == SHIP_NATIVE_OK, "owner disables");
        Check(notified == std::vector<uint64_t>{point} && registry.Snapshot(point).size() == 1,
              "disable notifies and leaves the snapshot");
        Check(!logs.empty() && logs.back().find("falhou fora do Dispatch") != std::string::npos,
              "disable reason logged");
        Check(registry.Disable("", lowHandle, "de novo") == SHIP_NATIVE_OK && notified.size() == 1,
              "disabling twice is a no-op");
        Check(registry.Unregister("mod", lowHandle) == SHIP_NATIVE_OK && notified.size() == 1,
              "unregister of a disabled hook does not notify again");

        uint64_t host = 0;
        registry.DeclarePoint("", PointSpec("test.host"), &host);
        Probe late{nullptr, "late"}, trigger{nullptr, "trigger"};
        uint64_t lateHandle = 0, triggerHandle = 0;
        trigger.during = [&](Probe&) {
            registry.Register("mod", HookSpec(late, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.own"), &lateHandle);
            Check(notified.size() == 1, "pending register does not notify during dispatch");
        };
        registry.Register("mod", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.host"), &triggerHandle);
        Payload payload{0};
        registry.Dispatch(host, &payload, sizeof(payload), nullptr, nullptr);
        Check(notified.size() == 2 && notified.back() == point && registry.Snapshot(point).size() == 2,
              "pending register notifies after the dispatch");

        notified.clear();
        registry.RemoveOwner("mod");
        Check(notified == std::vector<uint64_t>{point} && registry.Snapshot(point).empty(),
              "owner removal notifies the changed point once");

        Check(registry.SetPointListener("", point, [](uint64_t) { throw 1; }) == SHIP_NATIVE_OK, "listener replaced");
        Probe again{nullptr, "again"};
        uint64_t againHandle = 0;
        logs.clear();
        Check(registry.Register("mod", HookSpec(again, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.own"), &againHandle) ==
                      SHIP_NATIVE_OK &&
                  !logs.empty(),
              "a throwing listener is contained and logged");

        std::vector<NativeHookEntry> fromThread{NativeHookEntry{}};
        std::thread([&] { fromThread = registry.Snapshot(point); }).join();
        Check(fromThread.empty(), "snapshot off the registry thread is empty");
        Check(registry.SetPointListener("", point, {}) == SHIP_NATIVE_OK, "empty listener clears");
    }

    { // Retiradas síncronas, mesmo durante Dispatch de outro ponto.
        NativeHookRegistry registry(logger);
        uint64_t point = 0, host = 0;
        registry.DeclarePoint("", PointSpec("test.sync"), &point);
        registry.DeclarePoint("", PointSpec("test.trigger"), &host);
        unsigned notices = 0;
        Probe victim{nullptr, "victim"}, trigger{nullptr, "trigger"};
        uint64_t victimHandle = 0, triggerHandle = 0;
        registry.Register("victim", HookSpec(victim, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.sync"), &victimHandle);
        registry.SetPointListener("", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "synchronous snapshot is empty");
        });
        registry.Register("driver", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.trigger"), &triggerHandle);
        Payload payload{};
        for (int operation = 0; operation < 3; ++operation) {
            if (operation != 0) {
                registry.SetPointListener("", point, {});
                registry.Register("victim", HookSpec(victim, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.sync"),
                                  &victimHandle);
                registry.SetPointListener("", point, [&](uint64_t id) {
                    ++notices;
                    Check(registry.Snapshot(id).empty(), "retired hook is absent");
                });
            }
            notices = 0;
            trigger.during = [&](Probe&) {
                if (operation == 0) {
                    Check(registry.Unregister("victim", victimHandle) == SHIP_NATIVE_OK, "unregister during dispatch");
                } else if (operation == 1) {
                    Check(registry.Disable("", victimHandle, "teste síncrono") == SHIP_NATIVE_OK,
                          "disable during dispatch");
                } else {
                    registry.RemoveOwner("victim");
                }
                Check(notices == 1, "retirement notifies before returning inside Dispatch");
            };
            registry.Dispatch(host, &payload, sizeof(payload), nullptr, nullptr);
            Check(notices == 1, "no duplicate notification at the end");
        }
    }

    { // A terceira falha avisa só ao sair do Dispatch externo, sem pendentes nem remoções.
        NativeHookRegistry registry(logger);
        uint64_t point = 0, outer = 0;
        registry.DeclarePoint("", PointSpec("test.failure"), &point);
        registry.DeclarePoint("", PointSpec("test.outer"), &outer);
        Probe bad{nullptr, "bad"}, trigger{nullptr, "outer"};
        bad.status = SHIP_NATIVE_FAILURE;
        uint64_t badHandle = 0, triggerHandle = 0;
        registry.Register("bad", HookSpec(bad, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.failure"), &badHandle);
        unsigned notices = 0;
        registry.SetPointListener("", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "automatically disabled hook is absent");
        });
        Payload payload{};
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
        trigger.during = [&](Probe&) {
            registry.Dispatch(point, &payload, sizeof(payload), nullptr, nullptr);
            Check(notices == 0, "nested dispatch keeps the notification pending");
        };
        registry.Register("driver", HookSpec(trigger, SHIP_NATIVE_HOOK_OBSERVE, 0, 0, "test.outer"), &triggerHandle);
        registry.Dispatch(outer, &payload, sizeof(payload), nullptr, nullptr);
        Check(notices == 1 && bad.calls == 3, "outer dispatch flushes the automatic disable");
    }

    { // O ponto sai com o dono. Seu ouvinte ainda recebe a retirada antes da compactação.
        NativeHookRegistry registry(logger);
        uint64_t point = 0;
        registry.DeclarePoint("point.owner", PointSpec("test.drop"), &point);
        Probe visitor{nullptr, "visitor"};
        uint64_t handle = 0;
        registry.Register("visitor", HookSpec(visitor, SHIP_NATIVE_HOOK_TRANSFORM, 0, 0, "test.drop"), &handle);
        unsigned notices = 0;
        registry.SetPointListener("point.owner", point, [&](uint64_t id) {
            ++notices;
            Check(registry.Snapshot(id).empty(), "removed point has an empty snapshot");
        });
        registry.RemoveOwner("point.owner");
        registry.RemoveOwner("point.owner");
        Check(notices == 1 && !registry.FindPoint("test.drop", 1), "point retirement notifies once");
    }

    if (failures) { std::cerr << failures << " failures\n"; return 1; }
    std::cout << "Native hooks: order, modes, failures, deferred mutation and ownership passed.\n";
    return 0;
}
