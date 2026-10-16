# Handoff — COREEXT-010

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- **Problema.** A MinHook 1.3.4 congela as threads do jogo a cada `MH_EnableHook`/`MH_DisableHook` (~60 ms: snapshot
  das threads do sistema e suspensão das do jogo). O NEI instala 319 desvios no init, e o core os removia um a um
  no unload: ~19 s a mais no boot e ~20 s no fechamento. As provas de overlay com o NEI terminavam em "não fechou em
  15 s; encerrado à força".
- **Contrato (core).** O `NativeEscapeHatch` ganhou dois campos opcionais, `beginBatch` e `commitBatch`. O
  `NativeProvider` abre um lote:
  - em volta do init de um provider com escape hatch liberado (`EscapeHatchAllowed()`): os desvios do init ativam
    juntos quando ele retorna. Init que falha ou lança remove os seus ainda dentro do lote, e nenhum chega a
    rodar. Commit que falha derruba a carga, e o unload remove o que tiver sido aplicado;
  - em volta da remoção no destrutor: um lote só, aplicado antes do `onProviderUnload` e do `shutdown`.
  Fora desses pontos, `install_patch`/`remove_patch` valem no retorno, como antes. Host sem os campos mantém o
  comportamento antigo.
- **Host OoT.** Dentro do lote, `install` usa `MH_QueueEnableHook`; `remove` de desvio ativo usa
  `MH_QueueDisableHook` e adia o `MH_RemoveHook` para o commit; `remove` de desvio que ainda espera ligar sai na
  hora (sem congelar). O commit faz um `MH_ApplyQueued` e registra `lote com N desvios ligados e M removidos em X ms`.
- **RFC 0023 emendada** (seção "Lote de ativação"): o init não pode depender de um desvio próprio já ativo. Em
  troca, some a janela em que o desvio rodava antes de o mod guardar o `original` (o escape demo tinha essa corrida).
- **Carona do layout novo.** O `NativeProvider.h` entra no glob do layout id, então a rodada troca o id
  (`0d86df42` → `07cb0af3`). Foram junto as duas sobras que esperavam o próximo id:
  - `oot_scenes.h`: `LINKSPAN_OOT_SCENES_MAX_ENTRANCE_INDEX` 32764 → 32756 (0x7FF4), como o registro já exigia
    desde a UNBOUND-017; o `static_assert` agora amarra o `.cpp` ao header;
  - `oot_resources.h`: o transcodificador roda na thread que carrega o recurso (pool do resource manager), não na
    thread do jogo.

- **Rodada 2 (2026-09-27, revisão do Codex `rev010`/`rev-fix010`).** A primeira rodada de correções só retinha a
  DLL: o `ModHost` já tinha extraído o mod, o `NativeProvider` era destruído e o unload devolvia sucesso, com
  runtime e contexto perdidos. Agora:
  - **Quarentena no core.** `NativeProvider::PrepareUnload()` é a metade que pode falhar: tira os hooks do dono e
    remove os desvios num lote, antes de o `ModHost` extrair o mod. Se o host não confirmar a remoção,
    `UnloadMod` devolve `HostFailure` e o mod fica carregado, com provider, runtime e contexto; outro `UnloadMod`
    repete a limpeza (sem o gate de dependentes). `shutdown` e `FreeLibrary` seguem no destrutor, depois do
    `lua_close`, então os finalizadores Lua do mod ainda chamam a DLL, como antes do lote.
  - `install_patch` e `register_hook` recusados desde o início da desmontagem, e os hooks do dono são tirados de
    novo depois do `shutdown`.
  - O rollback do `Load` (init que falha, ativação que falha, init que lança) usa a mesma quarentena, e o caminho
    da exceção desmonta fora dos handlers, com a exceção da DLL já destruída.
  - `~ModHost` vaza com diagnóstico o provider ainda em quarentena, em vez de destruí-lo.
  - **Desvios por dono no host.** `beginBatch(owner)` e `install(owner, …)`. Cada entrada guarda dono, `created`,
    `active`, `pendingEnable` e `pendingRemoval`. Remoção que falha fica associada ao dono, com a fila global da
    MinHook neutralizada (o `MH_ApplyQueued` de outro provider não a religa nem falha por ela). Antes de criar um
    desvio num alvo com remoção pendente, o host tenta concluí-la; o retry seguido de reload não deixa
    `MH_ERROR_ALREADY_CREATED`.
  - Na integração corrigi mais três pontos: a primeira versão do r2 fazia o unload completo antes do `lua_close`
    (o teste dos finalizadores pegou); o id de patch era reaproveitado depois de um rollback retido (colidia com
    a entrada antiga); e uma remoção que falhava depois de um `MH_ApplyQueued` bem-sucedido era re-enfileirada
    como ligada.

## Commits

- core `6731d60` (`extern/ship-lua`, branch `agent/COREEXT-010-escape-batch`) — lote no `NativeProvider`, teste,
  RFC e doc.
- host `5c4138b80` — `OotNativeEscapeWin`, sobras do layout, NEI 0.3.2 e ponteiro do submódulo.
- core `f29505c` (branch `agent/MENU-001-mod-menu`) — rodada 2: quarentena, `PrepareUnload`, recusas na
  desmontagem, fake stateful nos testes, RFC 0023.
- host `7d26819bb` — rodada 2 do `OotNativeEscapeWin` e ponteiro do submódulo em `f29505c`.

## Arquivos alterados

- core: `include/shiplua/native/NativeProvider.h`, `src/native/NativeProvider.cpp`,
  `tests/native/NativeProviderTests.cpp`, `tests/native/EscapeFixture.c`, `rfcs/0023-native-escape-hatch.md`,
  `docs/native-providers.md`
- host: `soh/soh/native/OotNativeEscapeWin.cpp`, `oot_scenes.h`, `OotNativeScenes.cpp`, `oot_resources.h`,
  `tools/package-nei-core.ps1`, `extern/ship-lua`
- rodada 2, core: `include/shiplua/host/ModHost.h`, `include/shiplua/native/NativeProvider.h`,
  `src/host/ModHost.cpp`, `src/native/NativeProvider.cpp`, `tests/native/{NativeProviderTests.cpp,EscapeFixture.c,
  ThrowFixture.cpp}`, `rfcs/0023-native-escape-hatch.md`; host: `soh/soh/native/OotNativeEscapeWin.cpp`,
  `extern/ship-lua`

## Validação executada

Windows 11, VS 2022 (MSVC 14.44), Release x64.

```bash
cmake -S extern/ship-lua -B build/core-tests -G "Visual Studio 17 2022" -A x64
cmake --build build/core-tests --config Release
ctest --test-dir build/core-tests -C Release --output-on-failure
cmake --build build/x64 --config Release --target soh soh_symbols oot_native_scenes_tests oot_native_registry_tests oot_native_engine_tests
cmake --build build/nei-core-native --config Release
```

## Resultado da validação

- **Rodada 2 (2026-09-27):**
  - **Core:** 60/61 no `ctest`; só o `api_contract_tests` antigo segue vermelho. O `native_provider_tests` passa
    com o fake stateful: falha persistente de remoção deixa o mod carregado em quarentena, sem `shutdown` e com a
    DLL carregada; outro provider no alvo `0x2000` carrega e descarrega sem herdar a pendência; o retry depois da
    recuperação completa o unload; recarregar o mesmo alvo não dá `ALREADY_CREATED`; `install_patch` e
    `register_hook` recusados no `shutdown`; finalizador Lua ainda chama a DLL no unload.
  - **Host:** faz parte da rodada final da CEL-004 (`75a420414`, layout `b459725c`, `soh.exe` `4EE072B2…`).
  - **Em jogo** (`cel-005` s1, NEI 0.3.4 + WW Style): boot com `lote com 319 desvios ligados e 0 removidos em
    65 ms`; ao fechar, o WW Style descarrega, depois `lote com 0 desvios ligados e 319 removidos em 65 ms` e o NEI
    descarrega; fechou pela janela em 0,5 s.
  - **Instalação limpa** (`release-b459725c`, 4 mods): 319 desvios em 64 ms no boot e 62 ms no fechamento.
  - **Não coberto por teste:** o duplo defeito "init que lança + remoção travada" no `Load`. O provider vaza de
    propósito e a DLL fica presa, o que quebraria o `remove_all` e o balanço init/shutdown do teste; o caminho foi
    alinhado aos outros dois rollbacks do `Load` por revisão.
- **Rodada 1:**
  - **Core:** 59/60 no `ctest`. O `native_provider_tests` cobre lote no init, patch depois do init fora do lote,
    unload num lote só antes do aviso ao host, init que falha removendo antes do commit, commit que falha derrubando
    a carga e nenhum lote sem permissão. O único vermelho, `api_contract_tests` (`ship.hooks.result`,
    `set_speed_multiplier`, `storage.shared.clear`), falha igual na base `NATIVE-001` (`f8749f3`).
  - **No build do host, o `native_provider_tests` não linka** (`mz_zip_writer_*` sem prefixo contra o miniz prefixado
    `shiplua_`); por isso o core roda num build próprio (`build/core-tests`). Pré-existente.
  - **Host:** `soh.exe` `B0E853D0…` (32 217 088 bytes), `soh.symbols` gerado, layout `07cb0af3…`, `soh.o2r`
    inalterado (`C0D94440`). `oot_native_scenes/registry/engine_tests` e `linkspan_nei_core_tests`: exit 0.
  - **Em jogo** (`OOT-teste-cenas-20260914`, evidência `build/oot-upstream-001/evidence/coreext-010`):
    - sessão `a`, escape demo: `lote com 1 desvios ligados … em 61 ms`; fechou pela janela em 0,5 s;
    - sessão `a2`, escape demo com o save aberto: `hud_calls` de 19 a 439 e `skipped` subindo. O desvio ligado pelo
      commit do lote roda. Fechou em 0,4 s;
    - sessão `b`, NEI 0.3.2: `lote com 319 desvios ligados e 0 removidos em 59 ms`, e 70 ms entre "escape hatch" e
      "loaded mod" (antes ~19 s). O status mostra fork ativo, kaleido 3/3 e host 314/314. Ao fechar: `lote com 0
      desvios ligados e 319 removidos em 67 ms`, fechou pela janela em 0,5 s (antes ~20 s).

## Pendências

- Revisão externa da rodada 2 integrada (os três pontos corrigidos na integração não passaram por nova revisão do
  Codex: a cota de 5 h ficou em 91% até 07:05 de 2026-09-27).
- A cópia de teste ficou com o host `4EE072B2`, WW Style e NEI 0.3.4 em `mods`; o host anterior está em
  `host-anteriores\soh-1c4b4e39.*`.

## Riscos

- Mod que dependa de um desvio próprio já ativo dentro do init passa a ver a função original até o init retornar.
  Nenhum mod do repositório faz isso (NEI e escape demo conferidos).
- `MH_ApplyQueued` aplica toda a fila da MinHook. Hoje só o escape hatch usa MinHook no `soh.exe` (conferido por
  grep); um segundo usuário da biblioteca no processo teria a fila dele aplicada junto.
- Quarentena no fim do processo é vazamento intencional (provider, runtime e DLL ficam vivos até o processo
  acabar), com diagnóstico no log.
- Enquanto um mod está em quarentena, o runtime Lua dele continua vivo e suas funções nativas seguem chamáveis
  (a DLL está carregada); hooks e novos desvios são recusados.

## Próxima ação recomendada

Review da COREEXT-010 (rodadas 1 e 2) e push quando autorizado. Com a cota do Codex de volta, uma passada estática
sobre o `f29505c` e o `7d26819bb`.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 5c4138b80 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: 6731d60 f29505c.
- Host: 5c4138b80 7d26819bb d1b76ff56 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
