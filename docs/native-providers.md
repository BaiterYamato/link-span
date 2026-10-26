**English** · [Português (Brasil)](native-providers.pt-BR.md)

# Native providers

A native provider is a C/C++ library shipped inside a Link-Span mod. The loader calls its init function and supplies versioned C tables. The provider can register private functions for its own mod and query services supplied by the host or a core extension. Its functions do not need entries in a central schema or loader allowlist.

For a Windows x64 OoT mod, start with [Native OoT mods](native-mods.md). That guide uses the release SDK and a real OoT service. The generic core ABI does not itself implement game mechanics.

## Choose Lua or native code

Prefer Lua when the existing public API covers the task. It avoids a native compiler and host-layout coupling. Use native code when you need a C service, native hooks, direct access to the documented OoT layouts, or a framework that publishes C tables to other providers. A new function inside a mod does not require rebuilding the host; a missing host service may require a host adapter update.

Native code runs in the game process. Choose providers from trusted sources. ABI checks, package validation and the boot guard are compatibility and recovery tools; they do not isolate a DLL.

## Package and manifest

Put `manifest.toml` and `main.lua` at the package root. Put the library at the path declared in `[provider]`. A `.shipmod` is a ZIP with a different extension; do not add an enclosing folder. Supported platform fields are `win64`, `linux64` and `macos`. An absent field for the running platform causes an explicit error. These native targets are x86-64; this is not an ARM support claim.

```toml
id = "author.native_mod"
name = "Native mod"
version = "1.0.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"

[provider]
abi_version = "1.0"
win64 = "provider/native_mod.dll"
linux64 = "provider/libnative_mod.so"
macos = "provider/libnative_mod.dylib"
```

There is no `declares` field for native function names. They belong to the provider instance. For OoT, use `games = ["oot"]` at the top level and the release's Windows SDK; the generic fields above do not promise native MM integration.

Use the host's `package-linkspan-mod.ps1` for Windows packages. It writes ZIP entry names with `/`. See the [OoT packaging steps](native-mods.md#package-and-install); older Windows PowerShell archive commands can produce incompatible entry names.

## ABI versions: use the headers shipped with your target

The ABI header is `shiplua/native/ship_native_abi.h`. The core implements C ABI **1.0 through 1.4**, including hooks, the escape hatch and layout-bound public symbols: `SHIP_NATIVE_ABI_MAJOR` is 1 and `SHIP_NATIVE_ABI_MINOR` is 4. ABI 1.0 remains valid for ordinary native mods. Use the headers shipped with your target runtime and SDK; do not combine snapshots.

| ABI | Contract |
| --- | --- |
| 1.0 | `register_function` and `get_service`: private functions and host C services. |
| 1.1 | Adds `register_service` for core extensions during init. |
| 1.2 | Native hooks, including observe, transform and replace where the host permits them. |
| 1.3 | Executable fingerprint and gated symbol resolution/detours. |
| 1.4 | Public symbols bound to the host layout id; `ShipNativeRuntime` keeps the 1.3 layout. |

The manifest declares the required minor version. The descriptor reports the ABI actually compiled into the provider. The loader checks descriptor size, major/minor compatibility and init/shutdown callbacks. A manifest version alone cannot make an older runtime expose newer fields. Check the runtime's size and version before accessing appended members. The OoT SDK's `linkspan-sdk.json` records its native ABI and toolchain.

Export `ShipNative_Query` with C linkage, `SHIP_NATIVE_EXPORT` and `SHIP_NATIVE_CALL` (`__cdecl` on Windows). Return a `ShipNativeDescriptor` with `size`, `abi_major`, `abi_minor`, `init` and `shutdown`. Do not link the provider to the game, Link-Span core, Lua or libultraship. Standard compiler/OS libraries remain your toolchain's responsibility.

## Init, calls and service lookup

Init receives `const ShipNativeRuntime*` and writes a per-instance pointer through `void** instance`. Allocate and free the provider's state in its own library. Shutdown must tolerate a null instance after partial initialization. Do not transfer allocations between different CRTs.

Register private functions only during init with `register_function`. Each instance can register up to 256 functions. Its Lua entrypoint can invoke them through `ship.native.has` and `ship.native.call`; these names are private to that mod. `has` accepts a textual name of 1–128 bytes. Requests and responses are byte strings, including NUL, with a maximum of 65,536 bytes in each direction. The provider chooses the protocol. The writer is valid only during the call and copies bytes without taking ownership.

Native call errors become `nil, {code, message}`. Three consecutive native failures disable calls until the mod reloads; invalid call arguments do not count toward that limit. Memory corruption, hangs and process crashes are not caught by this mechanism.

Query a service with `get_service(context, name, version, minimum_size)`. A missing or incompatible service returns null. Check its documented table size, required callbacks and any layout/structure-size fields before use. The loader does not interpret the table or maintain a central catalogue of its names. Host tables must outlive their providers; a core-extension table must stay valid until its shutdown. This lifetime does **not** extend the lifetime of gameplay pointers returned by a service.

## Core extensions (`pre_game`)

A framework that publishes C services uses this manifest form:

```toml
id = "author.framework"
name = "Native framework"
version = "1.0.0"
api = ">=0.5.0 <0.6.0"
entrypoint = "main.lua"
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.1"
win64 = "provider/framework.dll"
```

During init, publish your versioned table:

```cpp
runtime->register_service(runtime->context, "author.framework.items", 1,
                          sizeof(items_api), &items_api);
```

`items_api` is the framework's own contract and storage; it must survive until shutdown. Publication is restricted to core extensions during init. Consumers query the same name/version/size and declare their framework dependency in the manifest. The loader orders dependencies and refuses to unload a service owner while a consumer remains loaded. Full shutdown runs in reverse load/dependency order.

On an ABI 1.2 runtime, a core extension can also declare and dispatch its own hook points; accepted modes and payload contracts belong to the point owner. On the OoT runtime, `install_patch`, `remove_patch` and private symbols require a core extension with ABI 1.3 or 1.4 and matching `[provider].host_fingerprints` (see the [host escape-hatch reference](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008)). With ABI 1.4, public symbols only need a matching `[provider].host_layout_id`; see [Public symbols](#public-symbols-abi-14). A matching layout never authorizes a detour.

## Native hooks (ABI 1.2)

With `abi_version = "1.2"`, a provider enters the game flow without adding a new host function. The host (or a core extension during init) declares named points with a version and payload size; any provider can register hooks on them:

```c
ShipNativeHookSpec spec = {sizeof(spec), "oot.actor.update", 1, sizeof(ShipOotActorHookV1),
                           SHIP_NATIVE_HOOK_REPLACE, 0, 0, MyUpdate, state};
uint64_t handle;
runtime->register_hook(runtime->context, &spec, &handle);
```

- `observe` (`BEFORE`/`AFTER` phase) reads; `transform` modifies the payload in a chain; `replace` is exclusive and decides whether to invoke `call->call_original(call)`.
- Higher priority runs first; ties follow load order.
- `UNSUPPORTED` refuses without counting a failure; three consecutive failures disable the hook.
- The mod's hooks are removed automatically on unload, before `shutdown`.
- A host may dispatch a point outside the game thread with its own dispatcher (for example an audio mix point on
  the audio thread). Its header says so; callbacks there must not block or call host services, and unregistering
  waits for the callback to return.

A core extension publishes its own points with `declare_hook_point` and triggers them with `dispatch_hook`. See [RFC 0021](../rfcs/0021-native-hooks.md).

### Extended health in `oot.actor.damage`

With [RFC 0031](../rfcs/0031-atores-para-mods.en.md), the native `oot.actor.damage` hook sees and returns `damage` in the target's extended-health units when extended health is enabled. Do not truncate it to `uint8_t` or assume a 255 ceiling: the host clamps it to 0..65535 and the remaining HP, then converts the pending debit to the vanilla mirror. The initial damage is still what the game calculated; the cooperative multiplier applies afterward. An unextended target keeps the F2i 0..255 path.

The actor's vanilla `u8` health is a mirror, not its real extended HP. A proposed debit is not proof of effective damage; it must be confirmed when applied. This change adds no native extended-health service table or new ABI struct. See [Actors for modders](actors-for-mods.md) for the mirror, confirmation rules and exclusions.

## Escape hatch (ABI 1.3)

For changes no service or hook covers, a core extension with `load_phase = "pre_game"` and `abi_version = "1.3"` (or `"1.4"`) can resolve executable functions by name and detour them:

```toml
[provider]
abi_version = "1.3"
win64 = "provider/framework.dll"
host_fingerprints = ["<executable SHA-256>"]
```

Replace the fingerprint placeholder with the executable's 64-character hexadecimal SHA-256.

```c
uintptr_t target;
void* original;
uint64_t patch;
if (runtime->resolve_symbol(runtime->context, "z_player.c!Player_Action_Roll", &target) == SHIP_NATIVE_OK)
    runtime->install_patch(runtime->context, target, MyRoll, &original, &patch);
```

- `get_host_fingerprint` reports which SHA-256 to declare.
- Outside the fingerprint list, or in an ordinary mod, these calls return `UNSUPPORTED`.
- A provider can install up to 1024 patches; beyond this, `install_patch` returns `LIMIT`.
- Patches are removed automatically on unload, before `shutdown`.
- In the OoT host, detours installed during init become active together only after init returns; init cannot depend on them. After init, each `install_patch` and `remove_patch` takes effect when it returns.
- Rebuild and republish the mod for every new executable. See [RFC 0023](../rfcs/0023-native-escape-hatch.md).

## Public symbols (ABI 1.4)

A core extension that only **calls** game functions or **reads** game variables does not need the executable fingerprint. With ABI 1.4 it declares the SDK layout id it was built against:

```toml
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.4"
win64 = "provider/framework.dll"
host_layout_id = "<64-character layout id from the SDK>"
```

- `resolve_symbol` first looks the plain name up in the host's public catalog: functions and variables declared directly in `soh/include/functions.h` and `soh/include/variables.h`. The catalog does not read `soh.symbols`, the PDB or the executable hash.
- The core compares `host_layout_id` with the host's layout before consulting the catalog. A missing or different layout leaves the public path `UNSUPPORTED`; the mod still loads, and its `init` decides whether to fail or degrade.
- Qualified names (`file.c!Name`) never use the public path.
- A name outside the catalog falls through to the private path only when `host_fingerprints` lists the current executable. A catalog failure does not fall through.
- A public address does not authorize `install_patch`: detours still need `host_fingerprints`.
- On any failure the core writes 0 to `*address`.
- `ShipNativeRuntime` and `ShipNativeDescriptor` keep their 1.3 layout; the descriptor must report minor 4 or later. A 1.3 manifest never uses the public path, even with a 1.4 DLL.
- The layout id guarantees matching declarations, not identical behavior: a function can keep its signature and change what it does. See [RFC 0029](../rfcs/0029-host-limpo-conteudo-mm-em-mods.md).

The OoT host publishes the catalog from RC11 on.

## Crash and boot protection

- **Crash log:** the host records which mod was in `init`, `call`, `hook` or `shutdown`, and which mod libraries appear in the stack (`WriteNativeCrashReport`).
- **Boot protection:** with `ModHost::EnableBootGuard`, a native mod that crashes startup is recorded in `mods/.shiplua-disabled` and skipped on subsequent starts.
- **Re-enable:** delete the mod's line from that file.
- **Protection window:** `ConfirmBoot()` ends protection after the game runs normally.
- See [RFC 0024](../rfcs/0024-native-crash-attribution.md).

Rejection message in the log:

```text
disabled by boot protection (...); delete the mod's line in .shiplua-disabled to re-enable it
```

## Unload and failure recovery

Failed init or entrypoint loading rolls back the instance. The generic unload sequence removes binding subscriptions, closes Lua while the provider is still available to finalizers, calls shutdown, unloads the library, then removes extracted files. Native hooks and patches in the newer runtime are removed before provider shutdown. If a patch cannot be removed safely, the runtime can quarantine the library instead of unloading code still in use.

The host must not destroy or move itself from inside a native callback. Unloading an actively executing provider is refused. Restore your own state changes and release resources according to each service's contract. C++ exceptions handled by the loader are destroyed before the library defining their types is unloaded.

## Escape hatch: batching and quarantine

The core opens a batch per owner during `init` and during patch removal on unload when the host offers `beginBatch` and `commitBatch`. Patches installed during init become active only after it returns. Outside a batch, `install_patch` and `remove_patch` take effect when they return.

If the host does not confirm removal of every patch, unload is quarantined: the library stays loaded and the provider cannot be treated as removed. This prevents a detour from pointing to unloaded code. A provider must not depend on an active patch during its own init or install patches in `shutdown`.

Detours remain restricted to core extensions with ABI 1.3 or 1.4, the executable fingerprint declared in the manifest and the limit of 1024 patches per provider.

## Trust, policy and isolation

`ShipLua::NativeProviderPolicy.enabled` defaults to false. The host chooses whether to run trusted native code; a mod manifest cannot enable this policy. An optional `authorize(manifest, absolute_path)` callback lets a host add approval or hash verification before mapping a library. The game does not automatically download or compile third-party code.

DLL/SO constructors and `DllMain` execute when the library is mapped, before `ShipNative_Query` and ABI validation. Transitive library dependencies are also trusted code. Native providers inherit the process's OS access; there is no process sandbox or isolation against memory errors, loops or crashes. The policy does not prevent a local process from replacing a file between approval and loading.

The loader requires library paths to remain within the mod and rejects traversal and links/reparse points. Windows uses local/system DLL lookup; POSIX uses `RTLD_LOCAL`. These reduce loading mistakes; they are not malicious-code containment. Do not promise hot reload with live references or arbitrary engine transformation from the generic ABI.

## OoT randomizer item queries

The OoT adapter offers `linkspan.oot.randomizer` v2 to query a seed's item catalogue and find an uncollected location. The v1 table remains available; previous service sizes and offsets are preserved. The detailed contract is in [RFC 0026](../rfcs/0026-oot-randomizer-item-hints.md); the OoT SDK header is `oot_randomizer.h`.

Queries require the game thread and buffers of the declared size. Location lookup requires a normal randomizer save to be loaded; ordinary saves return `SHIP_NATIVE_UNSUPPORTED`. The query does not grant items, reveal collected locations or charge the save. The mod decides how to present the hint and when to charge for it, after receiving a valid response.

The generic hooks `oot.actor.init` and `oot.play.draw_end` let you observe actors after complete initialization and draw at the end of the gameplay frame. The latter runs inside the rendering context. Both are notifications; they do not replace the original routine or guarantee support for a particular mechanic.

## OoT SDK contract

The OoT SDK is host-specific. For the service tables below, include the public headers in `soh/soh/native/` and request each table with `runtime->get_service`. Check the requested version, `size` and pointers before using it. The public contracts are:

| Service | Version | Header | Main use |
| --- | --- | --- | --- |
| `linkspan.oot.registry` | 1 and 2 | `oot_registry.h` | Dynamic OoT registrations |
| `linkspan.oot.randomizer` | 1 and 2 | `oot_randomizer.h` | Seed items and hints |
| `linkspan.oot.actor-models` | 1 | `oot_actor_models.h` | Models for the mod's actor types |
| `linkspan.oot.lights` | 1 | `oot_lights.h` | The mod's point lights |
| `linkspan.oot.render` | 1, 2 and 3 | `oot_render.h` | Drawing and rendering state within the permitted scope |
| `linkspan.oot.scenes` | 1, 2 and 3 | `oot_scenes.h` | Dynamic scenes and entrances |
| `linkspan.oot.save` | 1 | `oot_save.h` | Namespaced JSON blocks in the save |
| `linkspan.oot.resources` | 1, 2 and 3 | `oot_resources.h` | VFS, layers and resource invalidation |

Ownership, thread and scope rules are in each service header. Save namespaces belong to the loaded save, and rendering operations are valid only in the hooks or draw callbacks that the header permits.

OoT native points are in `oot_hooks.h`. Use `oot.actor.init` only to observe an actor after complete init, and `oot.play.draw_end` only to draw at the end of the frame inside the rendering context. The same header declares each other point's permitted mode (`observe`, `transform` or `replace`) and payload.

## Tests and examples

`tests/fixtures/mods/native-dash/` and `tests/native/` are **core test fixtures**, not modder examples shipped with Link-Span. Their `example.player` service is simulated, not an OoT/MM implementation. The independent test rebuilds only the provider, tests directories and ZIPs with oot/mm identities, and checks that the test host's SHA-256 does not change. This proves the generic boundary, not gameplay or native MM support.

Modder demos live in [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native). The OoT SDK's `examples/provider-example` is the minimal compatibility probe; the host's `soh/native-sdk/example` retains the full Dynamic Movement Remake example. See [the demo index](native-mods.md#native-demo-index).

Generic contracts: [provider runtime RFC](../rfcs/0011-native-provider-runtime.md) and [core-extension RFC](../rfcs/0015-core-extension-abi.md). Use the target SDK headers as the final contract.
