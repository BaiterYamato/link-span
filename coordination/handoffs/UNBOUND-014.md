# Handoff — UNBOUND-014

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review: peça de host na rodada W, correção de locale, fixture do lab e prova em jogo no D3D11.

## Resultado

- **Formato:** `xSpeed`/`ySpeed` opcionais por camada de `texScroll` e `twoTexScroll`, somados ao passo inteiro.
- **Host:** `AnimatedMatTexScrollParams` ganhou os dois floats (é o que muda o layout id desta rodada);
  `z_scene_proc.c` foi reescrito em cima de `MatAnim_ScrollOffset` (offset em quartos de texel calculado em f64,
  sem estado, com `fmod` de período 32768) e de `MatAnim_WriteScrollTile`, que escreve o tile size interpolado
  (`__gDPSetTileSizeLerp`, cinco palavras, coordenadas float) em vez do `Gfx_TexScrollEx` de sete palavras.
- **Volta do período:** o wrap passou de 2048 (512 texels, do MM) para 32768 quartos de texel, como a SPEC do
  Unbound pede; a volta só é invisível numa textura cujo tamanho divide o período.
- **Locale (causa da rolagem parada):** `setlocale(LC_ALL, ".UTF8")` no `main.c` adota o locale do usuário. Num
  Windows pt-BR o separador decimal do CRT vira vírgula e o `sscanf("%f")` do tinyxml2 para no ponto: `XSpeed="2.5"`
  virava 2 e `XSpeed="0.25"` virava 0. Agora só `LC_NUMERIC` volta para `C`, e o transcode do mod troca o separador
  do locale por ponto ao serializar. Isso valia para todo atributo float do XML de cena — posição de ator, spawn e
  ponto de path com fração eram truncados no ponto.

## Commits

Locais, no worktree `OOT-UPSTREAM-78DC-001` (nada foi empurrado):

- `f811f71d6` — rodada de host da fase W (floats no `AnimatedMatTexScrollParams`, `z_scene_proc.c`, factory).
- `224cd14af` — número fracionário do XML não depende do locale (host + transcode + teste).
- `2a9ce1965` — fixture: materiais do lab e do lab_b.

## Arquivos alterados

- soh/include/z64scene.h, soh/src/code/z_scene_proc.c
- soh/src/code/main.c
- soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.cpp
- soh/native-sdk/unbound-core/transcode.cpp, soh/native-sdk/unbound-core/format_tests.cpp
- soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py

## Validação executada

- `linkspan_unbound_format_tests` (inclui o caso de locale com vírgula decimal), `linkspan_unbound_json_tests`,
  `linkspan_unbound_converter_tests`, e os três testes nativos do host.
- Jogo: `soh.exe` `43CAD3FC` (layout `ce528052`), cópia `OOT-teste-cenas-20260914`. Sessões `a`, `d`, `e`, `f` em
  `build/oot-upstream-001/evidence/unbound-w/`; medida por FFT do perfil do quad listrado do lab
  (período de 8 texels = 30,6 px).

## Resultado da validação

- Testes: todos verdes.
- **Rolagem fracionária:** com `xSpeed 0.25` a textura anda **+3,85 texel a cada 3 s nos quatro intervalos**
  (`f-scroll-1..5.png`), ou seja 1,28 texel/s contra 1,25 esperado a 20 fps de jogo. Um passo inteiro seria
  5 texel/s, então a rolagem fracionária é mesmo mais lenta que o menor passo inteiro.
- **Antes da correção de locale:** `0,02 texel em 12 s` (parado) com `xSpeed 0.25`, e movimento compatível com 2,0
  quartos de texel por frame quando o valor era `2.5` — as duas metades da assinatura do truncamento no ponto.
- **Passo inteiro intacto:** a camada de `yStep 2` do segmento 9 continua rolando na vertical nas mesmas capturas.
- Save `file1.sav` restaurado (`1788F5DD`).

## Riscos

- `LC_NUMERIC` em `C` muda todo `printf`/`scanf` de número do host: nada no jogo formatava número para exibição
  pelo locale do usuário, mas é uma mudança global.
- O caminho não-Windows do `main` não chama `setlocale`; se alguma dependência chamar `setlocale(LC_ALL, "")` no
  Linux ou no macOS, o mesmo truncamento volta. Anotar se aparecer relato de mod com posição fracionária errada.
- O período de volta do offset mudou (2048 → 32768): uma textura que dependia do wrap curto do MM anda diferente.

## Pendências

- Exemplo `lake-hylia-slow-water` do plano não foi reproduzido; a prova usou a fixture do lab.

## Próxima ação recomendada

Review conjunta da rodada W (com UNBOUND-013, UNBOUND-015, CEL-002 e CEL-003).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 224cd14af e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 92d5174.
- Host: 224cd14af 2a9ce1965 f811f71d6 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
