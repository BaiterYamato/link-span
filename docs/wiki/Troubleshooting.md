**English** · [Português (Brasil)](Troubleshooting-pt-BR)

# Troubleshooting

## Find the first useful diagnostic

Read the game log beside the executable: `logs/Ship of Harkinian.log` for OoT or `logs/2 Ship 2 Harkinian.log` for MM. For Unbound also read `logs/linkspan-unbound.log`. Start at the first error or rejected mod, not later symptoms.

## A mod is absent or refused

- Confirm you started the Link-Span-enabled executable and put the package in that game's `mods/`.
- A ZIP must have `manifest.toml` at its root; a ZIP without it is ignored. Check the declared entrypoint/provider files and avoid duplicate installed versions of an id.
- Validate the actual installed package with `python tools/shipmod.py validate <package>`. Use the compiled validator, not only the CLI's basic fallback.
- Check the API range against **0.5.0**, the declared game and host version, required capabilities and dependencies. An old `>=0.4 <0.5` range excludes 0.5.0. Widening a range does not fix incompatible code; test before changing it.
- A missing required capability needs a suitable host or a revised mod. For an optional feature, use `ship.capabilities.has(...)` before invoking it.
- On Windows, try a short game path, such as `C:\Games\SoH`. RC9 still has a 260-character path limit and extracts packages under `mods/.shiplua-cache/`.

## Native layout or fingerprint mismatch

Use the provider compiled against this host's SDK. Rebuild or download a matching package; do not bypass the layout check. Escape-hatch providers also require the exact executable fingerprint and matching `soh.symbols`. Keep `soh.exe`, `soh.o2r` and `soh.symbols` from the same overlay. See [Native Mods](Native-Mods).

## Boot guard disabled a mod

If an attributed native failure happens while loading or before 300 gameplay frames, the next startup disables the culprit in **`mods/.shiplua-disabled`**. The package stays in `mods/`.

Close the game, read the log and move the suspect package outside `mods/`. After fixing or replacing it, remove only its line from `.shiplua-disabled` to re-enable it. Do not blindly clear the whole list. If startup keeps failing, restore the matching host/mod backup described in [Getting Started](Getting-Started).

## A key or resource does nothing

Check the configured binding and other mods using the same key. Actor creation can require an object loaded in the current scene; use the example's recommended scene. Check capabilities instead of assuming the mock and real host offer identical services.

For cross-world content, inspect the archive mount diagnostic and correct sibling folders or `SHIPLUA_MM_ROOT` / `SHIPLUA_OOT_ROOT`. Generate both archives through their normal game flows. See [Cross-World Assets](Cross-World-Assets).

For NEI, if items show up without an icon or model, find NEI's status line in the game log and check that it reads `assets: pacote: core+...` (the log text is in Portuguese). From NEI 0.3.24 on the assets are inside the `LinkSpan-NEI-*.shipmod` package, so there is nothing to generate; reinstall the package if that line shows no core. An older package (before 0.3.24) still depends on `nei-assets/nei-assets-core.o2r` beside the game; without it only Roc's Feather and Roc's Cape are available. For Unbound data, install the required Unbound Framework; otherwise RC9 skips those archive layers.

## Report a reproducible problem

Include host and overlay version, mod names/versions, exact actions and scene, and the relevant log excerpt with any stack trace. For native failures include SDK/layout information and whether the boot guard disabled a package. Remove secrets and personal information from attachments. File reports in [Link-Span issues](https://github.com/BaiterYamato/link-span/issues).
