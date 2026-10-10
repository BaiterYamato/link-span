**English** · [Português (Brasil)](FAQ-pt-BR)

# FAQ

**Do I need to rebuild the game for each mod?** No. Install Lua mods in an integrated host's `mods/`. Native mods compile their own provider against the matching SDK; a missing host service may require a host update.

**Does one mod work in both games?** A common-API mod can. Check its declared games and required capabilities; OoT-native services are not automatically available in MM.

**Is `.shipmod` different from ZIP?** No. It is a ZIP with another extension. Keep `manifest.toml` at the root.

**Are examples included with the core?** No. Get them from [link-span-examples](https://github.com/BaiterYamato/link-span-examples). A host release may include selected demos.

**Can I develop without a ROM?** You can validate manifests and run mock tests without one. Real gameplay and cross-world resource checks need your locally generated game archives.

**Must I own both games?** Single-game mods only need their target game's archive. Cross-world content needs the source game's archive; travel mods declaring `requires_both_games = true` need both.

**Are native providers sandboxed?** No. They run inside the host process. Install providers from sources you trust and use a matching SDK build.

**Can I publish extracted game assets?** Do not include ROMs or extracted copyrighted assets. Reference the player's archives and follow the licenses of any code or assets you redistribute.

**Where should I start?** [Getting Started](Getting-Started) for players; [Writing Your First Mod](Writing-Your-First-Mod) for authors; [Native Mods](Native-Mods) for providers.
