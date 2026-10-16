# Handoff — NEI-004A

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **O fork NEI roda na DLL do coremod `linkspan.nei`.**
  - O código de itens do `skijer/Not-Enough-Items` (`c29262b`, base `783139310`) compila em C dentro da
    `linkspan_nei_core.dll`.
  - Chama o `soh.exe` só por endereços resolvidos no init pelo escape hatch (ABI 1.3).
  - A DLL depende apenas da CRT e do `KERNEL32`.
  - Sem escape hatch para aquele `soh.exe`, o registro NEI-002 segue funcionando e o fork fica desligado.
- **Montagem (`soh/native-sdk/nei-core/fork/`):**
  - `sync.py` extrai as árvores do fork do commit fixo e aplica `patches/`:
    - `SurfaceType` decodificado e vértices `Vec3i` do host;
    - ids runtime dos botões traduzidos para os ids lógicos do fork.
  - `gen_compat.py` gera a camada `/FI`: enumeradores e declarações novas, mais 12 headers-sombra.
  - As 213 variáveis globais do host viram ponteiros resolvidos.
  - Funções que o fork acrescentou em arquivos do host (`extract.txt`) são copiadas com o fecho de
    dependências por `extract_additions.py`:
    - z_player (12);
    - bg_ice_shelter (7);
    - jya_ironobj e m_thunder (1 cada);
    - 20 funções de armazenamento do `nei_save.cpp`, C++ `extern "C"` convertido para C.
  - O que ficou de fora vira stub "recurso ausente" gerado por `gen_stubs.py` a partir de `stub-names.txt`
    (124 funções e 10 variáveis): rede Harpoon/Fleet, formas do MM, draw do randomizer e modelos C.
  - Display list de stub termina em `G_ENDDL`.
  - `gen_imports.py` lê os objetos COFF e gera thunks MASM mais a tabela de resolução: 287 funções e 32 dados.
- **Cola (`fork_glue.cpp`):**
  - desvia `Player_Update`/`Player_Draw` para `CustomItems_Update`/`CustomItems_OverrideDraw`, nos mesmos
    pontos do z_player do fork;
  - define Roc's Feather e Roc's Cape no registro `linkspan.nei.items`, com ícone e modelo provisórios até o
    NEI-007;
  - `nei-core` 0.2.0 declara ABI 1.3 e o `host_fingerprints` do `soh.exe` alvo.
- **Host (sem mudar o layout id `31899f1f`):**
  - Com LTCG, metade da API do jogo era inlinada em todos os usos e sumia do PDB.
    `OotNativeHostKeep.c`, gerado por `tools/gen-host-keep.py`, toma o endereço de cada protótipo de
    `functions.h` com definição no host, mais os extras de `tools/host-keep-extra.txt`.
  - O link Release x64 passou a `/OPT:NOICF`: o ICF fundia corpos idênticos em 1368 entradas `folded`, que o
    `resolve_symbol` recusa.
  - O `soh.exe` foi de 27,7 para 31,5 MB.
- **Demo:** `linkspan.nei-demo` 0.2.0 ganhou `fork_give`. **J** dá o Feather e **U** dá o Cape, os dois no
  C-esquerdo.

## Commits

- host `40530741c` — tabela de keep e link sem ICF.
- host `48569f926` — fork NEI na DLL (fase F).
- Os scripts `gen_imports.py`, `extract_additions.py` e `gen_stubs.py` tiveram a primeira versão escrita por
  subagentes Codex (terra, esforço alto). A versão final foi revisada e corrigida aqui:
  - COMMON do C tratado como definição;
  - remoção de comentários em uma passada;
  - `namespace {` e `extern "C" {` tratados como topo;
  - modo `.cpp` só do fork.

## Arquivos alterados

- `soh/native-sdk/nei-core/` (CMake, `nei_core.cpp`, demo, `fork/`)
- `tools/package-nei-core.ps1`
- `tools/gen-host-keep.py`, `tools/host-keep-extra.txt`, `tools/host-keep-exclude.txt`
- `soh/soh/native/OotNativeHostKeep.c`
- `soh/CMakeLists.txt`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh ; --target soh_symbols
  soh.exe B1A90CAA73E9…, 33539 símbolos, 2 folded (CRT), layout 31899f1f
cmake -S soh/native-sdk/nei-core -B build/nei-core-native -DOOT_NATIVE_SDK=… ; cmake --build … --config Release
  gen_imports: 287 funções, 32 dados, 0 erros; linkspan_nei_core_tests: "nei core: ok"
tools/package-nei-core.ps1 -> LinkSpan-NEI-Core/Demo-0.2.0-layout-31899f1f.zip (validador ok)
playtest na cópia RELEASE-UNIFICADA\OOT-teste-cenas-20260914 (evidência build/oot-upstream-001/evidence/nei-f)
  s1: crash (Nei_Save stub NULL) -> corrigido extraindo nei_save.cpp
  s2: sem crash; caixa de texto do demo aberta cobriu o teste
  s3: Feather parado (f2-03/04) e em corrida (f2-06) no ar; usos contados pelo registro
  s4: Cape: pulo (f3-04) e pulo duplo (f3-06); saída 0; save restaurado (1788F5DD)
```

## Resultado da validação

- O fork fica ativo (`fork: ativo (2 itens)`) com os 319 símbolos resolvidos e os dois desvios instalados.
- Roc's Feather: pula parado e em corrida.
- Roc's Cape: pulo e pulo duplo.
- 4 sessões válidas pelo harness. As duas últimas terminaram sem erro com saída 0.
- Um subagente Codex fez análise estática dos stubs que devolvem NULL/0: nenhum é alcançável pelo
  Feather/Cape. As display lists de stub sem `G_ENDDL` foram corrigidas no gerador.

## Pendências

- **NEI-007:** ícone e modelo do Feather/Cape são provisórios (hookshot e coração); os assets do fork não têm
  licença.
- **NEI-012:** o `NeiSaveData` do fork vive só em memória. A persistência no save por namespace fica para o
  NEI-012, e `Nei_TrirodNotify`/`Picto_*` seguem stub.
- **NEI-008:** a onda A ainda precisa de aquisição real (get-item do fork), animação MM (CVar
  `RocsItemsUseMmAnims`) e save. Shovel, Somaria e rods usam modelos C que hoje são stub (ver
  `stub-names.txt`); entram com o NEI-007.
- **Host:** `tools/gen-host-keep.py` já corrige a remoção de comentários. Regenerar a tabela no próximo rebuild
  acrescenta 3 funções de AudioLoad.
- **Não validado em jogo:** diferenças de comportamento de funções que o fork alterou no host e que ficam com a
  versão do host:
  - `Player.ivanFloating`, `SaveContext.ship.extButtons` e campos de cone do `EnMThunder`;
  - lista completa em `build/nei-core-native/nei-fork/compat-report.txt`.

## Riscos

- **O escape hatch amarra o pacote a um `soh.exe`.**
  - Todo rebuild do host muda o fingerprint e exige reempacotar o `linkspan.nei`.
  - Se o LTCG inlinar uma função nova que o fork chama, é preciso um extra no keep e outro rebuild.
  - O gate F ("sem recompilar o host durante as ondas") depende de a rodada de host da fase W trazer o keep
    completo.
- **O `sync.py` aplica patches com `git apply` dentro do worktree do host.** O teto
  `GIT_CEILING_DIRECTORIES` evita o descarte silencioso de caminhos fora do diretório atual; sem ele, os
  patches não entram e o erro só aparece na compilação.
- **O update do fork roda também na tela de título**, com o Player da demo. Um stub que devolva NULL num
  caminho por frame derruba o jogo antes do save carregar, como o `Nei_Save` fez na s1.

## Próxima ação recomendada

- NEI-008 onda A completa, nesta ordem:
  1. persistência do `NeiSaveData` (NEI-012);
  2. aquisição pelo get-item do fork;
  3. assets licenciáveis do Feather/Cape (NEI-007).
- Depois: Shovel e Somaria.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 40530741c e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 1f7b781.
- Host: 40530741c 48569f926 481b5a8f5 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
