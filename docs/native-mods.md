**English** · [Português (Brasil)](native-mods.pt-BR.md)

# Build a native OoT mod

Build a DLL against the SDK for the Link-Span OoT release you will run, package it with its manifest, and install the package in `mods/`. This guide documents **Shipwright OoT on Windows x64**. It does not promise native MM support. For the generic provider contract, see [Native providers](native-providers.md).

## Get the matching SDK

Download `LinkSpan-OoT-SDK-<version>-layout-<first-8-id-characters>-Win64.zip` from the matching [Link-Span release](https://github.com/BaiterYamato/link-span/releases). Extract it outside the game installation. The SDK ZIP is a development kit; do not place it in `mods/`.

Read `linkspan-sdk.json` before configuring. It records the release version, full `ootLayoutId`, `nativeProviderAbi`, `hostSha256`, configuration and compiler metadata. Use **the exact MSVC version in `compiler.version`**, Windows x64, CMake 3.26+, Python 3.11+, C++20 and Release `/MT` (`MSVC_RUNTIME_LIBRARY MultiThreaded`). Use an x64 Developer PowerShell and select that MSVC toolset if needed. The CMake package rejects a different compiler version; installing any Visual Studio 2022 version is not enough.

The bundle provides public headers, `include/oot_layout_id.h`, a relocatable CMake configuration, a compatibility `OotNativeSdk.cmake`, examples and packaging/inspection tools. `checksums.sha256` records its internal hashes. There are no host import libraries, game executable, ROM, local asset archives or saves. Keep the exported compile definitions and options. Do not link to `soh`, Link-Span core, Lua or libultraship.

`examples/provider-example` is a minimal compatibility probe. `examples/example` contains Dynamic Movement Remake source but omits its HUD textures; compiling it does not produce proof of a complete playable package. The bundle builder also copies Heart Seeds into `examples/heart-seeds`; it requires NEI. Read the actual bundle metadata and author guides before using an example.

## First build: use the SDK probe

Assume the SDK is extracted as `LinkSpan-OoT-SDK` in your current directory:

```powershell
$sdk = (Resolve-Path .\LinkSpan-OoT-SDK).Path
cmake -S "$sdk/examples/provider-example" -B .\probe-build -A x64 "-DCMAKE_PREFIX_PATH=$sdk"
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
cmake --build .\probe-build --config Release
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
```

The example's CMake file uses the package configuration and header-only target:

```cmake
cmake_minimum_required(VERSION 3.26)
project(LinkSpanProviderExample LANGUAGES CXX)
find_package(LinkSpanOotSdk CONFIG REQUIRED)
add_library(linkspan_sdk_probe SHARED provider.cpp)
target_link_libraries(linkspan_sdk_probe PRIVATE LinkSpan::OotSdk)
set_target_properties(linkspan_sdk_probe PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" upper)
    set_target_properties(linkspan_sdk_probe PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${upper} "${CMAKE_BINARY_DIR}/mod/provider")
endforeach()
configure_file(manifest.toml "${CMAKE_BINARY_DIR}/mod/manifest.toml" COPYONLY)
configure_file(main.lua "${CMAKE_BINARY_DIR}/mod/main.lua" COPYONLY)
```

`LinkSpan::OotSdk` is an INTERFACE target for headers, definitions and compile options. It does not link the host. Copy `examples/provider-example` into your own project before changing it, then give the target, manifest ID, DLL path and package their own names.

## Minimal manifest and init

The probe manifest is:

```toml
id = "example.linkspan_sdk_probe"
name = "Link-Span SDK Probe"
version = "0.1.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
games = ["oot"]

[provider]
abi_version = "1.2"
win64 = "provider/linkspan_sdk_probe.dll"
```

Keep a root `main.lua`; the probe's entrypoint just requires `ship`, with no gameplay mutation. Its native implementation is:

```cpp
#include <cstring>
#include "shiplua/native/ship_native_abi.h"
#include "oot_engine.h"
#include "oot_layout_id.h"
#include "z64.h"

static_assert(sizeof(void*) == 8, "Windows x64 requerido");

static ShipNativeStatus SHIP_NATIVE_CALL Init(const ShipNativeRuntime* runtime, void** instance) {
    if (!instance) return SHIP_NATIVE_FAILURE;
    *instance = nullptr;
    if (!runtime || runtime->size < sizeof(ShipNativeRuntime) || !runtime->get_service)
        return SHIP_NATIVE_UNSUPPORTED;
    const auto* engine = static_cast<const ShipOotEngineV1*>(runtime->get_service(
        runtime->context, LINKSPAN_OOT_ENGINE_SERVICE, LINKSPAN_OOT_ENGINE_VERSION,
        sizeof(ShipOotEngineV1)));
    if (!engine || engine->size < sizeof(ShipOotEngineV1) || !engine->layout_id ||
        std::strcmp(engine->layout_id, LINKSPAN_OOT_LAYOUT_ID) != 0 ||
        engine->play_state_size != sizeof(PlayState) || engine->player_size != sizeof(Player) ||
        engine->save_context_size != sizeof(SaveContext)) return SHIP_NATIVE_UNSUPPORTED;
    // Compatibility probe only. No gameplay pointer is retained.
    return SHIP_NATIVE_OK;
}

static void SHIP_NATIVE_CALL Shutdown(void*) {}

extern "C" SHIP_NATIVE_EXPORT const ShipNativeDescriptor* SHIP_NATIVE_CALL ShipNative_Query() {
    static const ShipNativeDescriptor descriptor = {
        sizeof(ShipNativeDescriptor), SHIP_NATIVE_ABI_MAJOR, SHIP_NATIVE_ABI_MINOR, Init, Shutdown
    };
    return &descriptor;
}
```

`ShipNative_Query` returns a descriptor with C linkage and the SDK's calling convention. Init runs once per mod instance, receives the runtime table and writes the instance pointer. The probe needs no allocated state and succeeds only after querying `linkspan.oot.engine` by **name, version and minimum table size**, then checking table size, full layout ID and `sizeof(PlayState)`, `sizeof(Player)` and `sizeof(SaveContext)`. A missing service or mismatch returns `SHIP_NATIVE_UNSUPPORTED`.

For a real mod, check all required table callbacks, the runtime version/size before using appended ABI fields, and every structure size you cast. Own and free your instance state in your DLL; shutdown must handle partial init and null state. Register private functions during init or native hooks under the supported ABI. Do not retain pointers returned by `get_play_state`, `get_player` or hook payloads across callbacks, frames or scenes. Query current gameplay state again; null outside gameplay is normal. Follow each service's thread rules.

## Layout ID and executable fingerprint

`LINKSPAN_OOT_LAYOUT_ID` is a 64-character SHA-256 identifier generated from public boundary headers, pointer size, compiler ID/version, platform and flags. The public inputs include `soh/include`, `soh/soh/native` and `shiplua/native`; line endings are normalized. The eight characters in package names are only a label. Compare the **full** ID in native init. A provider built for a different layout must refuse to initialize; do not edit the ID to bypass that check. Rebuild and repackage against the matching SDK.

A change confined to internal implementation can leave the layout unchanged. That does not guarantee an escape-hatch mod is compatible: ABI 1.3 detours also require a core extension, exact `[provider].host_fingerprints` SHA-256 of `soh.exe` and a matching `soh.symbols`. Layout identifies the boundary; executable fingerprint identifies binary bytes.

## Build with `OotNativeSdk.cmake`

Older demo CMake projects use `-DOOT_NATIVE_SDK` instead of `find_package`. The release ZIP includes a relocatable wrapper:

```powershell
$sdkFile = (Resolve-Path "$sdk/OotNativeSdk.cmake").Path
cmake -S .\link-span-examples\oot-native\item-demo -B .\item-build -A x64 "-DOOT_NATIVE_SDK=$sdkFile"
cmake --build .\item-build --config Release
```

For a host developer, `<host-build>/native-sdk/Release/OotNativeSdk.cmake` is generated from the host's include paths/definitions and references that checkout and dependencies. Use the file from the same configuration as your target executable. This local export and the independent release ZIP are different SDK forms.

The demo scripts in `link-span-examples/tools/` expect to run from a host checkout by default: their validator/layout-header defaults point at its build. Check each script's parameters. For a release-SDK project that already produces a complete `mod/` folder, use the general packager below.

## Package and install

Place the matching game executable at `game/soh.exe` for the following inspection commands:

```powershell
python "$sdk/tools/shipmod.py" validate .\probe-build\mod
python "$sdk/tools/shipmod.py" doctor .\probe-build\mod --host .\game\soh.exe
& "$sdk/tools/package-linkspan-mod.ps1" -Source .\probe-build\mod -OutputPath .\SDK-Probe.shipmod
```

`validate` falls back to basic structural checks when the canonical C++ validator is absent. `doctor <mod> --host <exe>` inspects PE exports/imports, ABI and executable fingerprints **without loading the DLL**. Plain `doctor` targets a full source repository and may report missing schemas/codegen in the SDK subset. This CLI has no `pack` command. `shipmod test` needs an external runner through `SHIPLUA_MOD_TEST_RUNNER`; its binary is not included.

The packager takes `-Source` and `-OutputPath`, accepts `.zip` or `.shipmod`, requires a root `manifest.toml`, and refuses to overwrite an existing output. It uses `linkspan-zip.ps1` for `/` entry names, fixed order and timestamps. Avoid `Compress-Archive` and `ZipFile.CreateFromDirectory` in Windows PowerShell 5.1; their entry separators can prevent the loader finding the root manifest.

Close the game. Back up the previous package and saves. Copy only the finished `.shipmod` or `.zip` into `mods/` beside the matching `soh.exe`, without extracting it. The archive root must contain `manifest.toml`, `main.lua` and `provider/linkspan_sdk_probe.dll` for this probe. Remove or move the previous package outside `mods/` to avoid duplicate mod IDs. Start the game and check its log for successful package load and native initialization. Test the actual mechanics, scene transitions, save/load and clean shutdown required by your mod. Compilation and static inspection do not prove gameplay.

## Debug and recover

Read the first rejection in the host log: missing DLL, service/version mismatch, wrong layout, failed init and missing dependencies need different fixes. Verify `ShipNative_Query`, x64 architecture, Release `/MT`, full layout and the SDK compiler version. A native crash log includes `Link-Span native:`, native activity on the crashing thread, provider modules in the stack and loaded modules.

The OoT boot guard can disable an identified provider after a crash during native loading or before 300 gameplay frames. On the next start it records the mod in `mods/.shiplua-disabled` and logs the reason; it does not delete the package. Close the game, fix or replace the provider, then remove only that mod's line to retry. `mods/.shiplua-boot` is a boot marker removed when startup is confirmed. The crash demo deliberately tests this path; use an isolated test installation.

For rollback, restore the old complete package and the matching host. Restore save/config backups when a schema changed. A layout check and boot guard do not sandbox a DLL: native code has the process's OS access and can crash or corrupt it. Use trusted providers.

## OoT service index

Each link opens the detailed host reference. Version numbers below are service-table versions, independent of the provider ABI. Query the exact version and size you need; later versions are not always prefix-compatible (notably randomizer v2).

- [`linkspan.oot.engine` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-engine): Layout, structure sizes, current gameplay pointers and actor spawn/kill.
- [`linkspan.oot.movement` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-movement): Virtual/physical input, transient settings, jumping, rolling and item shortcuts.
- [`linkspan.oot.resources` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-resources): VFS reads, archive mounts, ordered layers and JSON transcoders.
- [`linkspan.oot.registry` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-registry): Generic named spaces/entries; v2 adds explicit ownership and unload cleanup.
- [`linkspan.oot.ocarina` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-ocarina): Accepted songs, note patterns and submitting a recognized song.
- [`linkspan.oot.scenes` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-scenes): Scene/entrance registration and travel; title cards, overrides and horse settings.
- [`linkspan.oot.save` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-save): Versioned JSON namespaces, transactions and save dependencies.
- [`linkspan.oot.items` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-items): Synthetic items, C buttons, get-item flow, icon/ammo updates and language.
- [`linkspan.oot.actors` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-actors): Custom actor types and native callbacks.
- [`linkspan.oot.actor-models` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-actor-models): Data-defined static/animated NPC profiles with model, collision, talk and look.
- [`linkspan.oot.camera` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-camera): Owned subcamera and active-view queries.
- [`linkspan.oot.render` v1/2/3](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-render): Draw/matrix operations, toon state, native geometry and frame interpolation.
- [`linkspan.oot.world` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-world): Read-only floor, line, wall and water queries.
- [`linkspan.oot.colliders` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-colliders): Actor cylinders, hit results and player knockback.
- [`linkspan.oot.skeletons` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-skeletons): Host-owned skeleton animation/update/draw handles.
- [`linkspan.oot.text` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-text): Message tables by language, raw game bytes and enumeration.
- [`linkspan.oot.randomizer` v1/2](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-randomizer): Offering mod items to future seeds; v2 reads the current seed and hints.
- [`linkspan.oot.anchor` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-anchor): Sharing save namespaces through SoH multiplayer.
- [`linkspan.oot.lights` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-lights): Owner-scoped point lights and scene-lifetime handles.
- [`linkspan.nei.items` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-nei-items): NEI-owned inventory, quantities and upgrade levels; requires the NEI core extension.
- [`linkspan.unbound.json_factory` v1](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#service-unbound-json-factory): Framework-published schema/layer merge results; not a built-in host service.

Native hooks are ABI callbacks rather than a `get_service` table: see [OoT hooks](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#native-hooks), [room actors](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#room-actors) and [escape hatch](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008). Caching a service table does not allow retaining its gameplay pointers.

## Native demo index

These demos live in the examples repository, not the Link-Span core distribution. Build with the matching `-DOOT_NATIVE_SDK`. Build Unbound native demos together from `oot-native/unbound`. Shovel also needs `-DLINKSPAN_NEI_INCLUDE=<NEI include directory>`; the release SDK provides that public header under `include/`. Unbound demos require the framework package for their content.

- [`hooks-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/hooks-demo): Observe updates and replace actor draw.
- [`item-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/item-demo): Orb item, get-item reception and a custom actor.
- [`save-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/save-demo): Count save loads in a JSON namespace.
- [`view-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/view-demo): Owned camera and a model attached to a Player limb.
- [`world-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/world-demo): Floor queries, sword hits and cylinder collision.
- [`familiar-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/familiar-demo): Animated Keese familiar and attack collider.
- [`cel-render-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/cel-render-demo): Native rendering demo using the toon/render contracts.
- [`shovel-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/shovel-demo): NEI Shovel integration with an independent-item fallback.
- [`escape-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/escape-demo): Executable-specific Interface_Draw detour.
- [`crash-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/crash-demo): Deliberate init crash to test boot recovery; isolated tests only.
- [`unbound/scene-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/scene-demo): Register a copy of Hyrule Field and travel to it.
- [`unbound/field-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/field-demo): Patch the vanilla field room actor list.
- [`unbound/custom-actors-demo`](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native/unbound/custom-actors-demo): Unbound 0.9 JSON actor profiles and room patches.

The full Dynamic Movement Remake example stays in the [host SDK directory](https://github.com/BaiterYamato/Shipwright/tree/agent/OOT-UPSTREAM-001-78dc6d970-integration/soh/native-sdk/example). The minimal release probe and Heart Seeds are SDK examples, not core fixtures. Follow the [example repository build/package instructions](https://github.com/BaiterYamato/link-span-examples#demos-nativos-oot) and inspect each demo's manifest/dependencies.
