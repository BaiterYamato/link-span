# Handoff — OOT-UNBOUND-002A

## Estado

done — leitura de recursos em todas as camadas montadas publicada como
`linkspan.oot.resources` v2, com prova unitária, prova independente e playtest
isolado no Shipwright 9.2.3. Fechado em 2026-09-12 com commits locais, sem push.

## Resultado

- `ArchiveManager::LoadFileFromAllLayers` no libultraship enumera todas as
  cópias montadas de um caminho virtual, da menor para a maior prioridade.
- ABI C v2 em `oot_resources.h`: prefixo binário compatível com a v1, mais
  `read_file_layers` com callback síncrono por camada (bytes, archive de
  origem, versão de jogo, hash FNV-1a 64, tamanho, índice/total).
- A v1 permanece publicada; hosts e mods antigos não quebram.
- A regra de merge fica no provider; o host só entrega ordem estável. Nenhuma
  lógica específica de Unbound entrou no executável.
- Provider de exemplo exige a v2 e expõe `layer_probe` e
  `layer_runtime_probe`; a prova de runtime monta duas camadas com o mesmo JSON,
  compõe um hash determinístico e desmonta ambas antes de retornar.
- Contrato formalizado na RFC 0017 e documentado em `docs/native-mods.md`.

## Commits

- Host `OOT-NATIVE-001` (branch `agent/OOT-NATIVE-001-native-engine`):
  - `a068ec7` — serviço `linkspan.oot.registry` v1 (OOT-CORE-002), commitado
    nesta sessão com staging intermediário compilado e testado;
  - `53bef1f16` — resources v2 / camadas (este handoff).
- Submódulo `libultraship` (branch `agent/OOT-MOVE-001-transient-button-mappings`):
  - `db184921` — `LoadFileFromAllLayers`.
- SDK `NATIVE-001` (branch `agent/NATIVE-001-provider-runtime`):
  - `68de832` — coordenação OOT-CORE-001/002 e RFC 0016;
  - `59dd141` — RFC 0017, no fechamento.
- Sobras do host commitadas no fechamento: `221339a39` — `layer_runtime_probe`,
  exemplo 0.2.5, documentação do overlay e `native-mods.md`.
- Claim, handoff e UPDATE do plano: commit de coordenação no `link-span` no fechamento.
- Núcleo `link-span` (branch `release/v0.2.0-alpha.1`):
  - `3d2d802` — coordenação OOT-CORE-001, update do plano e claim OOT-MOVE-002.
- Push não autorizado; tudo apenas local.

## Arquivos alterados

### NATIVE-001

- `rfcs/0017-oot-resource-layers.md`

### link-span

- `coordination/claims/OOT-UNBOUND-002A.md`
- `coordination/handoffs/OOT-UNBOUND-002A.md`
- `docs/agents/Plans.md`

### OOT-NATIVE-001

- `libultraship` (ponteiro → `db184921`)
- `soh/soh/native/oot_resources.h`
- `soh/soh/native/OotNativeEngine.h`
- `soh/soh/native/OotNativeEngine.cpp`
- `soh/soh/ShipLuaBootstrap.cpp`
- `soh/native-sdk/tests/EngineTests.cpp`
- `soh/native-sdk/example/provider.cpp`
- `soh/native-sdk/example/main.lua`
- `soh/native-sdk/example/manifest.toml`
- `docs/native-mods.md`
- `docs/install-linkspan-overlay.md`

## Validação executada

- Relink do `soh.exe` com MSBuild x64 + `LinkToolPath` Hostx64 (o link anterior
  morreu com o executável truncado em 0 bytes às 15:07; reconstruído às 18:00).
- `oot_native_registry_tests.exe`: exit 0 (antes e depois do staging do commit 1).
- `oot_native_engine_tests.exe`: exit 0 — cobre ordem das camadas, bytes,
  origem, hash, interrupção do callback e recusa fora da thread do jogo.
- `test_independent.py`: provider recompilado fora da árvore e carregado contra
  o host final; SHA-256 do `soh.exe` permaneceu
  `5E96766F8B88082E917D3EA478F07B65155B1EBC4C09DB384739BD9BA047A65C`.
- `shiplua_manifest_validator`: Dynamic Movement Remake 0.2.5 válido.
- Playtest final isolado `20260912-190921`: `valid: true`, um mod carregado, sem
  exceção e com todos os arquivos rastreados inalterados. Marcador:
  `unbound-002a-layers: layers=2; ...linkspan-layer-base.zip>...linkspan-layer-override.zip; merged=a0cd794e461ab44b; cleanup=ok`.
- Overlay extraído novamente para verificação: checksums internos aprovados e
  nenhum `oot.o2r` presente.

## Resultado da validação

- Todos os testes executáveis e validadores retornaram código 0.
- O provider foi recompilado com velocidades 7 e 10 sem mudar o hash do host.
- O playtest confirmou duas camadas em ordem base → override e cleanup dos dois
  handles antes de `ShipLua inicializado`.
- A sessão estrita confirmou executável, O2Rs, mod e fixtures inalterados.
- O lifecycle reconhece claim e handoff como completos; o resultado global
  permanece não estrito apenas porque claims predecessoras em `review`
  (`OOT-CORE-001`, `OOT-COREEXT-001`, `OOT-MOVE-001`, `OOT-NATIVE-001` e
  `OOT-OVERLAY-001`) ainda reivindicam arquivos compartilhados desta pilha.

## Artefatos

- `OOT-NATIVE-001/build/distribution/Dynamic-Movement-Remake-0.2.5.zip`
  - SHA-256 `76F8075CBE40A3760FEC61FBD1F62550F237E1CC6E7B6D8D68212CD4107A4556`
- `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-OOT-UNBOUND-002A-ResourcesV2-Win64-overlay.zip`
  - SHA-256 `945DE40CE0A89BF5332AA6F328D64A5BFAD3943E5DE526C2A8BCD76D788DFDD5`
- Evidências:
  `OOT-NATIVE-001/build/runtime-evidence/unbound-002a-final-20260912-1912/`.

## Pendências

- `read_file_layers` entrega bytes e proveniência, mas ainda não registra uma
  factory externa no `ResourceLoader` nem fornece uma estratégia de merge por
  schema.
- Próximo recorte: factory JSON externa e merge por schema no coremod Unbound
  (`UNBOUND-002B`), consumindo `read_file_layers`.

## Riscos

- Bytes e caminhos recebidos pelo callback pertencem ao host e precisam ser
  copiados pelo mod caso sobrevivam à chamada.
- FNV-1a identifica conteúdo para cache e diagnóstico, mas não autentica um
  archive.
- Providers nativos continuam tendo acesso ao processo inteiro e podem causar
  crash; o contrato C evita ownership C++ cruzado, não oferece sandbox.
- Os ZIPs `linkspan-*-probe` do overlay são fixtures de validação e não
  representam conteúdo do Unbound final.

## Próxima ação recomendada

Abrir `OOT-UNBOUND-002B` para permitir que uma core extension registre uma
factory JSON por tipo/schema, consuma `read_file_layers` e publique o resultado
mesclado por handle sem adicionar código específico do mod ao host.
