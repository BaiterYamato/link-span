# Handoff — OOT-UPSTREAM-003

## Estado

Concluída: RC8 publicado como pre-release em 12/10/2026 ([oot-0.5.0-rc8](https://github.com/BaiterYamato/Shipwright/releases/tag/oot-0.5.0-rc8)); PR #3 (`4d8d7df622`) e
link-span-examples#2 (`5550620`) mergeados, com a mesma exceção do `clang-format`.
- Host: branch `agent/OOT-UPSTREAM-003-rc8`, PR [BaiterYamato/Shipwright#3](https://github.com/BaiterYamato/Shipwright/pull/3)
  (mergeado), sobre o merge `783e5605b8` da `agent/OOT-UPSTREAM-001-78dc6d970-integration` (#1 e #2 já mergeados).
- Exemplos: branch `agent/OOT-UPSTREAM-003-rc8` (`6379ead`), PR
  [BaiterYamato/link-span-examples#2](https://github.com/BaiterYamato/link-span-examples/pull/2).
- RC8 `oot-0.5.0-rc8` montado em `build/release/oot-0.5.0-rc8/`. O RC7 fica em rascunho (409931091) e não é publicado
  (decisão do mantenedor em 11/10/2026); o RC8 foi publicado com o ok dele em 12/10/2026.

## Resultado

- Opções do SoH gravadas por código: `SetSohCVarInt` grava o CVar e, se o valor mudou, enfileira o nome;
  `LinkSpan_ApplyPendingSettings` chama `ShipInit::Init` para cada nome no começo do `GameState_Update` (o que hotkeys e
  menu gravam no `Graph_StartFrame`) e no fim dele, depois do `OnGameFrameUpdate` (timers, `game.frame`). As duas chamadas
  ficam fora de qualquer despacho do GameInteractor. Passam pela fila o `set_setting_int` dos mods nativos e as seis
  gravações de `PersistentMasks` da API Lua de máscaras. Sem mod, a fila fica vazia.
- Skip da intro do Ganon (#7321): os dois passos dos destroços passam por `LinkSpan_ActorUpdate`, como o loop comum.
- NEI 0.3.23, Unbound Framework 0.6.13, DMR 0.2.22, Mic Ocarina 0.2.8, WW 0.1.4 e Shovel Demo 0.1.4. Layout `7376240d`
  inalterado.
- `soh.exe` compilado de `d37f2bcb18`, SHA-256 `fa236d42b500f478bf3995fb3eed345a7c210ace6c82a18c4efc91c758f780f9`.
  `548984679a` só formata um arquivo de teste e não muda o executável.
- Codex: auditoria do 9.3.0 (RC7), duas rodadas de revisão do patch (a segunda gerou a reaplicação no começo do frame),
  teste de roteamento no `EngineTests` e script dos docs do core para o RC8. Ajustes, build e validação foram meus.

## Commits

- `161f7f8498`: fila de opções do SoH e skip da intro do Ganon.
- `c6216b9594`: versões dos mods do RC8.
- `d37f2bcb18`: reaplicação também no começo do frame (fonte do executável distribuído).
- `548984679a`: clang-format no bloco novo do `EngineTests.cpp`.
- Exemplos `6379ead`: Shovel Demo 0.1.4.

## Arquivos alterados

- `soh/soh/ShipLuaBootstrap.cpp`, `soh/src/code/game.c`,
  `soh/soh/Enhancements/TimeSavers/SkipCutscene/GanonIntros.cpp`, `soh/native-sdk/tests/EngineTests.cpp`.
- Manifests e READMEs dos mods; `tools/package-{nei-core,unbound-framework}.ps1`.
- Fora do git do host: `logs/rc-pipeline-oot-next/` (`rc8-sdk/RELEASE-NOTES.md`, `rc8-coordination/`).

## Validação executada

- Build Release, CTest 68/68 (um teste novo de roteamento de settings); `rc-post-build` (reprodutibilidade, scanner
  OK).
- Bateria do RC8 sobre o Shipwright 9.3.0 oficial (11/10/2026, desde 19:24), gates com controle conectado e
  `-PrevTag rc7`.
- Evidência: `build/oot-upstream-001/evidence/release-g/rc8/` no host.

## Resultado da validação

Tudo na primeira passada (19:24–22:30), sem repetição:

- Instalação fresca: PROVA OK (0 mods, 371 comandos de cena XML).
- Sem mods (cópia fresca): 0 falhas, 0 erros no log, seções `linkspan` e `unbound` do save iguais.
- Matriz 9/9.
- Ciclo de save: retirar e reinstalar o NEI sem perder dados.
- Atualização NEI 0.3.22 → 0.3.23: ATUALIZAÇÃO OK.
- Fixtures: pacote 0.1.12 e 9 cenários (completa, boot, savestate, m18, a8200, r04, r0809, água, luzes) com 0 falhas.
- Sonda do SDK: carregada e descarregada; autocontenção: 37 referências, 0 ausentes.
- Gates 5/5: add-ons, capability, recuperação de boot, seletor de pausa e rollback RC8 → RC7 → RC8.
- Notas corrigidas depois da bateria (reaplicação "no fim do frame" → começo e fim); release regravada com os mesmos
  artefatos: só o hash das notas mudou em `SHA256SUMS.txt`, scanner de conteúdo protegido OK.
- CI do #3: `clang-format` falha só pelos arquivos antigos; nenhuma linha nova aparece no diff dele.
- Evidência: `RESUMO.txt` e `gates/` em `build/oot-upstream-001/evidence/release-g/rc8/`.

## Pendências

- Docs do core (README e wiki em `docs/wiki`) e esta coordenação vão ao link-span pela branch
  `agent/OOT-UPSTREAM-003-core-docs`; publicar a wiki (`link-span.wiki`) espera autorização.
- Destino dos rascunhos do RC6 e do RC7 no BaiterYamato/Shipwright: decisão do mantenedor.
- Prova física: reaplicação das opções com o DMR (descarregar no meio da sessão e conferir que a primeira pessoa
  volta), controle/câmera do DMR e microfone do Mic Ocarina.
- OOT-FORMAT-001 no começo do próximo ciclo (muda o layout id).

## Riscos

- Não há cenário automático em jogo para a reaplicação: a API Lua não lê a máscara atual. Cobertura por revisão, CTest e
  bateria.
- Uma opção gravada dentro de callback de ator, antes do update do jogador, só vale no frame seguinte.
- RC8 exige Shipwright 9.3.0 limpo; overlay e mods do RC6 não servem no layout `7376240d`.
- Mods nativos rodam no processo do jogo, sem sandbox; caminhos longos podem estourar MAX_PATH.

## Próxima ação recomendada

- Acompanhar o retorno da pre-release (issues no BaiterYamato/Shipwright), fazer a prova física das opções com o DMR
  e começar a OOT-FORMAT-001 no próximo ciclo.
