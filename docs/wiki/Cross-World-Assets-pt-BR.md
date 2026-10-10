[English](Cross-World-Assets) · **Português (Brasil)**

# Assets entre mundos

Gere `oot.o2r` e `mm.o2r` localmente pelo fluxo normal de extração de cada jogo. Cada host com Link-Span pode ler o archive do outro jogo ao vivo; não inclua esse archive no mod.

## Organize pastas de jogos irmãs

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

| Host em execução | Archive vizinho | Namespace de recursos |
|---|---|---|
| OoT | `../MM/mm.o2r` | `mm/` |
| MM | `../OOT/oot.o2r` | `oot/` |

No Windows, os nomes das pastas irmãs não diferenciam maiúsculas e minúsculas. Para outra organização, defina `SHIPLUA_MM_ROOT` no OoT ou `SHIPLUA_OOT_ROOT` no MM apontando para a pasta do jogo vizinho. O log inicial informa o caminho procurado e se montou o archive. Sem ele, os recursos entre mundos ficam indisponíveis; um mod comum de um só jogo não precisa dos dois archives.

## Experimente os exemplos

- [`elegy-statue`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/elegy-statue): no OoT, K alterna a estátua MM usando a chave de ator `oot.mm_elegy_statue`.
- [`rauru`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/rauru): no MM, K alterna Rauru usando `mm.oot_rauru`.
- [`oot-kokiri-sword`](https://github.com/BaiterYamato/link-span-examples/tree/main/lua/oot-kokiri-sword): no MM, T muda o visual da espada usando recursos OoT.

Instale o exemplo separadamente; confirme a montagem e as capabilities necessárias no log. Os recursos usam `mm/` ou `oot/`. O host cria aliases dos hashes de recursos de renderização que não conflitam e sanitiza diferenças de dialeto de display lists.

Ler recursos de renderização não executa o jogo vizinho. Suas cenas e áudio não ficam automaticamente utilizáveis no outro motor. Os formatos de recursos de esqueletos precisam continuar compatíveis, e materiais podem diferir. Viagens também exigem suporte do launcher/host e `world.travel`; montar um archive sozinho não oferece viagem.

Contrato completo: [Assets entre mundos](https://github.com/BaiterYamato/link-span/blob/main/docs/cross-world-assets.pt-BR.md) · [Launcher](https://github.com/BaiterYamato/link-span/blob/main/docs/launcher.pt-BR.md).
