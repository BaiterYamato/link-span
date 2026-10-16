# Handoff — OOT-MOVE-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

in_progress

## Resultado

- Mod independente `Dynamic-Movement-Remake-0.2.3.zip` criado com manifesto,
  Lua e provider nativo.
- X físico executa o pulo dedicado. A mantém a ação contextual, produz o
  rolamento normal em movimento e entra em sprint quando continua pressionado.
- Y alimenta a ação normal de espada, B a ação normal de cancelar/guardar e o
  analógico direito ativa a câmera livre enquanto o mod está carregado.
- O pulo escolhe a animação nativa parada/correndo e reproduz a voz nativa de
  auto-jump junto com o impulso.
- Quatro segundos sem movimento do analógico direito restauram a câmera automática;
  uma nova entrada reativa a câmera livre e reinicia o cronômetro.
- O host mantém `linkspan.oot.engine` v1 e publica o serviço genérico separado
  `linkspan.oot.movement` v1, com botões físicos, mappings transitórios e settings
  inteiros, sem exigir recompilação do loader para ajustar o mod.
- Overlay instalável da release Shipwright 9.2.3 gerado sem incorporar o mod.

## Commits

- Nenhum; commit e PR não foram autorizados pelo usuário.

## Arquivos alterados

- `OOT-NATIVE-001/libultraship/include/ship/controller/controldevice/controller/ControllerButton.h`
- `OOT-NATIVE-001/libultraship/src/ship/controller/controldevice/controller/ControllerButton.cpp`
- `OOT-NATIVE-001/soh/soh/native/oot_engine.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.h`
- `OOT-NATIVE-001/soh/soh/native/OotNativeEngine.cpp`
- `OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp`
- `OOT-NATIVE-001/soh/native-sdk/ExportSdk.cmake`
- `OOT-NATIVE-001/soh/native-sdk/example/CMakeLists.txt`
- `OOT-NATIVE-001/soh/native-sdk/example/manifest.toml`
- `OOT-NATIVE-001/soh/native-sdk/example/main.lua`
- `OOT-NATIVE-001/soh/native-sdk/example/provider.cpp`
- `OOT-NATIVE-001/soh/native-sdk/tests/EngineTests.cpp`
- `OOT-NATIVE-001/soh/native-sdk/tests/test_independent.py`
- `OOT-NATIVE-001/docs/native-mods.md`
- `NATIVE-001/rfcs/0014-dynamic-movement-remake.md`

## Validação executada

```powershell
cmake --build build/x64 --config Release --target oot_native_engine_tests -- /m
build/x64/Release/oot_native_engine_tests.exe
python soh/native-sdk/tests/test_independent.py ...
cmake --build build/x64 --config Release --target soh -- /m
pwsh tools/package-linkspan-overlay.ps1 ...
# Playtest isolado sobre a release oficial Shipwright 9.2.3.
```

## Resultado da validação

- Build Release do host concluído no Windows/MSVC.
- Teste do bridge e da máquina de estados passou.
- Providers independentes com impulsos 7 e 10 passaram; SHA-256 do host ficou
  `B3F716ABC4B6CBE70192F60D99801441154C24E272A09B1BAE7045426ECB9098`.
- Teste automatizado cobre X pulo, A rolamento/sprint, câmera livre e restauração
  de settings/mappings. A instalação real 9.2.3 carregou o ZIP 0.2.0 e encerrou
  sem alterar os mappings persistidos.
- O teste do serviço confirmou animação não nula, `NA_SE_VO_LI_AUTO_JUMP`,
  `camera-auto` e reativação `camera-free`. A instalação real carregou o mod
  0.2.3, confirmou o timeout configurado de 4,0 segundos e registrou a transição
  `camera-auto` sem erro.
- ZIP do mod: SHA-256
  `735C05EBE481FA7E8832AD326E6720B13C2BAC94FA78D9AA3417E582B19EE526`.
- Overlay: SHA-256
  `1EC140F0A235320717A2C488AA92237F6BB0773B67AA8C3911992D0943AF245D`.

## Pendências

- Testar X/A/Y/B e o analógico direito em controle Switch Pro físico; nenhum
  gamepad estava conectado no playtest automatizado.
- Implementar as próximas fatias do perfil: ZR/item, R/Quick Swap, L/escudo,
  Ocarina/Navi, +/- e seletores de equipamento/botas.
- Commit e PR dependem de autorização explícita.

## Riscos

- Provider nativo é acoplado ao fingerprint do SDK/host 9.2.3 e deve ser
  recompilado quando os layouts internos mudarem.
- Os demais controles já estão declarados no perfil do mod, mas ainda não têm
  comportamento ativo nesta fatia.

## Próxima ação recomendada

- Implementar a camada de eixos/gatilhos para ZL/ZR e a ação de item equipada.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Status hoje: claimed. A fatia X/A/Y/B do DMR está no host 3b28204fa (RC2+); as fatias seguintes viraram OOT-MOVE-002..007; o mantenedor jogou o DMR no RC5 (10/10).
- Core: commits na main que citam a tarefa: 6b8393b.
- Host: 3b28204fa (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: teste com Switch Pro físico; o DMR foi jogado pelo mantenedor no RC5.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
