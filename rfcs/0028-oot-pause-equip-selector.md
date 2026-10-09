# RFC 0028 — Seletor de equipamento do pause sem nome de mod no host (OoT)

Status: **aceita pelo mantenedor em 2026-10-09; fase 1 implementada** no candidato `oot-0.5.0-rc5` (host, DMR
0.2.19 e NEI 0.3.19). A fase 2 (lease) continua para a próxima mudança que já altere o layout id. Rascunho de
2026-10-04, número reservado pela tarefa COREEXT-012. Detalha a saída que a RFC 0027 esboça para a parte "seletor do DMR no kaleido" do commit `a76045848`.
As outras duas frentes C da RFC 0027 (áudio por caminho em `b13913077` e `IsUnboundLayerArchive` no bootstrap)
ficam fora daqui.

## Motivação

Fatos no host da integração (`0ff8f2ff4`):

- `soh/soh/native/OotNativeEngine.cpp:547` exporta `LinkSpan_DynamicMovementInventoryMenuEnabled()`, que devolve
  `dpadHudOwned`. Esse flag é o setting transitório `linkspan.hud.dpad` (mesmo arquivo, linhas 141–185), ligado pelo
  Dynamic Movement Remake (`soh/native-sdk/example/provider.cpp:350`).
- `soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_item.c` implementa um seletor: com o flag ligado, A sobre um
  item abre três molduras dos botões C (linhas 20–50 desenham, 744–755 abrem), e analógico/D-pad escolhem, A equipa
  pelo `KaleidoScope_SetupItemEquip` e B cancela (430–488).
- O fork do NEI copia a mesma máquina de estados na sua versão de `KaleidoScope_DrawItemSelect`
  (`nei-core/fork/overlay-merges/z_kaleido_item.KaleidoScope_DrawItemSelect.c`) e importa o símbolo do host
  (`nei_fork_protos.h:39`). A linha 36 dessa cópia ainda usa o símbolo para saber que o DMR mapeia D-direita para
  C-cima fora do inventário.

Problemas:

1. Nome de mod num símbolo do host, o que viola a regra §4 do plano depois do COREEXT-GATE.
2. Ter o HUD do D-pad liga uma segunda função, sem relação com ele. A RFC 0027 pede separar as duas coisas.
3. Não há dono: qualquer provider que ligue `linkspan.hud.dpad` ganha também o seletor.
4. O fork do NEI conhece um mapeamento de input do DMR.

Por causa disso, a linha "nenhuma onda de itens exige função específica no host" do Gate OoT (§18) está 🟡.

## Por que só a DLL não basta

| Mecanismo | Por que não resolve sozinho |
|---|---|
| escape hatch (RFC 0023) | O DMR teria de substituir `KaleidoScope_DrawItemSelect`, que o NEI já substitui. Com dois mods no mesmo alvo, o segundo recebe `LIMIT` e não há encadeamento (RFC 0023, linha 61). A linha 07 da matriz (DMR com NEI) quebraria. |
| hooks nativos (RFC 0021) | Pontos de hook são declarados por core extension. O pause do host não tem nenhum, e declarar um dentro do overlay já é recompilar o host. |
| HUD do Lua | Desenha, mas não consome o input do pause antes do kaleido. |

O host precisa manter um ponto de despacho no kaleido. O que muda é o contrato desse ponto.

## Decisão proposta

### Fase 1 — próxima rodada de host, sem mudar o layout id

1. **Setting transitório próprio:** `linkspan.pause.equip_selector`, com o bit `8` no probe
   `linkspan.transient_settings`. Fica independente de `linkspan.hud.dpad`. Como os demais settings transitórios,
   vive na memória do host, não toca CVar e volta a 0 quando a ponte do gamepad troca (unload do mod).
2. **Helper do host:** `LinkSpan_PauseEquipSelectorEnabled()` lê só o setting novo. O kaleido passa a usá-lo, e o
   comentário "Dynamic Movement: …" da linha 20 vira uma descrição genérica.
3. **Sem alias:** `LinkSpan_DynamicMovementInventoryMenuEnabled()` sai na mesma rodada. O fork do NEI não importa
   símbolos do host pelo PE: `fork/gen_imports.py` gera thunks MASM com os endereços do `soh.symbols` de cada build,
   e o fork só liga quando o fingerprint bate. Então o NEI já é recompilado a cada rodada de host, e o fork troca de
   nome no mesmo commit. Um NEI antigo num host novo continua carregando com o fork inativo, como hoje.
4. **Limpeza:** o estado do seletor (`sLinkSpanEquip*`) zera ao sair da página ou do estado 6, que é o que já
   acontece. Zera também quando o setting cai para 0 com o seletor aberto: fecha sem equipar e não deixa input
   pendente.
5. **DMR:** liga `linkspan.pause.equip_selector` junto com `linkspan.hud.dpad` quando o probe tem o bit 8. Num host
   antigo, sem o bit, não liga nada novo, e o helper antigo do host continua devolvendo `dpadHudOwned`.
6. **Fork do NEI:**
   - chama o helper novo;
   - a linha 36 passa a perguntar `LinkSpan_DpadHudOwned()`, que já é genérico (o dono do HUD do D-pad remapeia as
     direções; dentro do inventário, o NEI ignora o C-cima sintetizado).

**Limite conhecido da fase 1:** sem lease, quem grava por último vence, como em todos os settings transitórios de
hoje. Dois providers que liguem o seletor convivem; se um desligar, desliga para os dois.

### Fase 2 — só junto de uma mudança de ABI que já mude o layout id

Serviço `linkspan.oot.pause` v1, com lease por contexto de provider: o primeiro dono recebe `OK`, o segundo `LIMIT`,
e o `NativeProvider` solta o lease no unload. O dono vem do contexto do provider, nunca de um nome passado como
argumento (RFC 0027, linhas 91–92). O setting transitório vira fachada do lease. A fase 2 não entra sozinha numa
rodada: um header novo no SDK muda o layout id e obriga a reempacotar todos os mods nativos.

### Alternativas descartadas

- **Escape hatch no DMR:** recebe `LIMIT` ao lado do NEI (tabela acima).
- **Só trocar o nome do helper:** a RFC 0027 (linha 89) diz que isso não encerra a dívida, porque o flag do D-pad
  continuaria ligando o seletor.
- **Remover o seletor:** viável. O DMR perde "A escolhe o botão C", e os itens voltam a ser equipados pelos botões C,
  como no jogo original. A decisão é do mantenedor. Se for a escolhida, a fase 1 vira remoção:
  - no kaleido, as linhas 20–50, 430–488, 744–755 e a chamada das linhas 868–870;
  - a exportação do símbolo;
  - o mesmo trecho nas cópias do fork.

## Contrato

- **Lua:** nenhum. Não há API Lua nova.
- **Nativo:** o nome do setting e o bit do probe. `set_setting_int` fora da thread do jogo devolve `UNSUPPORTED`,
  como já faz; qualquer valor diferente de 0 vale 1; gravar no probe devolve `INVALID_ARGUMENT`.
- **Capacidade:** o bit `8` em `linkspan.transient_settings`. Nenhuma capacidade Lua.
- **MM:** não se aplica. O 2Ship não tem o kaleido do OoT nem o DMR; se um dia tiver, o setting é por host.

## Compatibilidade

| Host | DMR | NEI | Resultado |
|---|---|---|---|
| novo | novo | novo | seletor pelo setting novo, nos dois caminhos (host e fork) |
| novo | antigo (só `hud.dpad`) | qualquer | sem seletor; A sobre item segue o fluxo do jogo original; HUD do D-pad intacto |
| novo | qualquer | antigo | o NEI carrega com o fork inativo pelo fingerprint, como em toda rodada de host |
| antigo | novo | qualquer | probe sem o bit 8; o helper antigo devolve `dpadHudOwned` e o seletor funciona como hoje |

- **Layout id:** nenhum header do SDK muda, porque o setting é uma string mais um bit, definidos no host e no
  provider. O `soh.exe` muda, então NEI, DMR e Mic precisam ser reempacotados com o fingerprint novo, como em
  qualquer rodada.
- **Categoria na RFC 0027:** B, serviço ou hook genérico.

## Segurança

Não abre superfície nova: é um booleano na memória do host, sem persistência e sem ponteiro exposto. O seletor
continua passando por `GameInteractor_Should(VB_EQUIP_ITEM_TO_C_BUTTON, …)` e pelas checagens de idade e de item
vendido.

## Plano de testes

- **Teste de unidade do `OotNativeEngine`:**
  - o probe inclui o bit 8;
  - set/get do setting novo;
  - o setting volta a 0 quando a ponte troca;
  - o helper segue o setting novo e não o `linkspan.hud.dpad`.
- **Build do fork:** o `gen_imports.py` resolve o helper novo no `soh.symbols` e o nome antigo não aparece mais.
- **Fixture com outro nome de mod** (aceite da RFC 0027):
  - um provider que liga só o setting novo abre o seletor com A;
  - um que liga só `linkspan.hud.dpad` não abre.
- **Em jogo:**
  - DMR sozinho;
  - DMR com NEI (caminho do fork);
  - sem mods, com A sobre item no fluxo do jogo original;
  - desligar o setting com o seletor aberto (fecha sem equipar, sem A/B/D-pad pendentes);
  - salvar e recarregar.
- **Matriz §12.2 do RC seguinte:** linhas 01, 05, 06 e 07.

## Depreciação

Não há período de depreciação. O símbolo antigo não é API pública: só o fork do NEI o usa, e ele é recompilado contra
o `soh.symbols` de cada host. O DMR num host antigo continua funcionando, porque o probe não traz o bit 8 (tabela de
compatibilidade).

## Efeito no Gate

Com a fase 1 implementada e provada, a linha "nenhuma onda de itens exige função específica no host" pode passar a ✅:
- nenhum símbolo de host com nome de mod;
- o ponto que fica no kaleido é genérico e justificado pelo `LIMIT` do escape hatch.

O item C `a76045848` da RFC 0027 fecha na parte do kaleido; o `IsUnboundLayerArchive` continua aberto.
