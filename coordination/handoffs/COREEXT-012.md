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
