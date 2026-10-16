# Handoff — OOT-CORE-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — `linkspan.oot.resources` v1 publicado, testado, documentado e provado
em jogo no Shipwright 9.2.3.

## Resultado

- ABI C com handles opacos para leitura, enumeração, invalidação e montagem do VFS.
- Provider externo usa o serviço sem vínculo novo com `soh.exe`.
- Playtest real montou e desmontou um ZIP e confirmou a remoção do marcador.

## Commits

- `3b28204fa` — integração do serviço no host OoT.
- `8a1ca5244` — provider externo, documentação e prova em jogo.
- `894ba40` — Core Extension ABI 1.1 no SDK.
- Sem push.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/native/oot_resources.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp`
- `OOT-NATIVE-001/docs/native-mods.md`

## Validação executada

```powershell
build/x64/Release/oot_native_engine_tests.exe
git diff --check
# sessão core-001-20260912-1155
```

## Resultado da validação

- Teste nativo: exit 0.
- Playtest: `valid: true`, zero exceções e hashes antes/depois inalterados.
- Marcador: `mount=ok`, `marker=match`, `unmount=ok`, `gone=1`.
- Evidência: `OOT-NATIVE-001/build/runtime-evidence/core-001-20260912-1155/`.

## Pendências

- Exercitar o contrato pelos ports reais de Unbound e NEI.
- A serialização interna do ArchiveManager permanece melhoria futura do libultraship.

## Riscos

- Montagem e desmontagem devem ocorrer nas janelas de boot/shutdown documentadas.

## Próxima ação recomendada

Abrir `OOT-CORE-002` para registros dinâmicos namespaced.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 3b28204fa e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: 894ba40; a branch agent/NATIVE-001-provider-runtime é ancestral da main.
- Host: 3b28204fa 8a1ca5244 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
