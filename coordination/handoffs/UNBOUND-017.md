# Handoff — UNBOUND-017

## Estado

review: parte de host do Unbound 0.8 (`97e8893`), com o layout `0d86df42` mantido.

## Resultado

- **Save por nome:** antes de gravar, o host guarda no bloco `linkspan.entrances` (v1) o nome estável e a camada:
  - da entrada atual;
  - da cena salva, só quando é cena de mod;
  - dos dois pontos do Farore's Wind (`fw` e a cópia de reserva do port).
  No `OnLoadFile`, antes do switch de spawn do `Sram_OpenSave`, o nome vence o número. Entrada vanilla continua
  como número (vai `null` no bloco).
- **Mod ausente:** cena ou entrada nomeada que nenhum mod registra cai no spawn padrão da idade (casa do Link
  criança, Templo do Tempo adulto), com aviso. Farore's Wind nomeado e ausente é apagado (`set = 0`).
- **Áudio:** `Audio_PlayNatureAmbienceSequence` sai cedo com id fora da tabela.
- **`SetMeshFactory`:** as duas fábricas reservam `dlists` pelo número de polígonos, e a malha de formato 1 não
  tem mais o `single` sobrescrito pelo ponteiro da lista multi.
- **Grave Hole Jumps:** remenda a colisão da cena que o jogo carrega. Com a base Unbound montada é o
  `scenes/spot02/scene.json`, e o fallback é o recurso vanilla. Confere tipo do recurso, comandos nulos e a
  contagem de tipos e polígonos antes de escrever.
- **Combobox:** valor fora da lista não indexa fora do vetor nem do mapa.
- **`global.sav`:** JSON ilegível ou com campo de tipo errado é reconstruído, com aviso, e os slots continuam
  visíveis no mesmo boot.
- **Registro de cenas:** o último grupo de entradas termina antes das entradas de retorno dinâmico. O limite fica
  em `0x7FF4` no `.cpp`; o `oot_scenes.h` segue com 32764 por causa do layout id.
- **Revisão independente** (Codex `gpt-5.6-sol`, esforço alto, só leitura, diff exportado e a referência 0.8).
  Quatro achados no host:
  - **Corrigido: Grave Hole Jumps.** Comando nulo (a fábrica grava `nullptr` para comando que não entende, e o
    `OTRScene_ExecuteCommands` pula) e recurso de outro tipo derrubariam o boot. É regressão desta rodada, porque
    antes ele só lia o arquivo vanilla. Agora usa `dynamic_pointer_cast` e pula nulos.
  - **Corrigido: `global.sav`.** Campo de tipo errado num JSON válido escapava do `try`.
  - **Não mudado: bloco com versão desconhecida.** Ele é ignorado, e os números da seção base valem. Os outros
    blocos do host (`linkspan.horse.scene`, `linkspan.scenes`) fazem igual; ver riscos.
  - **Não mudado: ponteiro estático do Grave Hole Jumps.** Upstream e referência fazem igual, e o remendo só roda
    no init e na troca da CVar; ver riscos.
  - O achado do framework sobre o teto do grupo (UNBOUND-016) foi corrigido aqui, no registro de cenas.
  - **Segunda passada** (mesmo modelo, só as correções): um achado, o `oot_scenes.h` ainda anunciar 32764; ver
    riscos. Uma auditoria dos docs de coordenação contra o código apontou oito lacunas em claims, plano e
    inventário, já corrigidas.
- **Host final:** `soh.exe` `9DDBC766A3B3C3FE2062F2B0616F0B304E56CD4392A523B067F049465D4C8E1B` (32.205.824 bytes),
  `soh.symbols` `28F2612BFEBA1A71E25D1E9B9CB7B5C88EE17AF4A602D4B20D3B23195D544828`. O `soh.o2r` não mudou
  (`C0D94440…`). Layout
  `0d86df42…34ef`: o delta toca só `.cpp`, `.c` e `UIWidgets.hpp`, fora do glob do id.
- **Hosts da rodada:**
  - anterior `C5B8A777…`;
  - intermediário `633D198F…` (sessões a, a2, b, c);
  - diagnóstico `1739A7B9…` (diag1);
  - `288FA208…` (sessões d a g), guardado em `OOT-teste-cenas-20260914\host-anteriores`.
- **Pacotes:**
  - framework 0.5.0 (`1A5D431E…`, UNBOUND-016);
  - Scene Demo 0.1.2 (`0580BB58…`);
  - Field Demo 0.1.1 (`9FF74145…`);
  - E-fixtures 0.1.1 (`878819A0…`), só local: tem dados do jogo;
  - NEI 0.3.1 (`F40EBAA4…`), com a fingerprint do host final, e a demo do NEI 0.3.1 (`7BF8A3A4…`);
  - escape demo `host-9ddbc766` (`F25BDDC4…`).
  DMR 0.2.9 e Mic 0.1.4 seguem valendo (layout igual, sem fingerprint de host).
- **Overlay:** `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-0d86df42-DMR-0.2.9-Mic-0.1.4-NEI-0.3.1-Win64-overlay.zip`
  (`D496F912EC8B39258D26DF59C3254C1949656D62D2AD6AB3C82326BE6D1DAD1F`, 33.965.041 bytes, 20.363 entradas).

## Commits

Locais, no worktree `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration` (sem push):

- `e77f09d17` — UNBOUND-016: formato do Unbound 0.8 no framework (0.5.0)
- `6fa3f2a2a` — UNBOUND-017: save por nome e correções de host do Unbound 0.8

O `soh/native-sdk/unbound-core/unbound_format.h` fica fora: a diferença é só de fim de linha, de outro agente.

## Arquivos alterados

- soh/soh/native/OotNativeSaveGame.cpp
- soh/soh/native/OotNativeScenes.cpp
- soh/native-sdk/tests/SceneRegistryTests.cpp
- soh/src/code/code_800EC960.c
- soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp
- soh/soh/Enhancements/Restorations/GraveHoleJumps.cpp
- soh/soh/SohGui/UIWidgets.hpp
- soh/soh/SaveManager.cpp
- tools/package-nei-core.ps1

## Validação executada

Windows 11, MSVC 19.44 (VS 2022), Release x64:

```powershell
cmake --build build/x64 --config Release --target soh
cmake --build build/x64 --config Release --target soh_symbols oot_native_scenes_tests oot_native_registry_tests oot_native_engine_tests
build/x64/Release/oot_native_scenes_tests.exe
build/x64/Release/oot_native_registry_tests.exe
build/x64/Release/oot_native_engine_tests.exe
tools/package-nei-core.ps1 ; tools/package-linkspan-overlay.ps1 (DMR 0.2.9, Mic 0.1.4, NEI 0.3.1)
tools/test-linkspan-overlay.ps1 -ResourceLogging -ExpectedMods 3 -NoCapture (base 9.2.3 oficial + oot.o2r FD11AD51)
sessões em jogo na cópia RELEASE-UNIFICADA\OOT-teste-cenas-20260914 (console do LUS, shiplua_open_file)
evidência build/oot-upstream-001/evidence/unbound-017 e unbound-017-diag
```

## Resultado da validação

- **Testes nativos:** os três executáveis passaram. O `SceneRegistryTests` cobre os nomes estáveis, os grupos
  `0x7FF8`/`0x7FFC` recusados, o `0x7FF4` aceito e o AUTO esgotado depois dele.
- **Sessão a** (host `633D198F`, framework + E-fixtures, sem Scene Demo): o save chegou a `lab_b` em
  `Scene Init - sceneNum: 0x84, entranceIndex: 0x624`, e o `reset` gravou `savedScene="linkspan_e/lab_b"` e
  `entrance.name="linkspan_e/lab_b/main"` (`file1.after-a.sav`).
- **Sessão c** (host `633D198F`, + Scene Demo, cuja `linkspan_demo/field_copy` vem antes de `linkspan_e/*`): o
  mesmo save abriu `lab_b` já renumerada, em `Scene Init - sceneNum: 0x85, entranceIndex: 0x628`
  (`registro: scenes=8; entrances=8`).
- **Sessões b, b0, a2 e diag1:** um load depois de `reset` com Autosave grava 13 vezes (`Save File - fileNum: 0`).
  - O host anterior (b0, com `areasSpoiled = 4294967295` antes) grava uma vez.
  - O build de diagnóstico (diag1, `unbound-017-diag`) mapeou as 12 extras para
    `CheckTracker::CheckTrackerLoadGame` → `SetAreaSpoiled`. O save gravado pelo `reset` sai com
    `trackerData.areasSpoiled = 0`.
  - É comportamento antigo do SoH, não desta rodada.
- **Sessão d** (host `288FA208`, E-fixtures ausente):
  - avisos `o arquivo está na cena 'linkspan_e/lab_b', que nenhum mod carregado registra` e
    `a entrada 'linkspan_e/lab_b/main' não está registrada`;
  - spawn padrão em `Scene Init - sceneNum: 0x34, entranceIndex: 0xbb`.
- **Sessão e** (host `288FA208`, `global.sav` truncado):
  - `Global save could not be read: [json.exception.parse_error.101]` e reconstrução;
  - o log segue em `Init Meta - fileNum: 0`, e `e-1-arquivos.png` mostra o arquivo 1.
- **Sessão f** (host `288FA208`, framework + NEI 0.3.1 + escape demo): `host: 313/314 desvios, recusados:
  Interface_Draw`. A escape demo carrega antes e fica com o desvio.
- **Sessão g** (host `288FA208`, NEI sozinho): `host: 314/314 desvios`. Sem o framework, o Grave Hole Jumps
  remendou `scenes/shared/spot02_scene/spot02_scene`.
- **Grave Hole Jumps:** as sessões com o framework (a, a2, b, c, diag1, d, e, f) registram
  `remendando a colisão de scenes/spot02/scene.json` cerca de 1 s depois do init.
- **Sessão h** (host final `9DDBC766`, framework + E-fixtures + Scene Demo, as correções da revisão já dentro):
  - `registro: scenes=8; entrances=8; notes=0; layers=2`;
  - `Grave Hole Jumps: remendando a colisão de scenes/spot02/scene.json`;
  - o save que nomeia `lab_b` abriu em `Scene Init - sceneNum: 0x85, entranceIndex: 0x628`, e depois a Scene Demo
    levou o jogador para `0x80`/`0x614`.
- **Sessão i** (host final, NEI 0.3.1 sozinho, `global.sav` válido com `"audioSetting":"alto"`):
  - `Global save could not be read: [json.exception.type_error.302] type must be number, but is string. We are
    reconstructing it.`, seguido de `Init Meta - fileNum: 0`;
  - `i-1-arquivos.png` mostra o arquivo 1, e o `global.sav` volta com os padrões;
  - `host: 314/314 desvios`.
- **Prova limpa do overlay** (base 9.2.3 oficial + `oot.o2r` FD11AD51, `-ResourceLogging`, host `9DDBC766`):
  - 7.709 arquivos preservados byte a byte e 20.362 entradas de `checksums.sha256` conferidas;
  - `ShipLua carregou 3 mod(s)` (NEI 0.3.1, DMR 0.2.9, Mic 0.1.4) e 371 comandos de cena em XML;
  - vivo aos 170 s, sessão válida: PROVA OK (`evidence/unbound-017/overlay-9ddbc766`, `smoke-overlay-9ddbc766.txt`);
  - o overlay anterior desta rodada (`38FEF18F`, host `288FA208`) também deu PROVA OK e foi substituído.
- Nenhum log das sessões tem `[critical]` ou exceção.

## Riscos

- **NEI + escape demo:** instalados juntos, o NEI perde o desvio de `Interface_Draw` (o primeiro desvio vence).
- **Save antigo:** sem o bloco `linkspan.entrances`, carrega pelo número, como antes. O bloco nasce na primeira
  gravação com este host.
- **Bloco com versão desconhecida:** um save gravado por um host futuro, com bloco v2, carrega pelos números da
  seção base. Se a ordem dos mods mudou, pode abrir outra cena de mod. Uma v2 deve manter os campos da v1 legíveis.
- **Farore's Wind:** nomeado e ausente é descartado (`set = 0`), de propósito. As sessões não tinham FW ativo, então
  isso está coberto pelo código, não por prova em jogo.
- **Contrato do registro:** o `oot_scenes.h` ainda declara e documenta 32764 como índice máximo (linhas 12 e 35),
  mas o host recusa grupo acima de `0x7FF4`. Um mod nativo que peça `0x7FF8` ou `0x7FFC` recebe
  `SHIP_NATIVE_INVALID_ARGUMENT`. Mudar o header muda o layout id, então o limite novo fica para o próximo bump de
  layout. O framework só pede AUTO e não é afetado.
- **Grave Hole Jumps:** o remendo é aplicado uma vez por sessão, na cena que o jogo usa no boot. Trocar ou tirar um
  mod do cemitério com o jogo aberto não reaplica o remendo, como no upstream.
- **Scene Demo:** logo depois do load, ela leva o jogador para `linkspan_demo/field_copy` pelo nome. Um `reset` em
  seguida grava lá (sessão c). Não use essa demo em testes de save.
- **E-fixtures 0.1.1:** tem dados do jogo; fica só local.
- **Cópia de teste:** `RELEASE-UNIFICADA\OOT-teste-cenas-20260914` ficou com o host `9DDBC766`, só o NEI 0.3.1 em
  `mods` e o `global.sav` reconstruído. O `288FA208` (exe e símbolos) está em `host-anteriores`.

## Pendências

- Instalar na `RELEASE-UNIFICADA\OOT` só a pedido. O NEI 0.3.0 de lá não casa com o host novo: vai o 0.3.1.
- Lote de desvios do escape hatch com `MH_ApplyQueued`: não aprovado.
- Warp point por nome: opcional.
- `coordination/STATUS.md`: julgamento manual.

## Próxima ação recomendada

Review conjunta com a UNBOUND-016.
