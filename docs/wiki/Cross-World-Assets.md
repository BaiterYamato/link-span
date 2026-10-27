**English** · [Português (Brasil)](Cross-World-Assets-pt-BR)

# Cross-World Assets

Generate `oot.o2r` and `mm.o2r` locally through the normal extraction flow of each game. Each Link-Span-enabled host can read the other game's archive live; do not bundle that archive in your mod.

## Arrange sibling game folders

```text
Link-Span/
└── hosts/
    ├── oot/
    │   ├── soh.exe
    │   └── oot.o2r
    └── mm/
        ├── 2ship.exe
        └── mm.o2r
```

| Running host | Neighbor archive | Resource namespace |
|---|---|---|
| OoT | `../MM/mm.o2r` | `mm/` |
| MM | `../OOT/oot.o2r` | `oot/` |

On Windows, the sibling names are case-insensitive. For another layout, set `SHIPLUA_MM_ROOT` for OoT or `SHIPLUA_OOT_ROOT` for MM to the neighboring game directory. The startup log reports the searched path and whether the archive was mounted. If absent, cross-world resources are unavailable; an ordinary single-game mod does not need both archives.

## Try the examples

- `elegy-statue` (retired): the MM statue in OoT moved to the host's `puppets` mod shell with RFC 0029, outside the RC.
- [`rauru`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru): in MM, K toggles Rauru using `mm.oot_rauru`.
- [`oot-kokiri-sword`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/oot-kokiri-sword): in MM, T changes the sword appearance using OoT resources.

Install the example separately; confirm the mount and any required capabilities in the host log. Resources are addressed under `mm/` or `oot/`. The host aliases non-colliding render-resource hashes and sanitizes display-list dialect differences.

Reading render resources does not execute the neighboring game. Its scenes and audio are not automatically usable in the other engine. Skeletal resource formats must remain compatible, and materials can differ. World travel additionally needs launcher/host support and `world.travel`; mounting an archive alone does not provide travel.

Full contract: [Cross-world assets](https://github.com/BaiterYamato/link-span/blob/main/docs/cross-world-assets.md) · [Launcher](https://github.com/BaiterYamato/link-span/blob/main/docs/launcher.md).
