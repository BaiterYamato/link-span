# Handoff — COREEXT-012

## Estado

Review. RFC 0028 em rascunho, commitada no core (branch `agent/MENU-001-mod-menu`, sem push). Nada foi implementado no
host.

## Resultado

`rfcs/0028-oot-pause-equip-selector.md` detalha a saída da parte "seletor do DMR no kaleido" do item C `a76045848` da
RFC 0027.

- **Por que o host precisa de um ponto no kaleido:**
  - o escape hatch daria `LIMIT` ao DMR, porque o NEI já substitui `KaleidoScope_DrawItemSelect` (RFC 0023: segundo
    mod no mesmo alvo);
  - os hooks nativos não têm ponto no pause;
  - o HUD do Lua não consome input antes do kaleido.
- **Fase 1** (próxima rodada de host, sem mudar o layout id):
  - setting transitório `linkspan.pause.equip_selector` (bit 8 do probe), separado de `linkspan.hud.dpad`;
  - helper `LinkSpan_PauseEquipSelectorEnabled()`;
  - o nome antigo sai sem alias: o fork do NEI resolve símbolos pelo `soh.symbols` de cada build (`gen_imports.py`)
    e é recompilado a cada rodada;
  - a linha do fork que conhece o mapeamento D-direita → C-cima do DMR passa a usar `LinkSpan_DpadHudOwned()`;
  - o seletor fecha sem equipar se o setting cair com ele aberto.
- **Fase 2** (só junto de uma mudança de ABI que já mude o layout id): serviço `linkspan.oot.pause` com lease por
  contexto de provider.
- **Alternativas:**
  - descartadas: escape hatch no DMR e só renomear o helper;
  - remover o seletor fica como opção do mantenedor.

## Commits

- core `55ece4b` — `rfcs: 0028 (rascunho) seletor de equipamento do pause sem nome de mod no host (COREEXT-012)`.

## Arquivos alterados

- `extern/ship-lua/rfcs/0028-oot-pause-equip-selector.md` (novo).

## Validação executada

Referências da RFC conferidas no host `0ff8f2ff4`:

- `OotNativeEngine.cpp`:
  - 125–128 (bits do probe);
  - 141–185 (setting `linkspan.hud.dpad`);
  - 547 (export);
  - 712 (`LinkSpan_DpadHudOwned`).
- `z_kaleido_item.c`:
  - 20–50 (desenho);
  - 430–488 (máquina de estados);
  - 744–755 (abertura);
  - 868–870 (chamada do desenho).
- Fork do NEI:
  - `nei_fork_protos.h:39`;
  - linha 36 da cópia de `KaleidoScope_DrawItemSelect`;
  - `gen_imports.py`, que gera thunks com os endereços do `soh.symbols`.
- `provider.cpp:350` (DMR liga `linkspan.hud.dpad`).
- RFC 0023, linha 61 (`LIMIT` no mesmo alvo).

## Resultado da validação

Documento coerente com o código. Não houve build nem prova em jogo, porque não há implementação.

## Riscos

- A fase 1 não tem lease: quem grava por último vence, como em todos os settings transitórios de hoje.
- Qualquer mudança no host exige reempacotar NEI, DMR e Mic (fingerprint), como em toda rodada.

## Pendências

- Decisão do mantenedor: aprovar a fase 1, ou remover o seletor.
- Se aprovada, implementar numa rodada de host junto de outras mudanças (regra de agrupamento da RFC 0027), com os
  testes do plano da RFC.

## Próxima ação recomendada

Revisar a RFC 0028 junto da 0027. Com a fase 1 implementada e provada, a linha do Gate "nenhuma onda de itens exige
função específica no host" pode virar ✅.

### Fechamento para o RC5 (2026-10-09)

**Estado:** RFC 0028 aceita; fase 1 implementada no host do RC5. Este registro supera o estado anterior
"Nada foi implementado no host". O fechamento da prova em jogo fica em review até preencher a bateria abaixo;
a fase 2 continua fora desta rodada. Commits locais, sem push.

**Resultado:**
- `45530b878`: setting `linkspan.pause.equip_selector`, separado de `linkspan.hud.dpad`, e helper
  `LinkSpan_PauseEquipSelectorEnabled` no lugar do símbolo com nome de mod, sem alias.
- O seletor fecha sem equipar se o setting cair com ele aberto. O ponto no kaleido fica genérico; a justificativa
  da RFC permanece: o NEI já substitui `KaleidoScope_DrawItemSelect`, então outro replace daria `LIMIT` ao DMR.
- Layout `9a593a6e` sem mudança. Host final do RC5 embute `ddeffea` (`ddeffeadf`), SHA-256
  `A65950B1FFB0380FD6F242E0AD22BDE3B67BDFC55D125BD83D7936DDE144850B`.
- NEI 0.3.20, DMR 0.2.19 e Mic 0.2.5 são as versões desta rodada; o fingerprint precisa corresponder ao host final.

**Rebuild do NEI** (host `10a9407cc`): `/FI` com `linkspan_vanilla.h` e chave do host por ponteiro, macros só do
host copiadas pelo extrator, três conflitos resolvidos à mão (`Message_Decode`, `Message_DrawMain` e
`EnMThunder_SpinAttacking`, mais `Message_DecodeChecked` em overlay-extra) e assinatura antiga da resolução de
`KaleidoScope_DrawItemSelect` corrigida. Commit registrado; provado em jogo no RC5 com NEI 0.3.20 (OOT-RELEASE-001).

**Commits:** host `45530b878` (fase 1), `bf8715832` (versões), `10a9407cc` (rebuild NEI); core `99d912a` (RFCs 0027/0028 aceitas).

**Validação:** CTest 68/68 no host final. As provas anteriores desta tarefa eram documentais no `0ff8f2ff4`.
No RC5 (bateria e gates, OOT-RELEASE-001):
- gate-selector: sem DMR, A sobre o item segue o fluxo original e `buttonItems` não muda;
- gate-selector: molduras abrem, Deku Nut vai ao C-right e fica após save e reinício;
- queda do setting com o seletor aberto: não exercitada em jogo no RC5 (pendente);
- matriz 9/9; NEI 0.3.20, DMR 0.2.19 e Mic 0.2.5 com fingerprint `a65950b1…`;
- gate-rollback ok/completo.

**Pendências e riscos:** provar em jogo a queda do setting com o seletor aberto. Na fase 1 não há lease:
quem grava por último vence. A fase 2, `linkspan.oot.pause` com lease por provider, fica para uma mudança de ABI
que já altere o layout id. Recompilar o host exige reempacotar os mods que fixam o fingerprint.

**Próxima ação:** fase 1 provada no RC5; fica a prova da queda do setting com o seletor aberto. Fase 2 junto da próxima mudança de ABI.
