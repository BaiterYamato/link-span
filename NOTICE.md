# Notice — Link-Span

Link-Span is dedicated to the public domain under CC0 1.0 Universal. The legal code is in [LICENSE](LICENSE).
This covers the work of the Link-Span authors in this repository; the parts below keep their own terms.

## Build dependencies

CMake fetches these at build time and compiles them into the modloader:

| Component | Version | License | Copyright |
|---|---|---|---|
| Lua | 5.4.7 | MIT | Copyright (C) 1994-2024 Lua.org, PUC-Rio |
| toml++ | 3.4.0 | MIT | Copyright (c) Mark Gillard |
| miniz | 3.1.2 | MIT | Copyright 2013-2014 RAD Game Tools and Valve Software; 2010-2014 Rich Geldreich and Tenacious Software LLC |

Their license texts come with their sources and with each release (`THIRD-PARTY-NOTICES.md`).

## Hosts

The hosts under `hosts/` are separate projects. [2Ship2Harkinian](https://github.com/2ship2harkinian/2ship2harkinian)
is CC0 1.0. [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright) (HarbourMasters/Shipwright) publishes no
license; its code stays with its authors, and so does the code that the OoT host and the Link-Span mods port from the
projects below, which publish no license either. Each release package states what it ports in its own `NOTICE.md`.

## Credits and sources

Projects Link-Span takes code, data or design from, with the exact version:

| Project | Source | What Link-Span uses | License |
|---|---|---|---|
| Ship of Harkinian (HarbourMasters and contributors) | [HarbourMasters/Shipwright](https://github.com/HarbourMasters/Shipwright), tag 9.3.0 (`ecd889c201`) | the OoT host, forked as [BaiterYamato/Shipwright](https://github.com/BaiterYamato/Shipwright) | none published |
| 2Ship2Harkinian (HarbourMasters and contributors) | [2ship2harkinian/2ship2harkinian](https://github.com/2ship2harkinian/2ship2harkinian) | the MM host, forked as [BaiterYamato/2ship2harkinian](https://github.com/BaiterYamato/2ship2harkinian); the OoT host mounts its `mm.o2r` cross-world | CC0 1.0 |
| Ocarina of Time decompilation (zeldaret) | [zeldaret/oot](https://github.com/zeldaret/oot) | the code Shipwright builds on | none published |
| Majora's Mask decompilation (zeldaret) | [zeldaret/mm](https://github.com/zeldaret/mm), through the copy in 2Ship2Harkinian | the code 2Ship builds on, and the MM sound effect engine ported into the OoT host | none published |
| Not Enough Items (skijer and contributors) | [skijer/Shipwright, branch `Not-Enough-Items`](https://github.com/skijer/Shipwright/tree/Not-Enough-Items), commit `c29262b` | the item code in the NEI mod, the MM Goron form in the OoT host, and the `nei-assets` players generate on their own machine | none published |
| Unbound mod API (skijer) | [skijer/Shipwright, branch `unbound-modapi`](https://github.com/skijer/Shipwright/tree/unbound-modapi), commit `27bb6c445` | OoT host fixes in messages, the kaleido equipment panel and the audio tables (UNBOUND-032) | none published |
| Unbound (roborich and contributors) | [roborich/Shipwright](https://github.com/roborich/Shipwright), releases up to [9.3.0-unbound0.11](https://github.com/roborich/Shipwright/releases/tag/9.3.0-unbound0.11), including [9.2.3-unbound0.9](https://github.com/roborich/Shipwright/releases/tag/9.2.3-unbound0.9) and [9.3.0-unbound0.10](https://github.com/roborich/Shipwright/releases/tag/9.3.0-unbound0.10) | the Unbound Framework mod (format, type reader, `baseVersion`) and OoT host fixes for randomizer spawns, Anchor flags and room ids (UNBOUND-032) | none published |
| Mic Ocarina (skijer) | [`MicOcarina.cpp` in skijer/2ship2harkinian, branch `Not-Enough-Items`](https://github.com/skijer/2ship2harkinian/blob/Not-Enough-Items/mm/2s2h/Enhancements/Ocarina/MicOcarina.cpp), commit `5608ab477d` | the pitch detector of the Mic Ocarina mod, modified by the maintainer | CC0 1.0 |
| Wind Waker Style (roborich) | [roborich/Shipwright, branch `wind-waker-style-cel-shading`](https://github.com/roborich/Shipwright/tree/wind-waker-style-cel-shading), commit `cbcd3e719d` | the Wind Waker Style mod | none published |
| libultraship and Torch | [Kenix3/libultraship](https://github.com/Kenix3/libultraship), [HarbourMasters/Torch](https://github.com/HarbourMasters/Torch) | engine and asset extractor of both hosts | MIT |

Consulted while designing, with no code taken from them: [OoTMM](https://github.com/OoTMM/OoTMM) and
[ComboShip](https://github.com/Varuuna/ComboShip), on how MM forms can run inside OoT; and
[noclip.website](https://noclip.website), whose reproduction of Wind Waker's torch light the Wind Waker Style fork
credits.

The Navi icon on the DMR D-pad HUD is a cropped, downscaled capture of Navi from Ocarina of Time; the reference
image is [Zelda Wiki's `File:OoT_Navi_Model.png`](https://zeldawiki.wiki/wiki/File:OoT_Navi_Model.png). Navi belongs to Nintendo, so the icon is not
covered by the CC0 dedication.

## Game data

No ROM, extracted game asset or save is part of this repository or of its releases.
