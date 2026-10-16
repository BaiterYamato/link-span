# Handoff — OOT-CORE-006

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Serviço `linkspan.oot.save` v1 (`soh/soh/native/oot_save.h`, nono serviço da política, no fim da lista):
  - `open_namespace` com nome namespaced; o prefixo `linkspan.` é do host;
  - `read`, `write` (JSON de até 1 MiB), `erase`, `get_stored_version` e `set_required`;
  - `begin`/`commit`/`rollback` por namespace;
  - `get_slot`.
- Store puro em `OotNativeSave.cpp`, com mutex, porque a gravação do SoH roda no thread pool.
- Integração em `OotNativeSaveGame.cpp`:
  - seção `linkspan` v1 registrada no `SaveManager` logo depois de ele ser criado (`OTRGlobals.cpp`);
  - `InitFile` esvazia blocos e flags;
  - `OnLoadFile` restaura flags, avisa dependência obrigatória ausente e dispara `oot.save.loaded`;
  - `SaveSection` chama `OotBeforeSave` na thread do jogo, que dispara `oot.save.saving` e grava as flags;
  - `CopyZeldaFile` dispara `oot.save.copied` e `OnDeleteFile` dispara `oot.save.deleted`.
- Eventos são pontos de hook observe da RFC 0021, e não callbacks no serviço, para sair junto com o mod no unload.
- Flags das cenas de mod (RFC 0020) persistidas no bloco `linkspan.scenes`, pelo nome da cena.
- Blocos de namespaces não abertos na sessão voltam ao arquivo; entradas malformadas são descartadas.
- Demo `soh/native-sdk/save-demo`, empacotado por `tools/package-save-demo.ps1`, e RFC 0022.

## Commits

- `OOT-UPSTREAM-78DC-001`: `a19d83b1b`.
- `NATIVE-001`: `106a536` (RFC 0022).
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_save.h`, `OotNativeSave.h`, `OotNativeSave.cpp`, `OotNativeSaveGame.cpp`
- `soh/soh/native/oot_hooks.h`, `OotNativeHooks.h`, `OotNativeHooks.cpp`, `OotNativeScenes.h`, `OotNativeScenes.cpp`,
  `OotNativeEngine.cpp`
- `soh/soh/ShipLuaBootstrap.cpp`, `soh/soh/OTRGlobals.cpp`, `soh/soh/SaveManager.cpp`
- `soh/native-sdk/ExportSdk.cmake`, `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/save-demo/**`
- `tools/package-save-demo.ps1`, `docs/native-mods.md`

### NATIVE-001

- `rfcs/0022-oot-save-namespaces.md`

## Validação executada

```text
PowerShell: cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py ... --host x64/Release/soh.exe
build save-demo-native, hooks-demo-native, unbound-scene-native; package-save-demo/hooks-demo/unbound-scene-demo
Prova em jogo, 2 sessões: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência build/oot-upstream-001/evidence/core006-save-ingame
```

## Resultado da validação

- **Host:** `soh.exe` SHA-256 `809B82A0288F262518C7E3055877C5B51AC5C4FEB8BA695544E2D628773F66E7`, layout
  `abff43320d98e901ed637bb4accedd500e9523968ca7f4d2055b72d4461212f2`.
- **Testes:** engine tests, scenes tests e prova independente (hash inalterado) com exit 0. Os engine tests cobrem:
  - validação de nomes e versão divergente;
  - read/write, capacidade curta e JSON inválido;
  - transação: o save grava o estado de antes do begin, e rollback/commit funcionam;
  - required ausente;
  - bloco desconhecido preservado e entrada malformada descartada;
  - bloco do host e recusa fora da thread do jogo.
- **Pacotes** (layout `abff4332`): Save Demo `15D129FD…`, Hooks Demo `A5530DD3…`, Unbound Framework `3C383FBF…`
  (mesma DLL) e Scene Demo `9EDA71FE…`.
- **Sessão 1:**
  - log `save-demo: 1 loaded slot=0 read=vazio stored_version=0 loads=1`;
  - viagem para a cena 128;
  - save pelo pause (Start, B, A), com `save-demo: 2 saving slot=0 get_slot=0`;
  - `Save/file1.sav` passa a ter `sections.linkspan`, com `linkspan-demo.save` (`loads: 1`, `required: true`,
    `version: 1`) e `linkspan.scenes` (`linkspan_demo/field_copy`).
- **Sessão 2:** `save-demo: 1 loaded slot=0 read=ok stored_version=1 loads=2`, lido do disco.
- `file1.pre.sav` e `file1.after-session1.sav` guardados na evidência.

## Pendências

- Save feito dentro de uma cena de mod reabre em Kakariko (cena 0x52, entrada 0xDB), pela regra vanilla de respawn
  para cena sem mapeamento. Falta definir no registro de cenas uma entrada de retorno por cena.
- As flags gravadas na prova eram todas zero; a conversão `SavedSceneFlags` ↔ JSON só foi exercitada com zeros em
  jogo.
- Dependência obrigatória ausente só gera log; recusar a carga precisa de UI na seleção de arquivos.
- Os eventos `copied` e `deleted` não foram exercitados em jogo.
- DMR 0.2.9 e Mic 0.1.4 precisam de rebuild para `abff4332`.

## Riscos

- `oot.save.saving` roda só no save da seção base; saves parciais de outras seções não disparam o evento.
- Um save do SoH logo depois do load (seção não base) grava os blocos já importados; se um mod escrever durante o
  `loaded`, o dado pode ir ao disco antes do save manual. Isso é esperado, mas vale documentar para autores.
- O layout id muda de novo (`oot_save.h` e `oot_hooks.h`).

## Próxima ação recomendada

- COREEXT-008: escape hatch (símbolos, detours com dono e fingerprint do executável).
- Em seguida, OOT-CORE-003 (item, inventário e tipos de ator).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host a19d83b1b na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: 106a536.
- Host: a19d83b1b (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
