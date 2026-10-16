# Handoff — OOT-CORE-002

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — registro dinâmico genérico v1 implementado e provado no Shipwright
9.2.3 com provider externo.

## Resultado

- Novo serviço C `linkspan.oot.registry` v1 publicado pelo host OoT.
- Providers criam espaços namespaced com intervalo e stride, registram payloads
  copiados pelo host e consultam entradas por nome, ID ou handle.
- ID automático escolhe o menor valor livre; enumeração é ordenada por ID.
- Limites: 64 espaços, 65.536 entradas, nomes de 255 bytes e payload de 64 KiB.
- `destroy_space` remove entradas em cascata; o exemplo prova cleanup no unload.
- Dynamic Movement Remake 0.2.4 cria as entradas `jump` e `sprint` dentro do
  próprio mod. Catálogos novos não exigem nova função dedicada no loader.
- Contrato formalizado na RFC 0016 e documentado no guia do SDK.

## Commits

- Nenhum commit criado nesta tarefa; o usuário não autorizou commit ou push.
- Base local do host: `8a1ca5244`.
- Base local do SDK: `b15d67c` sobre `894ba40`.

## Arquivos alterados

### NATIVE-001

- `rfcs/0016-oot-dynamic-registry.md`
- `coordination/claims/OOT-CORE-001.md` (reconciliação anterior preservada)
- `coordination/handoffs/OOT-CORE-001.md` (reconciliação anterior preservada)
- `coordination/claims/OOT-CORE-002.md`
- `coordination/handoffs/OOT-CORE-002.md`

### OOT-NATIVE-001

- `soh/soh/native/oot_registry.h`
- `soh/soh/native/OotNativeRegistry.h`
- `soh/soh/native/OotNativeRegistry.cpp`
- `soh/soh/native/OotNativeEngine.cpp`
- `soh/soh/ShipLuaBootstrap.cpp`
- `soh/native-sdk/ExportSdk.cmake`
- `soh/native-sdk/tests/RegistryTests.cpp`
- `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/example/provider.cpp`
- `soh/native-sdk/example/main.lua`
- `soh/native-sdk/example/manifest.toml`
- `docs/native-mods.md`
- `docs/install-linkspan-overlay.md`

## Validação executada

- CMake Visual Studio x64 reconfigurado com `LINKSPAN_SDK_SOURCE_DIR=NATIVE-001`.
- `oot_native_registry_tests.exe`: exit 0.
- `oot_native_engine_tests.exe`: exit 0.
- `test_independent.py`: duas compilações do provider (pulo 7 e 10), ambas
  carregadas; SHA-256 do host permaneceu
  `98D8C558DD2C6E2131896D969471AF57122F5368A38064D751D2EB8A485949CC`.
- `shiplua_manifest_validator.exe`: Dynamic Movement Remake 0.2.4 válido.
- Headers/implementação/teste novos passam em `clang-format --dry-run --Werror`.
- `git diff --check`: aprovado.
- Overlay inspecionado sem extração: membros esperados, sem `oot.o2r`, e todos
  os hashes internos conferem.
- Playtest isolado `core-002-20260912-1445`: `valid: true`, inicialização e
  encerramento normais, nenhuma exceção, arquivos rastreados inalterados.
  Marcador:
  `core-002-registry: space=ok; entries=2; first=128; jump=example/dynamic_movement/jump:action=jump; id=128`.

O CTest desta configuração informa zero testes porque o projeto raiz não chama
`enable_testing`; por isso os dois executáveis foram executados diretamente.

## Resultado da validação

- Todos os testes executáveis e o validador retornaram código 0.
- O provider foi recompilado duas vezes sem modificar o hash do host.
- O playtest real confirmou o serviço no boot e no `game.ready`, sem crash.
- A verificação estrita da sessão confirmou hashes antes/depois idênticos.
- Nenhuma ROM, save ou configuração pessoal entrou nos artefatos.
- O validador de lifecycle confirmou todos os campos de claim/handoff; o gate
  permanece não estrito porque os claims predecessores `OOT-NATIVE-001`,
  `OOT-COREEXT-001` e `OOT-CORE-001` ainda estão formalmente em `review` e
  reivindicam arquivos compartilhados desta pilha.

## Artefatos

- `OOT-NATIVE-001/build/distribution/Dynamic-Movement-Remake-0.2.4.zip`
  - SHA-256 `4229303D0FFC8122A03D4D210E107E227E270FB9928D59DE3CCE163047F8F0F7`
- `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-OOT-CORE-002-RegistryV1-Win64-overlay.zip`
  - SHA-256 `1233D3923F81838452C4037102F5169DA5FBD88987EF2E43D4A225ED32798324`
- Evidências: `OOT-NATIVE-001/build/runtime-evidence/core-002-20260912-1445/`

## Riscos

- O registry v1 armazena dados e identidade, mas ainda não conecta uma entrada
  ao SceneDB, às entrances ou ao inventário real.
- Providers nativos continuam sendo código confiável no processo; a ABI evita
  ownership C++ cruzado, mas não oferece sandbox contra corrupção de memória.
- O criador deve chamar `destroy_space` no shutdown. O exemplo e o loader
  normal exercitam esse caminho; encerramento abrupto do processo descarta tudo.

## Pendências

- Executar `UNBOUND-002` e `UNBOUND-003`: factories/JSON em camadas, layouts
  ampliados e indirection de cena/entrance no overlay.
- Implementar então o adapter Unbound que consome espaços de cena/entrance e
  aplica o payload ao SceneDB do jogo.
- Em seguida abrir `OOT-CORE-003` para actor/item/inventory extension points e
  entregar o item sintético da NEI.
- Integrar e publicar somente após revisão; nenhum push foi feito.

## Próxima ação recomendada

Portar o substrato `UNBOUND-002/003` para o overlay e, sobre ele, criar a
primeira fatia vertical: um `.shipmod` registra uma cena sintética por nome e o
adapter resolve uma entrada real sem tabela compilada por mod.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 8a1ca5244 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: na main: 894ba40 b15d67c; a branch agent/NATIVE-001-provider-runtime é ancestral da main.
- Host: 8a1ca5244 a068ec711 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
