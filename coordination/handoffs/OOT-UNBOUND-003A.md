# Handoff — OOT-UNBOUND-003A

## Estado

review

## Resultado

- Ampliadas as contagens do runtime para atores vivos, atores de setup, salas, atores de transição, banco de objetos e meshes.
- `Actor::objBankIndex` agora alcança todos os 1.024 slots do banco e o importador OTR v0 preserva contagens `u8` entre 128 e 255.
- Adicionados clamps e guardas de limite antes de escrever em arrays; recursos excessivos falham com diagnóstico em vez de wrap ou corrupção.
- O savestate deixou de usar o array fixo de 256 atores de transição.
- Assertions de compilação impedem regressão dos tipos ampliados e divergência entre os layouts de mesh do recurso e do runtime.
- O coremod Unbound e mods nativos independentes continuam compilando fora do host com o SDK exportado.

## Commits

- `5f39d7e82` em OOT-NATIVE-001: commit local feito pelo Claude em 2026-09-13, a
  pedido do usuário com o Codex parado, depois de recompilar o `soh` e rodar
  `oot_native_engine_tests`, `oot_native_registry_tests`,
  `oot_actor_provider_tests`, `test_independent.py`, o ctest do `unbound-core` e
  `git diff --check`, todos com exit 0. Sem push.

## Arquivos alterados

- `OOT-NATIVE-001/soh/include/z64.h`
- `OOT-NATIVE-001/soh/include/z64actor.h`
- `OOT-NATIVE-001/soh/include/z64object.h`
- `OOT-NATIVE-001/soh/soh/Enhancements/savestates.cpp`
- `OOT-NATIVE-001/soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp`
- `OOT-NATIVE-001/soh/soh/resource/logging/SceneCommandLoggers.cpp`
- `OOT-NATIVE-001/soh/soh/resource/type/scenecommand/SetMesh.h`
- `OOT-NATIVE-001/soh/soh/unbound/UnboundLayoutChecks.cpp`
- `OOT-NATIVE-001/soh/soh/z_scene_otr.cpp`
- `OOT-NATIVE-001/soh/src/code/z_actor.c`
- `OOT-NATIVE-001/soh/src/code/z_room.c`
- `OOT-NATIVE-001/soh/src/code/z_scene.c`
- `OOT-NATIVE-001/docs/architecture/unbound-count-layouts.md`

## Validação executada

```text
MSBuild soh.vcxproj Release x64, WPO/LTCG desativados na linha de comando: exit 0
Build incremental nas mesmas condições: exit 0
oot_actor_provider_tests.exe: OotActorProviderTests: OK
oot_native_engine_tests.exe: exit 0
oot_native_registry_tests.exe: exit 0
linkspan_unbound_json_tests: 1/1 passou
test_independent.py: dois providers executados; host SHA-256 inalterado
git diff --check: exit 0
```

## Resultado da validação

- `soh.exe` gerado com SHA-256 `5503b5106e92728d3d504cda1b204e663987d74a87b39b3d7992c0fc878c336b`.
- A prova independente recompilou e executou dois providers externos sem alterar esse hash.
- O build Release padrão com LTCG esgotou o heap do linker de 32 bits (`C1002`); a validação repetível usou linker de 64 bits e desativou WPO/LTCG somente por propriedades da linha de comando.
- O diretório raiz de build não registra CTests; os targets nativos foram compilados e executados diretamente, e o projeto isolado do Unbound foi validado pelo próprio CTest.

## Pendências

- Criar fixtures de cena que exercitem em runtime os patamares acima dos limites vanilla e seus overflows.
- Completar os demais recortes de `UNBOUND-003`: colisão ampla, scene/entrance indirection, containers dinâmicos e ownership ao destruir `PlayState`.
- Ampliar os índices de objeto privados ainda mantidos em `s8` por atores vanilla específicos.
- Executar playtest real com archive Unbound; este recorte foi validado sem ROM.

## Riscos

- O banco possui 1.024 slots estruturais, mas o pool em bytes dos objetos ainda é limitado pelo alocador da cena.
- Alguns atores vanilla guardam cópias privadas do índice de objeto em `s8`; os novos guardas evitam acesso inválido, mas esses atores ainda não usam slots acima de 127.
- Alterações em `soh/include` mudam intencionalmente o fingerprint da ABI; os coremods nativos precisam ser recompilados uma vez para este novo overlay.

## Próxima ação recomendada

- Abrir `OOT-UNBOUND-003B` para índices/containers de colisão e fixtures sintéticas de fronteira, preservando este substrato como a alteração única do host.
