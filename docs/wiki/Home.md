**English** · [Português (Brasil)](Home-pt-BR)

# Link-Span Wiki

Link-Span is a shared Lua 5.4 modloader for Ocarina of Time in Shipwright and Majora's Mask in 2Ship2Harkinian. The Lua API is **0.5.0**. A mod using the common API can target both games; game-specific features require the host's capabilities.

## Start with a task

- [Getting Started](Getting-Started)
- [Writing Your First Mod](Writing-Your-First-Mod)
- [Native Mods](Native-Mods)
- [Example Mods](Example-Mods)
- [Cross-World Assets](Cross-World-Assets)
- [Packaging and Publishing](Packaging-and-Publishing)
- [Troubleshooting](Troubleshooting)
- [FAQ](FAQ)

## What it provides

- An isolated Lua state for each mod, manifests, compatibility checks and dependency ordering.
- Events, rebindable hotkeys and an actor API with opaque handles.
- Folder, ZIP and `.shipmod` loading; a development CLI and a ROM-free mock runtime.
- Cross-world resource namespaces and a launcher for game selection and world travel, where supported by the installed hosts.
- Native providers and core extensions. Native code runs inside the game process; Lua isolation does not sandbox a DLL.

The OoT installation described here uses the **oot-0.5.0 RC8 overlay for Shipwright 9.3.0**. MM and launcher setup have their own requirements; see [Getting Started](Getting-Started).

## Full references

[Modder guides](https://github.com/BaiterYamato/link-span/blob/main/docs/README.md) · [Lua API](https://github.com/BaiterYamato/link-span/blob/main/generated/docs/api-reference.md) · [Examples repository](https://github.com/BaiterYamato/link-span-examples)

Examples live in the separate repository. Core test fixtures are not the modder examples.

## Research (pt-BR)

These internal research records remain in Portuguese. They are background material, not installation guides.

- [Research-External-Item-Systems](https://github.com/BaiterYamato/link-span/blob/main/docs/wiki/Research-External-Item-Systems.md)
- [Research-Survival-System](https://github.com/BaiterYamato/link-span/blob/main/docs/wiki/Research-Survival-System.md)
