**English** · [Português (Brasil)](Packaging-and-Publishing-pt-BR)

# Packaging and Publishing

## Package the mod contents

During development, install a folder. For distribution, use a **ZIP** with `manifest.toml` at its root. `.shipmod` is an alternative extension for the same ZIP format, not a different container. Include the declared entrypoint, scripts, original assets, provider binaries and legal notices that your mod needs.

```text
first-mod.zip
├── manifest.toml
├── main.lua
├── LICENSE
└── NOTICE.md
```

From a directory containing `first-mod/`:

```powershell
Compress-Archive -Path first-mod/* -DestinationPath first-mod.zip
python tools/shipmod.py validate first-mod.zip
```

This packages the contents, not an enclosing `first-mod/` directory. To use the alternative extension:

```powershell
Rename-Item -LiteralPath first-mod.zip -NewName first-mod.shipmod
python tools/shipmod.py validate first-mod.shipmod
```

The CLI has `new`, `validate`, `test` and `doctor`; there is no `pack` or `publish` subcommand.

## Validate before publishing

1. Run the **compiled manifest validator** on the final package through `shipmod.py validate`. Build `shiplua_manifest_validator` first, or configure `SHIPLUA_MANIFEST_VALIDATOR`; for native mods, use the matching SDK/host validator. If the CLI reports basic fallback validation, complete the canonical check before publishing.
2. Check id, mod version, API range, games, required capabilities, dependencies and every declared file path. Native providers need matching ABI, platform, layout and any host fingerprint.
3. Run mock tests where applicable, then install the final package in a clean test game's `mods/`. Confirm load, intended behavior, save/reload, removal and relevant mod combinations in the real host. Record the host/SDK version and results.
4. Include installation, requirements, limitations, update and rollback instructions. Remove build output, caches, personal saves and developer-only files from the package.
5. Include your license and third-party notices; distinguish your own code from ported code and research credits. Link-Span's own work is CC0 1.0, but third-party code and assets keep their own terms.
6. Do not include ROMs, `oot.o2r`, `oot.otr`, `mm.o2r`, dumps or extracted copyrighted game assets. Reference the player's locally generated archives instead. Publish only assets you can redistribute.
7. Give the release a version, list compatible hosts and dependencies, and provide a checksum for the final artifact.

The manifest validator does not execute Lua, prove native safety, resolve all installed dependencies or demonstrate gameplay. Keep those checks separate.

References: [Manifest](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest.md) · [CLI](https://github.com/BaiterYamato/link-span/blob/main/docs/shipmod-cli.md) · [Manifest validator](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest-validator.md) · [Native mods](https://github.com/BaiterYamato/link-span/blob/main/docs/native-mods.md) · [License](https://github.com/BaiterYamato/link-span/blob/main/LICENSE) · [Notices](https://github.com/BaiterYamato/link-span/blob/main/NOTICE.md).
