# Handoff — OOT-UNBOUND-002B

## Estado

done — factory JSON versionada publicada por um coremod externo, consumida
por outro ZIP e comprovada no Shipwright 9.2.3 sem recompilar o host. Fechado em 2026-09-12 com
commits locais, sem push.

## Resultado

- `linkspan.unbound.framework` consulta `linkspan.oot.resources` v2 e publica
  `linkspan.unbound.json_factory` v1 pela Core Extension ABI 1.1.
- O schema `linkspan.unbound.actor-patch/v1` associa o tipo `actor_patch` às
  regras de objetos recursivos e substituição integral de arrays/escalares.
- O coremod valida `$schema` em todas as camadas, mantém resultados por handles
  opacos e expõe JSON, número de camadas, hash e liberação explícita.
- Um consumidor independente monta duas fixtures, chama o serviço publicado,
  lê o resultado e desmonta tudo antes de retornar.
- Nenhum código específico do Unbound foi adicionado ou recompilado no host.

## Commits

- Host `OOT-NATIVE-001`, no fechamento: `dad1b2914` — `unbound-core`, script de
  pacote e seção de `native-mods.md`.
- Claim, handoff e UPDATE do plano: commit de coordenação no `link-span` no fechamento.
- O host testado permaneceu no commit local `53bef1f16`, com `resources` v2 já
  integrado pela tarefa anterior.

## Arquivos alterados

### OOT-NATIVE-001

- `soh/native-sdk/unbound-core/CMakeLists.txt`
- `soh/native-sdk/unbound-core/include/linkspan/unbound/json_factory.h`
- `soh/native-sdk/unbound-core/json_merge.h`
- `soh/native-sdk/unbound-core/json_merge.cpp`
- `soh/native-sdk/unbound-core/unbound_core.cpp`
- `soh/native-sdk/unbound-core/unbound_consumer.cpp`
- `soh/native-sdk/unbound-core/core-main.lua`
- `soh/native-sdk/unbound-core/consumer-main.lua`
- `soh/native-sdk/unbound-core/tests.cpp`
- `soh/native-sdk/unbound-core/README.md`
- `tools/package-unbound-factory-demo.ps1`
- `docs/native-mods.md`

### link-span

- `coordination/claims/OOT-UNBOUND-002B.md`
- `coordination/handoffs/OOT-UNBOUND-002B.md`
- `docs/agents/Plans.md`

## Validação executada

- Projeto externo configurado com o `OotNativeSdk.cmake` exportado pelo host.
- Core e consumer compilados como DLLs sem link com `soh.exe` ou bibliotecas
  internas do jogo.
- `ctest --test-dir build/unbound-factory-native -C Release`: 1/1 aprovado.
- Testes cobrem merge recursivo, substituição de array, schema divergente e
  JSON inválido.
- `shiplua_manifest_validator`: coremod e consumer válidos.
- Bundle inspecionado em memória: seis entradas, checksums internos aprovados e
  nenhum `soh.exe`, `soh.o2r` ou `oot.o2r` incluído.
- Playtest final isolado `20260912-194800`: dois mods carregados, processo
  responsivo, marcador de merge presente e `valid: true` no gate estrito.

## Resultado da validação

- Marcador final:
  `unbound-002b-factory: schema=linkspan.unbound.actor-patch/v1; layers=2; hash=412c9d58a4e6de21; json={"$schema":"linkspan.unbound.actor-patch/v1","actor":{"name":"guard","stats":{"health":8,"speed":1},"drops":["heart"]},"enabled":false}; cleanup=ok`.
- O objeto `stats` preservou `speed=1` da base e recebeu `health=8` do
  override; o array `drops` foi substituído integralmente.
- `soh.exe` manteve SHA-256
  `5E96766F8B88082E917D3EA478F07B65155B1EBC4C09DB384739BD9BA047A65C`
  antes e depois.
- `oot.o2r`, `soh.o2r`, coremod, consumer e fixtures permaneceram inalterados.

## Artefatos

- Bundle instalável:
  `OOT-NATIVE-001/build/unbound-factory-packages/LinkSpan-OOT-UNBOUND-002B-Mods-0.1.0.zip`
  - SHA-256 `F1F60F873A1FA939E4C6278E4F898FAA0E5FAD0F6E93A29FBBD36872B26C2F69`
- Coremod:
  `OOT-NATIVE-001/build/unbound-factory-packages/LinkSpan-Unbound-Framework-0.1.0.zip`
  - SHA-256 `1E17FD56FD3B92B6D4A3643CB0C3699951B0554013E26BC6F1C2CE6E86F2483A`
- Consumer:
  `OOT-NATIVE-001/build/unbound-factory-packages/LinkSpan-Unbound-Factory-Demo-0.1.0.zip`
  - SHA-256 `A3C08FDD1D2AA4CD3E388520DCC86AC312C68869D8E1CD979677089FBB8A34CA`
- Evidência final:
  `OOT-NATIVE-001/build/runtime-evidence/unbound-002b-final-20260912-1948/`.

## Pendências

- O resultado ainda não é materializado como cena/sala pelo `ResourceLoader`;
  esta integração vem após os layouts ampliados.
- `null`, `$replace`, `$order` e remoção de propriedades pertencem ao
  `UNBOUND-004`.
- Próximo recorte: `UNBOUND-003`, com tipos, containers, ownership e layouts
  ampliados exigidos pelas cenas do Unbound.

## Riscos

- Handles e ponteiros retornados pelo serviço pertencem ao coremod e não podem
  sobreviver ao seu shutdown.
- O serviço é restrito à thread do jogo e limita cada documento/resultante a
  64 KiB nesta versão de prova.
- Como qualquer provider nativo, um coremod defeituoso pode derrubar o processo;
  a ABI C disciplina ownership, mas não cria sandbox.

## Próxima ação recomendada

Abrir `OOT-UNBOUND-003*` para inventariar e portar primeiro os tipos ampliados
que alteram ABI/layout do host, com static assertions e fixture sintética antes
de ligar a factory JSON às cenas reais.
