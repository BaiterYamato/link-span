# Handoff — UNBOUND-022

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Review. Commit local no host (`c6c46379c`), sem push. Framework Unbound 0.6.4 empacotado só na evidência.

## Resultado

Pendência da UNBOUND-021 (M01 da matriz do Prelude): o que muda além de ±1 048 576 (`BGCHECK_XYZ_ABSMAX`) e o
framework avisar. A conclusão, depois da revisão:

- **A colisão continua.** O `BGCHECK_POS_ERROR_CHECK` só imprime nos cinco caminhos do `z_bgcheck.c` (chão, parede,
  teto, linha, esfera). O aviso é `osSyncPrintf`, desligado neste build (`LOG_LEVEL_GAME_PRINTS=6`). Em jogo, o chão
  vale até o fim da geometria (UNBOUND-021).
- **O que muda são os efeitos.** Com |c| > 1 048 576, o `z_effect_soft_sprite.c` apaga os `EffectSs` (poeira,
  faíscas, respingos) em vez de desenhá-los, sem aviso no log.

O framework anota spawns, atores de sala, transition actors e `bounds` da colisão nesse caso. A nota sai antes do
gameplay para documento de mod (checagem da UNBOUND-019) e no log do Unbound quando a cena carrega. Sem mudar o host
nem o XML.

- **Texto:** uma nota por lista, com a contagem e a primeira posição. Exemplo:
  `<doc> spawns: 1 posição(ões) além de ±1048576 (ex.: 0=1048577,0,0); lá o jogo apaga os efeitos de partícula
  (EffectSs) em vez de desenhá-los`.
- **Limiar:** módulo > 1 048 576 (estrito, como o `EffectSs`), medido no valor que o jogo recebe. Nas posições é o
  f32, porque as fábricas leem com `FloatAttribute`: 1 048 576,03 vira 1 048 576 e não é anotado. Nos bounds é o
  inteiro do `Integral` que vai ao XML: 1 048 576,5 vira 1 048 577 e é anotado.
- **XML:** inalterado; a posição continua indo como veio.
- `DecimalText` saiu do `Xml::Float` para as notas usarem a mesma formatação (ponto decimal com qualquer locale).
- Fixture `UB022-A` no `make-unbound-limit-fixtures.py` (só JSON).

## Revisão

Revisão do Codex (`gpt-6.1-sol` alto) sobre a 021 e a 022: `REVIEW.md` em `codex-rev-unbound021-022`, no scratchpad
053dbb62, com testes em g++ e comparação de 32 XMLs antes e depois (zero diferenças). Três achados médios, todos
corrigidos por mim; re-revisão das correções em `codex-rerev-021-022`.

| # | Achado | Correção |
|---|---|---|
| 1 | A nota dizia que parede, teto, linha de visão e esfera recusam a posição; no host, o check só imprime | nota, README e comentários reescritos para o efeito real (`EffectSs` apagado); handoff da 021 corrigido |
| 2 | A nota comparava o double do JSON: 1 048 575,99 (f32 = 1 048 576) e bounds 1 048 575,5 (XML = 1 048 576) escapavam | compara o f32 nas posições e o `Integral` nos bounds, com o limiar estrito do `EffectSs`; testes de arredondamento |
| 3 | (da 021) a fixture `edge_*` não pegava a regressão da máscara de 13 bits | corrigido na 021 (e-fixtures 0.1.3) |

Re-revisão (`REVIEW.md` em `codex-rerev-021-022`, com harness g++ e o `FloatAttribute` real do tinyxml2): achados
1 e 3 resolvidos, 2 parcial. Restaram dois, corrigidos por mim na UNBOUND-023 (framework 0.6.5):

| # | Achado | Correção |
|---|---|---|
| 4 (médio, resíduo do 2) | a nota usava o cast direto do double; o XML leva o texto `%.9g`, então ±1 048 576,063 virava o f32 ±1 048 576,125 na nota, mas o jogo lê ±1 048 576 (18 falsos positivos por locale) | o f32 sai do texto do `DecimalText` via `std::from_chars`; teste `.063` nos dois sinais, três listas e três eixos |
| 5 (baixo) | o README dizia chão provado até ±1 048 806, posição em que o log mostra perda de chão | texto corrigido: piso até a borda em ±1 048 800, perda de chão logo depois |

## Commits

- host `c6c46379c` — `unbound: nota para posição além do limite do mundo (EffectSs), framework 0.6.4 (UNBOUND-022)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/transcode.cpp`: `DecimalText`, `kWorldLimit`, `WorldLimitCheck` e as quatro notas.
- `soh/native-sdk/unbound-core/limits_tests.cpp`: `TestWorldLimit` (no limite e além em cada lista, arredondamento
  f32 e inteiro, os três eixos nos bounds, XML intacto).
- `soh/native-sdk/unbound-core/README.md`: seção "Limite do mundo (0.6.4)".
- `tools/package-unbound-framework.ps1`: versão 0.6.4.
- `tools/make-unbound-limit-fixtures.py`: grava também `UB022-A.o2r`.

## Validação executada

- Build do framework sem avisos; CTest 7/7.
- Pacote `LinkSpan-Unbound-Framework-0.6.4-layout-9a593a6e.shipmod` (`51389434…`) em
  `build/oot-upstream-001/evidence/unbound-022/package`.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`), sem input: cinco sessões até o `game.ready`
  (`ub022-proof.ps1` no scratchpad 053dbb62). Todas válidas no harness, com exit 0 e `mods/` restaurada.

## Resultado da validação

Pacote final `51389434…`, cinco sessões, todas válidas, com exit 0. A rodada anterior à revisão (`40028523`) deu os
mesmos números com o texto antigo e está em `run1-40028523`.

| Sessão | Camadas | Resumo no log |
|---|---|---|
| `mundo` | UB022-A | `referências: documentos=3 conferidas=3 ausentes=0 recusados=0 notas=4 ms=0` |
| `limites` | UB020-A | `documentos=3 conferidas=3 ... notas=4 ms=693` (igual à 020) |
| `ub019-a` | UB019-A | `documentos=4 conferidas=8 ausentes=3 recusados=1 ms=3` (igual à 019) |
| `ub018-abc` | UB018 A, B, C | `documentos=4 conferidas=26 ausentes=0 recusados=0 ms=6` (igual à 018) |
| `sem-camadas` | — | `documentos=0 ... ms=0` |

No `logs/linkspan-unbound.log` da `mundo` saem as quatro notas: spawn `0=1048577,0,0`, transition actor
`0=0,-1048600,0`, bounds `max=1048800,10,10` e ator `0=0,0,-1048576.5`. O ator em x = 1 048 576 exato não sai.

## Riscos

- Paths, luzes pontuais, centros de mesh tipo 2 e water boxes não são conferidos. Um ator que segue path pode
  sair do limite sem nota; luz e água não geram `EffectSs` por si.
- Setups alternativos são transcodificados e anotados como o setup 0 (mesma função).
- A geometria do `collision.bin` não é lida pelo framework: vértice além do limite com `bounds` dentro não gera nota.
- A nota não diz o número do setup; num documento com setups alternativos o autor precisa procurar.

## Pendências

- Restante da prioridade 5 da matriz (M12, M15, M16/M17/M19, M24, R02).

## Próxima ação recomendada

Levar o 0.6.4 para a próxima RC. Continuar a prioridade 5 com R02 e M12 na e-fixtures.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host c6c46379c na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 4c18c16.
- Host: c6c46379c (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
