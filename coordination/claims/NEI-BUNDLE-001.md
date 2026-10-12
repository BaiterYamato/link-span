# NEI-BUNDLE-001

- Status: review
- Agent: Claude
- Platform: Windows
- Branch: agent/NEI-BUNDLE-001 (link-span, BaiterYamato/Shipwright e BaiterYamato/link-span-examples)
- Started: 2026-10-12T14:25:00Z
- Depends on: OOT-CREDITS-001 (o host nasce do commit 8b75e3c326), OOT-UPSTREAM-003 (RC8 publicado)
- Files:
  - README.md, README.pt-BR.md
  - docs/wiki/{Getting-Started,Troubleshooting}{,-pt-BR}.md
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/{assets.cpp,assets.h,assets/README.md,fork_save.cpp,tests.cpp,NOTICE.md,README*.md}
  - OOT-UPSTREAM-78DC-001/soh/native-sdk/nei-core/fork/{extracted-fixes.txt,fork_items.c,inventory_unit.c,nei_host_overrides.h,resource_guard.c}
  - OOT-UPSTREAM-78DC-001/tools/{build-nei-assets.py,package-nei-core.ps1}
  - OOT-UPSTREAM-78DC-001/{NOTICE-LINKSPAN.md,docs/licensing/OVERLAY-NOTICE.md}
  - link-span-examples/oot-native/shovel-demo/**, link-span-examples/tools/package-shovel-demo.ps1
  - logs/rc-pipeline-oot-next/** (rc-build, rc-package, rc-post-build; fora de repositório)
- Goal:
  - Pedido do mantenedor: os assets do NEI vão sempre dentro do mod. A partir do NEI 0.3.24 o pacote
    `LinkSpan-NEI-*.shipmod` leva, em `assets/`, o núcleo, a expansão SM64, a expansão SSBB e o manifesto; quem joga não
    gera nem copia nada. `<jogo>/nei-assets/` só vale se o pacote não tiver o núcleo.
  - Texturas do fork que eram cópia exata de textura do jogo (17) não entram no pacote: os modelos leem pelo caminho
    do jogo no `oot.o2r`/`mm.o2r` do jogador.
  - O menu de pausa deixa de cair quando falta a textura do nome de um item. Shovel Demo 0.1.5 leva ícone, nome e modelo
    da pá no próprio zip.
  - Docs do core (wiki e README) descrevem o comportamento novo.
