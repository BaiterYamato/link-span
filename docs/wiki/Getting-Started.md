**English** · [Português (Brasil)](Getting-Started-pt-BR)

# Getting Started

## Install on OoT: overlay and independent mods

Use the **oot-0.5.0 RC5** files and their release notes from the [Link-Span-enabled Shipwright fork](https://github.com/BaiterYamato/Shipwright/releases). Choose the overlay for **Shipwright 9.2.3**, not the author SDK ZIP. Check the release's `SHA256SUMS.txt` against the downloaded files.

1. Extract official Shipwright 9.2.3 into a new, short path, such as `C:\Games\SoH`. Use Shipwright's normal extraction flow to generate `oot.o2r` from your own game ROM. Run the game once, then close it.
2. Back up the whole game folder. At minimum keep `Save/`, `shipofharkinian.json`, `mods/`, `soh.exe`, `soh.o2r` and `soh.symbols`.
3. Extract `LinkSpan-Shipwright-9.2.3-…-Win64-overlay.zip` into the game root. Replace **`soh.exe`, `soh.o2r` and `soh.symbols` together**.
4. Put the selected mod ZIPs in `mods/` without extracting them. Start `soh.exe` and inspect `logs/Ship of Harkinian.log` for loaded or rejected mods.

```text
SoH/
├── soh.exe
├── soh.o2r
├── soh.symbols
├── oot.o2r              # generated locally by the player
├── Save/
├── logs/
└── mods/               # selected mod packages or development folders
```

RC5 provides separate packages for Not Enough Items 0.3.20, Unbound Framework 0.6.11, Dynamic Movement Remake 0.2.19, Mic Ocarina 0.2.5, Wind Waker Style 0.1.2 and Shovel Demo 0.1.2, plus the Unbound Actors Demo data archive and author SDK. Install only what you want; the overlay is the host, while these packages are mods.

With Unbound Framework installed, first startup generates `oot-unbound.o2r` and `oot-unbound.o2r.source.json` beside `oot.o2r`. Its detailed log is `logs/linkspan-unbound.log`. Unbound data archives need the framework; RC5 skips those layers when it is absent.

Most NEI items require locally generated `nei-assets/nei-assets-core.o2r`. Without it, only Roc's Feather and Roc's Cape are available. RC5's generator requires a host checkout and `build-nei-assets.py`; follow the NEI package README. No player-only generator is provided in these notes.

## MM and the launcher

MM needs a **Link-Span-enabled 2Ship2Harkinian host**. A normal upstream executable does not gain the loader by receiving a mod ZIP. Use the documented [host integration](https://github.com/BaiterYamato/link-span/blob/main/docs/host-integration.md) or [MM fork](https://github.com/BaiterYamato/2ship2harkinian). Generate `mm.o2r` through 2Ship's normal extraction flow, then put mods in `mods/` beside `2ship.exe` and inspect `logs/2 Ship 2 Harkinian.log`.

The OoT RC5 overlay is not an MM overlay. For a dual-host installation, follow the [launcher guide](https://github.com/BaiterYamato/link-span/blob/main/docs/launcher.md). A packaged launcher uses `link-span.exe` and host folders `hosts/oot/` and `hosts/mm/`; it chooses the available game or offers a chooser when both are available. A mod declaring `requires_both_games = true` needs both games. See [Cross-World Assets](Cross-World-Assets) for sibling archives.

## Update or roll back

Close the game first. Move the old mod package outside `mods/`, then install the replacement; do not leave duplicate versions. When changing the RC5 overlay, replace NEI, DMR and Mic Ocarina with matching builds because they target the exact host executable. Shovel Demo and Wind Waker Style need a matching layout id; Unbound uses the host's public services.

To roll back, restore the backup: the three host files together, the previous mods, saves and configuration. A newer save or migrated `shipofharkinian.json` is not guaranteed to work with an older or official host.

No ROM or player-generated `oot.o2r`, `oot.otr` or `mm.o2r` is distributed. [Troubleshooting](Troubleshooting) covers recovery after a crash.
