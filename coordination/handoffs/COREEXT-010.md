# Handoff — COREEXT-010

## Estado

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

## Commits

- core `6731d60` (`extern/ship-lua`, branch `agent/COREEXT-010-escape-batch`) — lote no `NativeProvider`, teste,
  RFC e doc.
- host `5c4138b80` — `OotNativeEscapeWin`, sobras do layout, NEI 0.3.2 e ponteiro do submódulo.

## Arquivos alterados

- core: `include/shiplua/native/NativeProvider.h`, `src/native/NativeProvider.cpp`,
  `tests/native/NativeProviderTests.cpp`, `tests/native/EscapeFixture.c`, `rfcs/0023-native-escape-hatch.md`,
  `docs/native-providers.md`
- host: `soh/soh/native/OotNativeEscapeWin.cpp`, `oot_scenes.h`, `OotNativeScenes.cpp`, `oot_resources.h`,
  `tools/package-nei-core.ps1`, `extern/ship-lua`

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

- O layout `07cb0af3` é intermediário: a mesma rodada de host ainda recebe W-1 e os pontos de extensão da CEL-004.
  Os mods nativos publicados (Unbound, DMR, Mic, demos) são reempacotados uma vez, no layout final da rodada, junto
  com o overlay e a prova limpa.
- A cópia de teste ficou com o host `B0E853D0` e só o NEI 0.3.2 em `mods`. O host anterior está em
  `host-anteriores\soh-9ddbc766.*`.

## Riscos

- Mod que dependa de um desvio próprio já ativo dentro do init passa a ver a função original até o init retornar.
  Nenhum mod do repositório faz isso (NEI e escape demo conferidos).
- `MH_ApplyQueued` aplica toda a fila da MinHook. Hoje só o escape hatch usa MinHook no `soh.exe` (conferido por
  grep); um segundo usuário da biblioteca no processo teria a fila dele aplicada junto.
- A revisão externa (Codex) não rodou: a cota de 5 h acabou às 15:20 e o Kimi está sem acesso na assinatura. A
  revisão foi minha, contra o `hook.c` da MinHook usada.

## Próxima ação recomendada

Seguir a rodada de host com W-1 e os pontos de extensão da CEL-004; depois reempacotar os mods para o layout
final e rodar a prova limpa do overlay, que deve fechar pela janela.
