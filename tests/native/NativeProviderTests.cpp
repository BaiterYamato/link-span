#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <utility>

#include <miniz.h>
#include "shiplua/host/ModHost.h"
#include "shiplua/manifest/ManifestParser.h"
#include "shiplua/native/NativeDiagnostics.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "FixtureService.h"

namespace fs = std::filesystem;
using namespace ShipLua;
namespace {
int failures = 0;
void Check(bool ok, const std::string& message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void Run(ModHost& host, const std::string& id, const std::string& source) {
    auto* runtime = host.GetRuntime(id);
    if (!runtime) { Check(false, "missing runtime " + id); return; }
    const auto result = runtime->DoString(source);
    Check(result.isOk(), result.message);
}
std::string ManifestText(const std::string& id) {
    return "id='" + id + "'\nname='Native test'\nversion='1.0.0'\napi='>=0.5 <1.0'\n"
           "entrypoint='main.lua'\ngames=['oot','mm']\n";
}
std::string ProviderText(const std::string& library = "provider/module.bin",
                         const std::string& abi = "1.0") {
    return "[provider]\nabi_version='" + abi + "'\nwin64='" + library + "'\nlinux64='" + library +
           "'\nmacos='" + library + "'\n";
}
void MakeMod(const fs::path& dir, const fs::path& library, const std::string& id,
             const std::string& source = "local s=require('ship'); assert(s.native.has('echo'))",
             const std::string& topLevel = {}, const std::string& abi = "1.0") {
    fs::create_directories(dir / "provider");
    fs::copy_file(library, dir / "provider/module.bin", fs::copy_options::overwrite_existing);
    std::ofstream(dir / "manifest.toml") << ManifestText(id) << topLevel << ProviderText("provider/module.bin", abi);
    std::ofstream(dir / "main.lua") << source;
}
void Package(const fs::path& dir, const fs::path& target) {
    mz_zip_archive zip{};
    Check(mz_zip_writer_init_file(&zip, target.string().c_str(), 0) != 0, "create package");
    for (const auto* name : {"manifest.toml", "main.lua", "provider/module.bin"}) {
        Check(mz_zip_writer_add_file(&zip, name, (dir/name).string().c_str(), nullptr, 0,
                                    MZ_BEST_SPEED) != 0, "package entry");
    }
    Check(mz_zip_writer_finalize_archive(&zip) != 0, "finalize package");
    mz_zip_writer_end(&zip);
}
}

int main(int argc, char** argv) {
    if (argc != 12) return 2;
    const auto root = fs::temp_directory_path() / ("shiplua-native-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root);
    for (const std::string game : {"oot", "mm"}) {
        FixtureService service{};
        NativeProviderPolicy policy;
        policy.enabled = true;
        policy.services = {{"test.lifecycle", 1, sizeof(service), &service}};
        LuaApiHostContext context;
        context.gameId = game;
        context.hostVersion = "1.0.0";
        MakeMod(root / "good", argv[1], "test.native");

        { // Código nativo não pode se autorizar no manifesto.
            ModHost disabled(context);
            auto result = disabled.LoadModFromDirectory((root/"good").string());
            Check(result.code == ErrorCode::PermissionDenied && service.initialized == 0,
                  "default rejection before init");
            auto deniedPolicy = policy;
            deniedPolicy.authorize = [](const Manifest&, const fs::path& path) {
                Check(path.is_absolute(), "authorizer receives resolved path"); return false;
            };
            ModHost denied(context, {}, deniedPolicy);
            result = denied.LoadModFromDirectory((root/"good").string());
            Check(result.code == ErrorCode::PermissionDenied && service.initialized == 0, "authorization denial");
        }
        {
            ModHost host(context, {}, policy);
            service.host = &host;
            service.unload = [](void* pointer) -> int {
                auto& active = *static_cast<ModHost*>(pointer);
                const auto result = active.UnloadMod("test.native");
                active.UnloadAll(); // deve terminar e preservar o callback ativo
                return result.code == ErrorCode::InvalidState && active.IsLoaded("test.native");
            };
            auto result = host.LoadModFromDirectory((root/"good").string());
            Check(result.isOk(), "load: " + result.message);
            Run(host, "test.native", R"lua(
                local n=require('ship').native
                assert(n.call('echo','hello')=='hello')
                assert(n.call('echo','unload')=='denied')
                assert(n.call('echo','a\0b')=='a\0b')
                assert(n.call('echo','count')=='1')
                assert(n.call('echo','late')=='denied' and not n.has('late'))
                local x,e=n.call('other_mod.echo','x'); assert(x==nil and e.code=='invalid_handle')
                x,e=n.call('echo',42); assert(x==nil and e.code=='invalid_argument')
                x,e=n.call('echo',string.rep('x',65537)); assert(x==nil and e.code=='resource_limit')
                assert(#n.call('echo',string.rep('x',65536))==65536)
                x,e=n.call('echo','limit'); assert(x==nil and e.code=='resource_limit')
                assert(n.call('echo','after-limit')=='after-limit')
            )lua");
            MakeMod(root/"second", argv[1], "test.second");
            Check(host.LoadModFromDirectory((root/"second").string()).isOk(), "second instance");
            Run(host, "test.second", "assert(require('ship').native.call('echo','count')=='1')");
            Check(host.UnloadMod("test.native").isOk(), "unload");
            Run(host, "test.second", "assert(require('ship').native.call('echo','count')=='2')");
            Check(host.LoadModFromDirectory((root/"good").string()).isOk(), "reload");
            Run(host, "test.native", "assert(require('ship').native.call('echo','count')=='1')");
            Run(host, "test.native", R"lua(
                local n=require('ship').native
                for i=1,3 do local x,e=n.call('echo','fail'); assert(x==nil and e.code=='host_failure') end
                local x,e=n.call('echo','x'); assert(x==nil and e.code=='invalid_state' and not n.has('echo'))
            )lua");
            ModHost moved(std::move(host));
            Run(moved, "test.second", "assert(require('ship').native.call('echo','count')=='3')");
        }
        Check(service.initialized == service.shutdown, "balanced instance cleanup after moves/reloads");
        {
            const auto mods = root / ("core-order-" + game);
            MakeMod(mods / "z-core", argv[7], "test.core", "loaded=true",
                    "kind='core_extension'\nload_phase='pre_game'\n", "1.1");
            MakeMod(mods / "a-consumer", argv[8], "test.consumer",
                    "assert(require('ship').native.call('read_core','')=='42')", {}, "1.1");
            ModHost host(context, {}, policy);
            const auto loaded = host.LoadModsFromRoot(mods, root / ("core-cache-" + game));
            Check(loaded.isOk(), "core root load: " + loaded.message);
            Check(loaded.isOk() && loaded.value->loadedIds ==
                      std::vector<std::string>{"test.core", "test.consumer"},
                  "pre_game core loads before lexically earlier runtime consumer");
            Check(host.UnloadMod("test.core").code == ErrorCode::InvalidState,
                  "core unload blocked while native consumer holds its service");
            Check(host.UnloadMod("test.consumer").isOk(), "consumer unload");
            Check(host.UnloadMod("test.core").isOk(), "core unload after consumer");
        }
        {
            const auto mods = root / ("hooks-" + game);
            MakeMod(mods / "z-core", argv[9], "test.hooks.core", "loaded=true",
                    "kind='core_extension'\nload_phase='pre_game'\n", "1.2");
            MakeMod(mods / "a-consumer", argv[10], "test.hooks.consumer", "loaded=true", {}, "1.2");
            auto hookPolicy = policy;
            hookPolicy.hooks = std::make_shared<NativeHookRegistry>();
            const auto& hooks = *hookPolicy.hooks;
            ModHost host(context, {}, hookPolicy);
            const auto loaded = host.LoadModsFromRoot(mods, root / ("hooks-cache-" + game));
            Check(loaded.isOk() && loaded.value->loadedIds ==
                      std::vector<std::string>{"test.hooks.core", "test.hooks.consumer"},
                  "hook core and consumer load");
            Check(hooks.HookCount() == 2, "consumer hooks registered in init");
            Run(host, "test.hooks.core", "assert(require('ship').native.call('run','5')=='12')");
            Run(host, "test.hooks.consumer", R"lua(
                local n=require('ship').native
                assert(n.call('hooks','observed')=='12')
                assert(n.call('hooks','declared')=='2')
                assert(n.call('hooks','replace')=='0')
                assert(n.call('hooks','replace')=='4')
            )lua");
            Run(host, "test.hooks.core", "assert(require('ship').native.call('run','5')=='112')");
            Run(host, "test.hooks.consumer", "assert(require('ship').native.call('hooks','unhook')=='0')");
            Run(host, "test.hooks.core", "assert(require('ship').native.call('run','5')=='12')");
            Run(host, "test.hooks.consumer", "assert(require('ship').native.call('hooks','replace')=='0')");
            Check(host.UnloadMod("test.hooks.consumer").isOk() && hooks.HookCount() == 0,
                  "unload removes consumer hooks");
            Run(host, "test.hooks.core", "assert(require('ship').native.call('run','5')=='10')");
            Check(host.LoadModFromDirectory((mods / "a-consumer").string()).isOk() && hooks.HookCount() == 2,
                  "consumer reload registers again");
            Check(host.UnloadMod("test.hooks.core").isOk(), "core unload with hooked point");
            Check(hooks.HookCount() == 0 && !hooks.FindPoint("test.hooks.value", 1),
                  "core unload drops its point and foreign hooks");
        }
        {
            // Escape hatch ABI 1.3: host falso de patches, fingerprint conferido pelo manifesto.
            struct FakePatches {
                int installed = 0;
                int removed = 0;
                uint64_t next = 1;
            } patches;
            auto hatch = std::make_shared<NativeEscapeHatch>();
            hatch->fingerprint = [] { return std::string(64, 'a'); };
            hatch->resolve = [](const char* name, uintptr_t* address) -> ShipNativeStatus {
                if (std::string(name) != "Target") return SHIP_NATIVE_UNSUPPORTED;
                *address = 0x1000;
                return SHIP_NATIVE_OK;
            };
            hatch->install = [&patches](uintptr_t target, void*, void** original, uint64_t* id) -> ShipNativeStatus {
                if (target != 0x1000) return SHIP_NATIVE_INVALID_ARGUMENT;
                ++patches.installed;
                *original = reinterpret_cast<void*>(target);
                *id = patches.next++;
                return SHIP_NATIVE_OK;
            };
            hatch->remove = [&patches](uint64_t) -> ShipNativeStatus {
                ++patches.removed;
                return SHIP_NATIVE_OK;
            };
            auto escapePolicy = policy;
            escapePolicy.escapeHatch = hatch;
            // Estado por mod nos serviços do host é devolvido depois de os patches saírem.
            std::vector<std::string> unloadedOwners;
            escapePolicy.onProviderUnload = [&](std::string_view owner) {
                unloadedOwners.emplace_back(std::string(owner) + (patches.removed == 1 ? ":sem-patch" : ":com-patch"));
            };
            const std::string core = "kind='core_extension'\nload_phase='pre_game'\n";
            const auto escapeRoot = root / ("escape-" + game);
            MakeMod(escapeRoot / "allowed", argv[11], "test.escape.allowed", "loaded=true", core, "1.3");
            {
                std::ofstream manifest(escapeRoot / "allowed/manifest.toml", std::ios::app);
                manifest << "host_fingerprints=['" << std::string(64, 'A') << "']\n";
            }
            MakeMod(escapeRoot / "denied", argv[11], "test.escape.denied", "loaded=true", core, "1.3");
            {
                std::ofstream manifest(escapeRoot / "denied/manifest.toml", std::ios::app);
                manifest << "host_fingerprints=['" << std::string(64, 'b') << "']\n";
            }
            MakeMod(escapeRoot / "plain", argv[11], "test.escape.plain", "loaded=true", {}, "1.3");
            {
                ModHost host(context, {}, escapePolicy);
                Check(host.LoadModFromDirectory((escapeRoot / "allowed").string()).isOk(), "escape allowed load");
                Run(host, "test.escape.allowed", R"lua(
                    local r=require('ship').native.call('report','')
                    assert(r=='fp=0:64:aaaa res=0 ins=0 orig=1', r)
                )lua");
                Check(patches.installed == 1 && patches.removed == 0, "patch installed with matching fingerprint");
                Check(host.UnloadMod("test.escape.allowed").isOk() && patches.removed == 1,
                      "unload removes the core extension patch");
                Check(unloadedOwners.size() == 1 && unloadedOwners[0] == "test.escape.allowed:sem-patch",
                      "unload notifies the host after removing patches");
                Check(host.LoadModFromDirectory((escapeRoot / "denied").string()).isOk(), "escape denied load");
                Run(host, "test.escape.denied", R"lua(
                    local r=require('ship').native.call('report','')
                    assert(r=='fp=0:64:aaaa res=2 ins=2 orig=0', r)
                )lua");
                Check(host.LoadModFromDirectory((escapeRoot / "plain").string()).isOk(), "escape plain mod load");
                Run(host, "test.escape.plain", R"lua(
                    local r=require('ship').native.call('report','')
                    assert(r=='fp=0:64:aaaa res=2 ins=2 orig=0', r)
                )lua");
                Check(patches.installed == 1, "no patch without permission");
            }
            {
                ModHost host(context, {}, policy);
                MakeMod(escapeRoot / "nohatch", argv[11], "test.escape.nohatch", "loaded=true", core, "1.3");
                Check(host.LoadModFromDirectory((escapeRoot / "nohatch").string()).isOk(), "host without hatch");
                Run(host, "test.escape.nohatch", R"lua(
                    local r=require('ship').native.call('report','')
                    assert(r=='fp=2:0: res=2 ins=2 orig=0', r)
                )lua");
            }
            {
                // COREEXT-010: lote de ativação em volta do init e do unload; fora dele, cada desvio vale na
                // hora. Minúscula = dentro do lote.
                std::string log;
                int depth = 0;
                bool failInit = false;
                bool failCommit = false;
                auto batched = std::make_shared<NativeEscapeHatch>(*hatch);
                batched->resolve = [&failInit](const char* name, uintptr_t* address) -> ShipNativeStatus {
                    const std::string text(name);
                    if (text != "Target" && !(failInit && text == "FailAfterPatch")) return SHIP_NATIVE_UNSUPPORTED;
                    *address = 0x1000;
                    return SHIP_NATIVE_OK;
                };
                batched->install = [&](uintptr_t target, void*, void** original, uint64_t* id) -> ShipNativeStatus {
                    if (target != 0x1000) return SHIP_NATIVE_INVALID_ARGUMENT;
                    log += depth ? 'i' : 'I';
                    *original = reinterpret_cast<void*>(target);
                    *id = patches.next++;
                    return SHIP_NATIVE_OK;
                };
                batched->remove = [&](uint64_t) -> ShipNativeStatus {
                    log += depth ? 'r' : 'R';
                    return SHIP_NATIVE_OK;
                };
                batched->beginBatch = [&] {
                    ++depth;
                    log += '[';
                };
                batched->commitBatch = [&]() -> ShipNativeStatus {
                    --depth;
                    log += ']';
                    return failCommit ? SHIP_NATIVE_FAILURE : SHIP_NATIVE_OK;
                };
                auto batchPolicy = policy;
                batchPolicy.escapeHatch = batched;
                batchPolicy.onProviderUnload = [&](std::string_view) { log += 'u'; };
                ModHost host(context, {}, batchPolicy);
                Check(host.LoadModFromDirectory((escapeRoot / "allowed").string()).isOk() && log == "[i]",
                      "init patches activate in one batch");
                Run(host, "test.escape.allowed", R"lua(
                    local r=require('ship').native.call('patch_now','')
                    assert(r=='0', r)
                )lua");
                Check(log == "[i]I", "patch after init is not batched");
                Check(host.UnloadMod("test.escape.allowed").isOk() && log == "[i]I[rr]u",
                      "unload removes every patch in one batch before notifying the host");
                log.clear();
                failInit = true;
                Check(!host.LoadModFromDirectory((escapeRoot / "allowed").string()).isOk() && log == "[ir]u",
                      "failed init removes its patches before the batch commits");
                log.clear();
                Check(host.LoadModFromDirectory((escapeRoot / "denied").string()).isOk() && log.empty(),
                      "no batch without permission");
                log.clear();
                failInit = false;
                failCommit = true;
                Check(!host.LoadModFromDirectory((escapeRoot / "allowed").string()).isOk() && log == "[i][r]u",
                      "failed activation fails the load and removes the patch in another batch");
            }
        }
        {
            // Proteção de boot e atribuição de crash (COREEXT-009).
            const auto guardRoot = root / ("guard-" + game);
            const auto guardCache = root / ("guard-cache-" + game);
            MakeMod(guardRoot / "native", argv[1], "test.guard");
            auto readText = [](const fs::path& path) {
                std::ifstream input(path, std::ios::binary);
                return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            };
            {
                ModHost host(context, {}, policy);
                host.EnableBootGuard(guardRoot);
                const auto loaded = host.LoadModsFromRoot(guardRoot, guardCache);
                Check(loaded.isOk() && loaded.value->loadedIds == std::vector<std::string>{"test.guard"},
                      "guard loads healthy native mod");
                Check(readText(guardRoot / ".shiplua-boot") == "running\t\n", "marker says running after load");
                char report[4096];
                size_t used = 0;
#ifdef _WIN32
                const auto dll = LoadLibraryW((guardRoot / "native/provider/module.bin").c_str());
                const auto query = dll ? reinterpret_cast<uintptr_t>(GetProcAddress(dll, "ShipNative_Query")) : 0;
                char owner[128] = {};
                Check(query && FindNativeModuleOwner(query, owner, sizeof(owner)) && std::string(owner) == "test.guard",
                      "module map attributes the provider DLL");
                if (dll) FreeLibrary(dll);
#endif
                {
                    const NativeActivityScope outer("test.guard", "call", "echo");
                    const NativeActivityScope inner("test.other", "hook", "oot.actor.update");
                    used = WriteNativeCrashReport(report, sizeof(report));
                }
                const std::string text(report, used);
                Check(text.find("mod 'test.other' em hook oot.actor.update") != std::string::npos &&
                          text.find("mod 'test.guard' em call echo") != std::string::npos &&
                          text.find("'test.other' sera desativado") != std::string::npos,
                      "crash report lists activity and blames the innermost owner: " + text);
                Check(readText(guardRoot / ".shiplua-boot") == "crashed\ttest.other\n", "crash marks the blamed mod");
                used = WriteNativeCrashReport(report, 16);
                Check(used <= 16, "crash report respects capacity");
            }
            {
                // Simula o marcador de um boot que caiu durante o init do mod.
                std::ofstream(guardRoot / ".shiplua-boot", std::ios::binary) << "loading\ttest.guard\n";
                ModHost host(context, {}, policy);
                host.EnableBootGuard(guardRoot);
                const auto loaded = host.LoadModsFromRoot(guardRoot, guardCache);
                Check(loaded.isOk() && loaded.value->loadedIds.empty() &&
                          loaded.value->rejected.count("test.guard") &&
                          loaded.value->rejected.at("test.guard").find("prote") != std::string::npos,
                      "mod that crashed the boot is disabled");
                const auto disabled = readText(guardRoot / ".shiplua-disabled");
                Check(disabled.find("test.guard\t") != std::string::npos &&
                          disabled.find("test.other\t") == std::string::npos,
                      "disabled list names the mod (the earlier crash marker was overwritten by the new boot)");
                host.ConfirmBoot();
                Check(!fs::exists(guardRoot / ".shiplua-boot"), "confirm removes marker");
            }
            {
                fs::remove(guardRoot / ".shiplua-disabled");
                ModHost host(context, {}, policy);
                host.EnableBootGuard(guardRoot);
                const auto loaded = host.LoadModsFromRoot(guardRoot, guardCache);
                Check(loaded.isOk() && loaded.value->loadedIds == std::vector<std::string>{"test.guard"},
                      "removing the disabled line re-enables the mod");
                host.ConfirmBoot();
                char report[1024];
                const NativeActivityScope activity("test.guard", "call", "echo");
                const auto used = WriteNativeCrashReport(report, sizeof(report));
                Check(std::string(report, used).find("sera desativado") == std::string::npos &&
                          !fs::exists(guardRoot / ".shiplua-boot"),
                      "after confirm a crash is reported but no mod is disabled");
            }
        }
        {
            const auto mods = root / ("core-duplicate-" + game);
            const std::string fields = "kind='core_extension'\nload_phase='pre_game'\n";
            MakeMod(mods / "one", argv[7], "test.core.one", "loaded=true", fields, "1.1");
            MakeMod(mods / "two", argv[7], "test.core.two", "loaded=true", fields, "1.1");
            ModHost host(context, {}, policy);
            const auto loaded = host.LoadModsFromRoot(mods, root / ("duplicate-cache-" + game));
            Check(loaded.isOk() && loaded.value->loadedIds ==
                      std::vector<std::string>{"test.core.one"},
                  "first core service owner remains loaded");
            Check(loaded.isOk() && loaded.value->rejected.contains("test.core.two"),
                  "duplicate core service is isolated and rejected");
        }
        {
            const auto mods = root / ("core-rollback-" + game);
            MakeMod(mods / "z-core", argv[7], "test.core.rollback", "error('core rollback')",
                    "kind='core_extension'\nload_phase='pre_game'\n", "1.1");
            MakeMod(mods / "a-consumer", argv[8], "test.consumer.rollback",
                    "assert(require('ship').native.call('read_core','')=='42')", {}, "1.1");
            ModHost host(context, {}, policy);
            const auto loaded = host.LoadModsFromRoot(mods, root / ("rollback-cache-" + game));
            Check(loaded.isOk() && loaded.value->rejected.contains("test.core.rollback"),
                  "failed core entrypoint is rejected");
            Check(loaded.isOk() && loaded.value->rejected.contains("test.consumer.rollback") &&
                      host.Count() == 0,
                  "failed core publishes no residual service to later mods");
        }
        {
            const auto staging = root / ("core-package-staging-" + game);
            const auto mods = root / ("core-packages-" + game);
            const auto cache = root / ("core-package-cache-" + game);
            fs::create_directories(mods);
            MakeMod(staging / "core", argv[7], "test.packaged.core", "loaded=true",
                    "kind='core_extension'\nload_phase='pre_game'\n", "1.1");
            MakeMod(staging / "consumer", argv[8], "test.packaged.consumer",
                    "assert(require('ship').native.call('read_core','')=='42')", {}, "1.1");
            Package(staging / "core", mods / "z-core.zip");
            Package(staging / "consumer", mods / "a-consumer.zip");
            ModHost host(context, {}, policy);
            const auto loaded = host.LoadModsFromRoot(mods, cache);
            Check(loaded.isOk() && loaded.value->loadedIds ==
                      std::vector<std::string>{"test.packaged.core", "test.packaged.consumer"},
                  "core extension and consumer load as independent ZIP mods");
            host.UnloadAll();
            Check(fs::is_empty(cache), "packaged core and consumer release extraction cache");
        }
        for (int index = 2; index <= 6; ++index) {
            MakeMod(root/"bad", argv[index], "test.bad");
            ModHost host(context, {}, policy);
            const auto result = host.LoadModFromDirectory((root/"bad").string());
            Check(!result.isOk() && host.Count() == 0, "invalid ABI/init/duplicate/export rolls back");
            Check(service.initialized == service.shutdown, "partial init cleanup");
        }
        MakeMod(root/"bad", argv[1], "test.bad", "error('rollback')");
        {
            ModHost host(context, {}, policy);
            const auto result = host.LoadModFromDirectory((root/"bad").string());
            Check(result.code == ErrorCode::ScriptFailure && host.Count() == 0, "Lua failure rollback");
        }
        Check(service.initialized == service.shutdown, "Lua failure invokes shutdown");
        {
            ModHost host(context, {}, policy);
            MakeMod(root/"gc", argv[1], "test.gc", R"lua(
                local n=require('ship').native
                finalizer=setmetatable({}, {__gc=function() n.call('echo','gc') end})
            )lua");
            Check(host.LoadModFromDirectory((root/"gc").string()).isOk(), "finalizer mod load");
            const auto before = service.calls;
            Check(host.UnloadMod("test.gc").isOk(), "unload with captured native closure");
            Check(service.calls == before+1, "finalizer calls provider while DLL and context remain alive");
            MakeMod(root/"gc", argv[1], "test.gc", R"lua(
                local n=require('ship').native
                finalizer=setmetatable({}, {__gc=function() n.call('echo','gc-rollback') end})
                error('rollback with finalizer')
            )lua");
            const auto rollbackBefore = service.calls;
            Check(!host.LoadModFromDirectory((root/"gc").string()).isOk(), "rollback with finalizer");
            Check(service.calls == rollbackBefore+1, "rollback finalizer runs before provider shutdown");
        }
        {
            Package(root/"good", root/"native.shipmod");
            ModHost host(context, {}, policy);
            Check(host.LoadModFromPackage((root/"native.shipmod").string(), (root/"cache").string()).isOk(),
                  "load packaged native mod");
            Run(host, "test.native", "assert(require('ship').native.call('echo','package')=='package')");
            Check(host.UnloadMod("test.native").isOk(), "package unload releases DLL before cleanup");
            Check(fs::is_empty(root/"cache"), "package cache empty after unload");
        }
        {
            ModHost host(context, {}, policy);
            fs::create_directories(root/"plain");
            std::ofstream(root/"plain/manifest.toml") << ManifestText("test.plain");
            std::ofstream(root/"plain/main.lua") << "local n=require('ship').native; assert(not n.has('echo')); "
                "local x,e=n.call('echo',''); assert(x==nil and e.code=='unsupported')";
            Check(host.LoadModFromDirectory((root/"plain").string()).isOk(), "plain Lua has no native privileges");
            const auto parsed = ParseManifestString(ManifestText("test.native")+ProviderText());
            Check(host.LoadModFromManifestAndSource(*parsed.value, "").code == ErrorCode::InvalidArgument,
                  "no native loading from source without directory");
        }
        Check(service.initialized == service.shutdown, "all providers shut down");
        {
            auto parsed = ParseManifestString(ManifestText("test.native")+ProviderText());
            auto provider = NativeProvider::Load(*parsed.value, root/"good", policy);
            Check(provider.isOk(), "direct provider load for thread guard");
            if (provider.isOk()) {
                NativeCallResult wrongThread;
                std::thread worker([&] {
                    char response[16];
                    wrongThread = (*provider.value)->Call("echo", "x", 1, response, sizeof(response));
                });
                worker.join();
                Check(wrongThread.code == ErrorCode::InvalidState, "reject native call on foreign thread");
            }
            parsed.value->provider->win64 = "provider/missing.bin";
            parsed.value->provider->linux64 = "provider/missing.bin";
            parsed.value->provider->macos = "provider/missing.bin";
            Check(!NativeProvider::Load(*parsed.value, root/"good", policy).isOk(), "reject missing library");
        }
        {
            std::error_code error;
            fs::create_directory_symlink(root/"good", root/"linked", error);
            if (!error) {
                ModHost host(context, {}, policy);
                const auto result = host.LoadModFromDirectory((root/"linked").string());
                Check(result.code == ErrorCode::PermissionDenied, "reject linked mod root");
                fs::remove(root/"linked");
            } else {
                std::cout << "Symlink test skipped: OS denied creating the fixture (" << error.message() << ").\n";
            }
        }
    }
    for (const std::string path : {"../escape.dll", "/absolute.dll", "C:/evil.dll", "a/../../evil.dll", "a\\evil.dll", "a/./evil.dll"}) {
        Check(!ParseManifestString(ManifestText("test.invalid")+ProviderText(path)).isOk(), "reject path " + path);
    }
    Check(!ParseManifestString(ManifestText("test.invalid")+"provider='oops'\n").isOk(), "provider type");
    Check(!ParseManifestString(ManifestText("test.invalid")+ProviderText()+"unknown='x'\n").isOk(), "unknown provider field");
    Check(!ParseManifestString(ManifestText("test.invalid")+"[provider]\nabi_version='1.0'\n").isOk(), "missing binary");
    Check(!ParseManifestString(ManifestText("test.invalid")+
        "kind='core_extension'\nload_phase='pre_game'\n"+ProviderText()).isOk(),
        "core extension requires ABI 1.1");
    Check(!ParseManifestString(ManifestText("test.invalid")+ProviderText("provider/module.bin", "1.3")+
        "host_fingerprints=['" + std::string(64, 'a') + "']\n").isOk(),
        "host_fingerprints only for core extensions");
    Check(!ParseManifestString(ManifestText("test.invalid")+"kind='core_extension'\nload_phase='pre_game'\n"+
        ProviderText("provider/module.bin", "1.3")+"host_fingerprints=['xyz']\n").isOk(),
        "host_fingerprints must be SHA-256");
    Check(!ParseManifestString(ManifestText("test.invalid")+
        "load_phase='pre_game'\n"+ProviderText()).isOk(),
        "pre_game phase is reserved for core extensions");
    for (const std::string range : {">=0.4.1", "*", ">0.4.999", ">=0.5.0-alpha"}) {
        auto source = ManifestText("test.invalid") + ProviderText();
        source.replace(source.find(">=0.5 <1.0"), std::string(">=0.5 <1.0").size(), range);
        Check(!ParseManifestString(source).isOk(), "native range excludes legacy versions: " + range);
    }
    // Arquivos de teste exclusivamente sob diretório temporário único desta execução.
    fs::remove_all(root);
    if (failures) { std::cerr << failures << " failures\n"; return 1; }
    std::cout << "Native providers: directory, package, ABI, services, isolation and rollback passed (oot/mm).\n";
    return 0;
}
