# Handoff — NEI-HOST-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **Por que uma rodada de host.**
  - O fork NEI mudou 314 funções do host: z_player, z_player_lib, z_parameter, kaleido, z_actor, câmera,
    chefes e atores.
  - Sem sobrepor essas funções, o código do fork que roda na DLL chama a versão vanilla. O rod era sacado,
    mas não ganhava a ação de item; os ícones e o pipeline de Player divergiam.
  - O usuário escolheu, em 2026-09-25, fazer uma rodada de host antes das ondas. O Gate F passa de "sem
    recompilar o host" para "uma rodada antes das ondas".
- **Host (`soh.exe`).**
  - `soh/hookable_sources.txt` lista os 81 arquivos compilados sem inline e sem LTCG. Assim cada função tem
    endereço próprio para o desvio.
  - O layout id continua `ce528052`: só mudou `.c`/`.cpp` e flags.
  - O `soh.exe` da cópia de teste passou a `0D7D8899`.
- **DLL.**
  - `fork/overlay.py` lista as funções que o fork mudou. O `extract_additions --tabela` copia cada uma e gera
    `nei_overlay_<arquivo>`, e o `overlay_glue` desvia todas no `StartOverlays`.
  - Merges à mão, para funções que o fork e o upstream mudaram, ficam em `overlay-merges/`; extras em
    `overlay-extra/`; exclusões em `overlay-exclude.txt`.
  - Dados:
    - estáticos repetidos viram `nei_q_<arquivo>__<nome>`;
    - endereços de função do host viram `nei_fnaddr_`;
    - o `.c` de dados incluído (`#include "x.c"` sob `soh/src`) entra no texto extraído, o que corrigiu o crash
      no `Camera_Update` (`gDbgCamEnabled` e `D_8015BD7C`).
- **Core (extern/ship-lua).** O limite de patches por provider passou de 256 para 1024: com 256, o 257º desvio
  voltava `LIMIT` e o kaleido do fork ficava desligado.
- **Correções que a prova em jogo exigiu.**
  - **Id runtime × id lógico.**
    - O botão guarda o id runtime do `linkspan.oot.items` (0xA0-0xEF), faixa que cruza os ids lógicos do fork.
    - O `player_unit.c` redefine `B_BTN_ITEM`/`C_BTN_ITEM`/`DPAD_ITEM` com `NeiFork_ToLogicalItem`.
    - O `custom_items.c` e o `extended_player.c` já traduzem pelo patch 0002 e ficam de fora: traduzir duas
      vezes erra.
  - **VB do host na faixa sintética.**
    - `VB_ITEM_ACTION_BE_NONE` e `VB_CHANGE_HELD_ITEM_AND_USE_ITEM` bloqueiam item de mod.
    - O `extracted-fixes.txt` deixa passar o item do fork (`NeiFork_IsForkItem`), que monta a ação dele no
      `Player_UseItem` do fork.
  - **Array `extern` incompleto.** `sItemButtons[]` dava `ARRAY_COUNT` 0 (o MSVC só avisa). A declaração
    externa ganha `[N]`, contado no inicializador.
  - **Stubs que devolviam 0 quando 0 é um valor.**
    - Forma 0 é a Fierce Deity, multiplicador 0 para o Link e textbox 0x6F0A no `Player_Init` pelo
      `MmBottle_*`.
    - O `PikaMode_ButtonIcon(btn, orig)` devolvia NULL: todo ícone de botão sumia.
    - 20 nomes foram para `stubs.c` com valor neutro ou repasse do argumento.
  - **C4013.**
    - 88 funções eram chamadas sem protótipo; o retorno era lido como `int`.
    - O `PikaMode_ButtonIcon` corrigido passou a devolver o ponteiro truncado a 32 bits, e o Fast3D caiu no
      `strncmp` do `OtrSignatureCheck`.
    - `f32` e `u8` sem protótipo também voltavam errados (multiplicadores de corrida e testes booleanos).
    - O `fork/nei_fork_protos.h` vai por `/FI` com os protótipos do fork, e o alvo `nei_fork` compila com
      `/we4013`.
- **Instrumento.** O `pipeline_probe.c` conta:
  - o último botão lido;
  - o último `Player_UseItem`;
  - o gatilho do golpe (`NeiProbe_Attack` no `Player_ActionHandler_7`).

  O hotkey `stats_now` imprime a linha na hora.

## Commits

- host `4f40d1126` — rodada de host com as funções do host que o fork mudou (NEI-HOST-001).
- core `d5aba71` (extern/ship-lua, branch `agent/NEI-HOST-001-patch-cap`, sem push) — limite de patches 1024.

## Arquivos alterados

- `soh/CMakeLists.txt`, `soh/hookable_sources.txt` (novo), `soh/native-sdk/tools/symdump.cpp`
- `soh/native-sdk/nei-core/CMakeLists.txt`, `core-main.lua`
- `soh/native-sdk/nei-core/fork/`:
  - novos: `overlay.py`, `overlay_glue.cpp/.h`, `overlay_table.h`, `overlay-merges/`, `overlay-extra/`,
    `overlay-exclude.txt`, `nei_ext.h`, `nei_fork_protos.h`;
  - alterados: `extract_additions.py`, `sync.py`, `gen_imports.py`, `player_unit.c`, `fork_items.c`, `stubs.c`,
    `stub-names.txt`, `extracted-fixes.txt`, `nei_host_overrides.h`, `pipeline_probe.c`, `actor_guard.c`,
    `fork_glue.cpp`, `extract.txt`
- `extern/ship-lua`: `src/native/NativeProvider.cpp`, `docs/native-providers.md`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh ; --target soh_symbols
cmake build/nei-core-native ; cmake --build build/nei-core-native --config Release   (0 erros, 0 C4013)
tools/check-nei-fork-helpers.ps1: 1081 funções e 602 dados resolvidos, 0 erros; DLL só com CRT e KERNEL32
ctest --test-dir build/nei-core-native -C Release: 1/1
tools/package-nei-core.ps1 -> LinkSpan-NEI-Core-0.2.0-layout-ce528052.zip
playtest RELEASE-UNIFICADA\OOT-teste-nei-003 (soh.exe 0D7D8899), evidência build/oot-upstream-001/evidence/nei-host-001
```

## Resultado da validação

- `stats` em todas as sessões: `host: 314/314 desvios`, `kaleido: ativo (3 desvios)`, `fork: ativo (24 itens)`.
- Sessão s5 (sequência completa da onda A): sem crash; os ícones do HUD estão certos, e o resultado de cada item
  está no handoff do NEI-008.
- Sessões anteriores: s1 caiu no textbox 0x6F0A (stubs do MmBottle), s2 no `Camera_Update` (dados duplicados) e
  s3 no Fast3D (C4013). Cada uma virou uma correção acima.

## Pendências

- **A `RELEASE-UNIFICADA\OOT` do usuário ainda tem o `soh.exe` anterior.** O NEI deste pacote exige o
  `soh.exe` novo, porque as funções de `hookable_sources.txt` precisam de endereço próprio. Atualizar a release
  com o host novo, junto com o overlay.
- **Push** do host e do branch do core: aguarda autorização.
- **Instrumentos a remover no NEI-016:**
  - as linhas `NeiProbe_*` do `extracted-fixes.txt`;
  - o hotkey `stats_now`.

  O `pipeline_probe.c` em si é do NEI-005.
- **Guarda textual.** O `overlay.py` compara o texto das funções. Uma mudança do fork só em macro ou em header
  não vira desvio; os merges à mão precisam ser refeitos a cada sync do fork.

## Riscos

- 314 desvios com MinHook no mesmo processo. Uma função do host que o fork mudou e que outro mod também desvia
  fica com o último que instalou.
- O `soh.exe` sem inline nos 81 arquivos pode ficar um pouco mais lento. Não houve queda perceptível de frame
  nas sessões.

## Próxima ação recomendada

- Review do NEI-HOST-001 junto com o NEI-008.
- Depois, NEI-009 (onda B), sem nova rodada de host, conforme o Gate F revisado.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 4f40d1126 na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: d5aba71.
- Host: 4f40d1126 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
