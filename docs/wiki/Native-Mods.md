**English** · [Português (Brasil)](Native-Mods-pt-BR)

# Native Mods

A native mod packages Lua with a C/C++ provider library. Lua calls the mod's private functions through `ship.native.call`. Core extensions can publish versioned services for other providers. New private functions do not need an entry in the Lua API schema or a rebuild of the host; a service missing from the host may require an adapter update.

## Use the matching OoT SDK

Download the RC10 **`LinkSpan-OoT-SDK-oot-0.5.0-rc10-layout-ee6449e8-Win64.zip`** alongside the matching host release. Extract it outside the game folder. The SDK is not a mod: do not put it in `mods/`.

Read `AUTHOR-GUIDE.md`, `OOT-NATIVE-CONTRACT.md` and `linkspan-sdk.json` in the SDK. Use the MSVC version recorded there. A host source build exports `build/x64/native-sdk/Release/OotNativeSdk.cmake`; set `OOT_NATIVE_SDK` to the actual `OotNativeSdk.cmake` in your extracted SDK or build.

The detailed authoring and packaging steps are in [docs/native-mods.md](https://github.com/BaiterYamato/link-span/blob/main/docs/native-mods.md). The generic provider contract is in [Native providers](https://github.com/BaiterYamato/link-span/blob/main/docs/native-providers.md).

## Compatibility and installation

- Build for the provider platform declared in the manifest and the correct host SDK/layout id.
- Validate with the matching SDK/host validator. An older core validator may reject newer provider ABI versions.
- An escape-hatch provider also targets the exact host executable fingerprint and matching symbols. A matching Lua API version alone is insufficient.
- Put the validated mod ZIP, `.shipmod` or folder in `mods/`; keep `manifest.toml` at its root and the library at its declared provider path.
- OoT services do not automatically exist in MM. Check the host's documented services before claiming MM support.

Native libraries execute inside the game process with its permissions. Layout and ABI checks establish compatibility, not a sandbox. A native crash can terminate the game; the boot guard is recovery, not containment.

Browse the native demos in [Example Mods](Example-Mods). The `crash-demo` deliberately crashes and belongs only in an isolated recovery test. See [Troubleshooting](Troubleshooting) for `mods/.shiplua-disabled`.
