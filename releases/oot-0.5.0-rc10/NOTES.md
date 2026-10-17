## What is in this release

Link-Span is a modloader for Ship of Harkinian (Shipwright) 9.3.0. This release candidate has the overlay with the
modloader (API 0.5.0, native providers up to ABI 1.3), the native mods built for it, the SDK for mod authors and the
pinned sources.

| Download | What it is |
|---|---|
| OoT Overlay — Windows | the host `soh.exe`, `soh.o2r` and `soh.symbols` with Link-Span, layout `ee6449e8` |
| OoT SDK | headers, CMake, examples and guides for native mod authors (`rc10`) |
| Mod Examples — ZIP | the RC10 mods in `release-mods/`, plus Lua examples and native demos; see `README.txt` and `CATALOG.json` |
| Source | the core, host and examples sources at the commits used, with their licenses and `SOURCE-LOCK.json` |

The mods in `release-mods/`:

| Mod | Version | What it is |
|---|---|---|
| Not Enough Items | 0.3.25 | extra inventory page and the items of the NEI fork (skijer), with their assets |
| Unbound Framework | 0.6.15 | reader for the Unbound 2 format (scenes, rooms, actors, text) |
| Dynamic Movement Remake | 0.2.24 | movement, camera, HUD D-pad and the pause selector |
| Mic Ocarina | 0.2.10 | play the ocarina with a microphone |
| Wind Waker Style | 0.1.6 | Wind Waker style lighting, sky and clouds |
| Shovel Demo | 0.1.6 | example item that uses the NEI service, with its own icon, name and model |
| `LinkSpan-Unbound09-Actors-Demo.o2r` | data | four actors in Hyrule Field, in the Unbound 0.9 format (needs the Unbound Framework) |

## What changed since RC9

- **New layout id: `ee6449e8`** (RC9: `7376240d`). The Link-Span code in the host is now formatted with
  clang-format 14, the formatter the Shipwright project checks in CI. Only whitespace, line breaks and comments
  changed, so the game behaves exactly as in RC9. The native contract headers are part of the layout id, though, so
  the id changed and every native mod was rebuilt for it.
- **Every native mod has a new version** with no change in behavior: NEI 0.3.25, Unbound Framework 0.6.15, DMR
  0.2.24, Mic Ocarina 0.2.10, Wind Waker Style 0.1.6 and Shovel Demo 0.1.6. The RC9 packages do not load on the RC10
  overlay, and the RC10 packages do not load on the RC9 overlay: the log rejects them for the layout id.
- **The Actors Demo** is data only and works the same as in RC9.
- **This page and the Mod Examples ZIP are now in English.** The ZIP folders were renamed: `release-mods/`
  (was `mods-do-release/`), `native-oot/` (was `nativos-oot/`), `diagnostics/` (was `diagnosticos/`) and `docs/`;
  `README.txt` and `CATALOG.json` replace `LEIA-ME.txt` and `CATALOGO.json`.

## Install

1. Extract the official Shipwright 9.3.0 into a new folder, generate `oot.o2r` from your ROM the usual way and start
   the game once. Close it.
2. Keep a copy of the whole game folder (or at least `Save/`, `shipofharkinian.json`, `mods/` and the `soh.exe`,
   `soh.o2r` and `soh.symbols` in use). That copy is your way back.
3. Extract the overlay into the game root. `soh.exe`, `soh.o2r` and `soh.symbols` go together: they belong to the
   same host build.
4. Extract the Mod Examples ZIP outside the game folder. Copy the mods you want from `release-mods/` into `mods/`
   without extracting them, and start the game. The log (`logs/Ship of Harkinian.log`) says what loaded and what was
   rejected. On the first run with the Unbound Framework it converts the scenes and writes `oot-unbound.o2r` and
   `oot-unbound.o2r.source.json` next to `oot.o2r`. Detailed Unbound diagnostics also go to
   `logs/linkspan-unbound.log`.

Never copy the whole Mod Examples ZIP into `mods/`. The SDK ZIP is for mod authors: extract it outside the game
folder and do not put it in `mods/`.

## Update and roll back

- Coming from RC9: close the game, extract the RC10 overlay over the RC9 folder and replace **all** the RC9 mods in
  `mods/` with the RC10 ones. Because of the new layout id, none of the RC9 native packages loads on RC10.
- Coming from RC8 or earlier on Shipwright 9.3.0: same as above; the `nei-assets/` folder of NEI 0.3.23 is no longer
  needed. Coming from RC6 (Shipwright 9.2.3): start from a fresh Shipwright 9.3.0 folder (step 1 of the install),
  then copy your `Save/` and `shipofharkinian.json` from the RC6 folder before installing the overlay.
- Update with the game closed: take the old version out of `mods/` (keep it outside the game folder) and copy the
  new one. The save keeps the data of missing mods, so removing a mod and putting it back does not lose its progress.
- NEI, DMR and Mic Ocarina are built for this `soh.exe` (SHA-256:
  `40c8a7253a7183c7e93662ebe3369492bf1d94102bd7f6363776a83087338b9b`). When you replace the overlay, replace those
  three too. The Shovel Demo, Wind Waker Style and the Unbound Framework depend on the layout id (`ee6449e8`): if a
  new overlay changes the layout, they need a new version as well.
- To go back to RC9, close the game and restore the copy from step 2 of the install: `soh.exe`, `soh.o2r` and
  `soh.symbols` together, the RC9 mods and the saved saves and settings. Do not expect a save made with this version
  to open in an older one.
- If a native mod crashes the game while loading, or before 300 game frames with the culprit identified, it is
  disabled on the next start: it goes into the `mods/.shiplua-disabled` list and the file stays in `mods/`. Delete
  its line from the list to enable it again. If the boot keeps crashing, take the suspect package out of `mods/`
  and check the log.
- The host may migrate `shipofharkinian.json` to a format the official Shipwright cannot read; to go back to the
  official build, restore the copy from step 2.

## Known limitations

- **Audio:** the host follows Shipwright 9.3.0, which reverted the self-pumping audio thread of the development
  builds. Audio behaves like the official 9.3.0.
- **NEI:** the fork's forms (Garo, Gerudo, Kafei, Keaton, Rito), network mode, Pikachu, Wolf Link, the Mario expansion
  (libsm64) and the Majora's Mask transformation masks are not in this version. The golden Deku flowers of the Rod of
  Seasons summer depend on the forms. The Gust Jar's Shadow gust stuns small enemies but does not blind; Chateau
  Romani (only obtainable with the Romani Mask) does not give infinite magic.
- **Unbound:** limits are not unlimited. Live actors and actors per room stop at 8,192 (they may be rejected earlier
  for memory); paths and lights use 8-bit indices; fixed cameras use 16-bit coordinates; the reference graph has a
  64 MiB budget; there are 255 lights per list and 765 across a scene and its rooms.
- **Savestates** are refused after a scene change or after creating or removing a bridge light; the Rod of Seasons
  may rewrite water after a load.
- **DMR and Mic Ocarina** were tested by automation; use with a physical controller and microphone is up to the
  player. A stuck audio driver can delay closing the game with Mic Ocarina enabled.
- **Wind Waker Style** starts with the sky off; the full look needs "Use Sky" and "Enable Actor Shadows".
- **Performance:** there is no frame time or memory measurement under load, and no proof on the OpenGL backend for
  the most recent Unbound rounds.
- **Game folder path:** packages are extracted into `mods/.shiplua-cache/` and the host still follows the Windows
  260-character limit. In a game folder with a ~140-character path, DMR and the Unbound Framework were rejected
  ("file name too long" in the log); use a short path such as `C:\Games\SoH`.
- **Native mods** run inside the game process, without a sandbox. A DLL only loads in the host it was built for
  (layout id; for the escape hatch, also the SHA-256 of `soh.exe` and `soh.symbols`).
- **Actors Demo:** the dialogue is in English only. The actors adapt the official Unbound 0.9 example (roborich) and
  use game models extracted by the player; the rest is Link-Span work under CC0.
- **Licenses:** Link-Span's own work is in the public domain under CC0 1.0. The Navi icon of DMR is not covered by
  CC0 (the author of the drawing is not recorded). Shipwright, NEI, Unbound and Wind Waker Style do not publish a
  license; the code ported from them stays with its authors. The NEI fork's assets in the NEI package and in the
  Shovel Demo stay with skijer and contributors; each package's notice says where they come from.

## For authors

Use the SDK of this release with the MSVC version recorded in `linkspan-sdk.json`. The guide
(`AUTHOR-GUIDE.md`, Portuguese in `AUTHOR-GUIDE.pt-BR.md`) covers building, validating with `shipmod.py`, packaging and
installing. The OoT contract (`OOT-NATIVE-CONTRACT.md`) and `SERVICES.md` list services, versions and pointer lifetime
rules. **Native mods built for RC9 must be rebuilt** against this SDK: the layout id changed from `7376240d` to
`ee6449e8`, even though no structure or service changed. Mods with a `.o2r` in their `assets/` folder should mount
it through the resource service, as NEI and the Shovel Demo do; such an archive must not carry a `portVersion` file
at its root. More examples are in [link-span-examples](https://github.com/BaiterYamato/link-span-examples).
