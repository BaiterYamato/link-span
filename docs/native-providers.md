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

The ABI header is `shiplua/native/ship_native_abi.h`. `SHIP_NATIVE_ABI_MAJOR` is 1. The supplied generic core checkout has `SHIP_NATIVE_ABI_MINOR = 1` and its loader accepts manifest versions **1.0 and 1.1**. The core integrated into the supplied OoT host has minor **3**, accepts **1.0–1.3**, and is the source used to build the OoT release SDK. Do not combine headers from these different snapshots.

| ABI | Contract |
| --- | --- |
| 1.0 | `register_function` and `get_service`: private functions and host C services. |
| 1.1 | Adds `register_service` for core extensions during init. |
| 1.2 | In the OoT-integrated core: native hooks, including observe, transform and replace where the host permits them. |
| 1.3 | In the OoT-integrated core: executable fingerprint and gated symbol resolution/detours. |

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

On an ABI 1.2 runtime, a core extension can also declare and dispatch its own hook points; accepted modes and payload contracts belong to the point owner. On the OoT ABI 1.3 runtime, `resolve_symbol`, `install_patch` and `remove_patch` require a core extension with `abi_version = "1.3"` and matching `[provider].host_fingerprints`. See the [host escape-hatch reference](https://github.com/BaiterYamato/Shipwright/blob/agent/OOT-UPSTREAM-001-78dc6d970-integration/docs/native-mods.md#escape-hatch-coreext-008). A matching layout is insufficient for these operations.

## Unload and failure recovery

Failed init or entrypoint loading rolls back the instance. The generic unload sequence removes binding subscriptions, closes Lua while the provider is still available to finalizers, calls shutdown, unloads the library, then removes extracted files. Native hooks and patches in the newer runtime are removed before provider shutdown. If a patch cannot be removed safely, the runtime can quarantine the library instead of unloading code still in use.

The host must not destroy or move itself from inside a native callback. Unloading an actively executing provider is refused. Restore your own state changes and release resources according to each service's contract. C++ exceptions handled by the loader are destroyed before the library defining their types is unloaded.

## Trust, policy and isolation

`ShipLua::NativeProviderPolicy.enabled` defaults to false. The host chooses whether to run trusted native code; a mod manifest cannot enable this policy. An optional `authorize(manifest, absolute_path)` callback lets a host add approval or hash verification before mapping a library. The game does not automatically download or compile third-party code.

DLL/SO constructors and `DllMain` execute when the library is mapped, before `ShipNative_Query` and ABI validation. Transitive library dependencies are also trusted code. Native providers inherit the process's OS access; there is no process sandbox or isolation against memory errors, loops or crashes. The policy does not prevent a local process from replacing a file between approval and loading.

The loader requires library paths to remain within the mod and rejects traversal and links/reparse points. Windows uses local/system DLL lookup; POSIX uses `RTLD_LOCAL`. These reduce loading mistakes; they are not malicious-code containment. Do not promise hot reload with live references or arbitrary engine transformation from the generic ABI.

## Tests and examples

`tests/fixtures/mods/native-dash/` and `tests/native/` are **core test fixtures**, not modder examples shipped with Link-Span. Their `example.player` service is simulated, not an OoT/MM implementation. The independent test rebuilds only the provider, tests directories and ZIPs with oot/mm identities, and checks that the test host's SHA-256 does not change. This proves the generic boundary, not gameplay or native MM support.

Modder demos live in [link-span-examples](https://github.com/BaiterYamato/link-span-examples/tree/main/oot-native). The OoT SDK's `examples/provider-example` is the minimal compatibility probe; the host's `soh/native-sdk/example` retains the full Dynamic Movement Remake example. See [the demo index](native-mods.md#native-demo-index).

Generic contracts: [provider runtime RFC](../rfcs/0011-native-provider-runtime.md) and [core-extension RFC](../rfcs/0015-core-extension-abi.md). Use the target SDK headers as the final contract.
