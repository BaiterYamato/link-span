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

The hosts under `hosts/` are separate projects. 2Ship2Harkinian is CC0 1.0. Ship of Harkinian
(HarbourMasters/Shipwright) publishes no license; its code stays with its authors, and so does the code that
Link-Span mods port from Not Enough Items (skijer) and Unbound and Wind Waker Style (roborich), which publish
no license either. Each release package states what it ports in its own `NOTICE.md`.

## Game data

No ROM, extracted game asset or save is part of this repository or of its releases.
