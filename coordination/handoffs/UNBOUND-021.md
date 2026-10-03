# Handoff — UNBOUND-021

## Estado

Review. Commit local no host (`e06663aa5`), sem push. E-fixtures 0.1.2 empacotada só na evidência (tem dados do jogo
copiados da base convertida, como as anteriores).

## Resultado

Gate OoT §10.3/§14.3, prioridade 5 da matriz de limites do Prelude (`MATRIZ.md` em `codex-prelude-limits`, scratchpad
053dbb62): fixtures só com recursos nossos e prova em jogo, sem mudar o host. Três cenas novas no roteiro da
e-fixtures (tecla K), geradas pelo `make_fixtures.py`. Display lists e vértices em XML, `collision.bin` próprio e só
o setup (luz, céu, som) copiado do Hyrule Field da base.

- **`edge_pos` / `edge_neg` (M01, M13, M14):** piso nos cantos (+x, −z) e (−x, +z) do mundo. O spawn fica em
  (±1 048 500, 0, ∓1 048 500), 76 unidades antes de ±1 048 576 (`BGCHECK_XYZ_ABSMAX`) nos dois eixos. O piso passa do
  limite até ±1 048 800, e faixas azuis marcam x e z = ±1 048 576. O `collision.bin` tem exatamente 8 193 vértices
  e 32 769 polígonos. O chão sob o spawn é o polígono 32768, que usa o vértice 8192: os dois ficam acima dos tetos
  vanilla (índice de vértice de 13 bits e índice de polígono s16). O enchimento repete uma grade de 91×89 vértices
  nos índices baixos.
- **`span` (M02):** uma display list com um quad de 81 920 unidades (vértices em x = ±40 960), com piso de colisão
  igual e spawns em x = +40 000 (`main`) e x = −40 000 (`oeste`). Com vértice s16, x = 40 960 viraria −24 576 e o
  trecho sob os spawns não seria desenhado.
- **Sonda da e-fixtures:** o relatório traz `piso=<índice do polígono de chão na colisão estática>` e
  `alturaChao=`. Cada mudança do flag de chão sai como `chao 1->0` / `chao 0->1`, com até 24 por cena. Um relatório
  de quadro perdido não some mais (`>=` no lugar de `==`).

**Achado (M01):** no limite do mundo, o chão continua valendo além de ±1 048 576. O jogador só perde o chão onde a
geometria acaba (±1 048 806 = 1 048 800 + raio). No `z_bgcheck.c`, o raycast de chão chama o
`BGCHECK_POS_ERROR_CHECK` só para imprimir (linha 1678) e segue. Já o line test (1892), a parede (2080), o teto (2154)
e a esfera (2367) retornam cedo, sem colisão, quando a posição passa do limite. O aviso é `osSyncPrintf` e não chega
ao log do SoH. Para o autor, isso quer dizer que além de ±1 048 575 só o chão funciona: parede, teto, linha de visão
(câmera, projéteis) e esfera não colidem, e nada avisa. O "intervalo efetivo aberto" da matriz vale para esses
quatro caminhos, não para o chão.

## Commits

- host `e06663aa5` — `unbound: fixtures do Prelude com recursos próprios nos limites (e-fixtures 0.1.2, UNBOUND-021)`.

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_edge`, `make_span`, `edge_collision`, quads e
  display lists XML genéricos.
- `soh/native-sdk/unbound-core/e-fixtures/e_fixtures.cpp`: `piso=`, `alturaChao=`, mudanças de chão, relatório com
  `>=`.
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: quatro entradas novas no roteiro (antes do retorno à casa).
- `tools/package-unbound-e-fixtures.ps1`: versão 0.1.2, dependência do framework `>=0.4.0 <0.7.0` (a 0.1.1 exigia
  `<0.6.0` e não carregava com o framework 0.6.x).

## Validação executada

- Gerador contra a base `playable-nei-completion-20260930/oot-unbound.o2r`: 10 cenas; checker do Unbound
  `verificados=27 recusados=0`. Conferido no `collision.bin`: 32 769 polígonos, e o 32768 contém o vértice 8192 e o
  ponto do spawn nas duas cenas (o gerador faz `assert` disso).
- Build do DLL sem avisos (`linkspan_unbound_e_fixtures.dll`); framework 0.6.3 sem mudança (`AE99F255`).
- Pacote `LinkSpan-Unbound-E-Fixtures-0.1.2-layout-9a593a6e.shipmod` (`EF4C4F07…`) em
  `build/oot-upstream-001/evidence/unbound-021/e-fixtures`; validador do Link-Span ok.
- Prova em jogo na cópia `smoke-rc2-20261003` (host RC2 `E278E9A3`, `oot.o2r` NTSC-US 1.1), com o framework 0.6.3 e a
  e-fixtures 0.1.2. Foi uma sessão: abre o arquivo 1, percorre o roteiro com K e, em cada edge, anda para fora em x,
  espera o void-out e anda para a esquerda. Script `ub021-proof.ps1` no scratchpad 053dbb62; sessão válida no
  harness, exit 0, `mods/`, `Save/` e config restaurados.

## Resultado da validação

Sessão `evidence/unbound-021/session` (a `run1` dessincronizou o roteiro com um K durante o void-out e não chegou à
`span`; mesmos números nas edge).

| Cena | Chegada | Depois de andar para fora (x) | Captura |
|---|---|---|---|
| `edge_pos` | `pos=1048500.0,0.0,-1048500.0 chao=1 piso=32768` | `chao 1->0` em x = 1 048 806,0; queda a y = −3 090; respawn no spawn (`piso=32768`) | `10`–`14` |
| `edge_neg` | `pos=-1048500.0,0.0,1048500.0 chao=1 piso=32768` | `chao 1->0` em x = −1 048 806,0; queda; respawn no spawn | `20`–`24` |
| `span/main` | `pos=40000.0,-0.0,0.0 chao=1 piso=1` | — | `30`: faixa desenhada sob o Link até o horizonte |
| `span/oeste` | `pos=-40000.0,0.0,0.0 chao=1 piso=0` | — | `31`: idem na outra ponta |

As cenas da fase E no caminho (casa, wide, cavern, cavern_b, lab, lab_b, many) chegaram como antes, com o relatório
novo (`piso=` na wide = 39562, acima de s16, e na many = 22349). Nenhuma linha de erro no log.

## Riscos

- O trecho "andar para a esquerda" não testou o limite em z: depois do respawn a câmera girou e o Link andou para
  dentro (−195 em x, +59 em z). O extremo em z está coberto só pelo spawn em z = ∓1 048 500, de pé e com chão.
- A e-fixtures copia o setup e cenas inteiras da base convertida: o pacote tem dados do jogo e fica só em `build/`.
- O enchimento repete triângulos coplanares (32 767 polígonos sobre 15 840 distintos): prova os índices, não o custo
  de uma malha de colisão real desse tamanho (a `wide` da fase E continua sendo a prova de 45 000 polígonos distintos).

## Pendências

- Nota no framework para posição ou `bounds` além de ±1 048 575 (spawn, ator, colisão): hoje não sai aviso nenhum, e
  lá só o chão funciona. Mudança de DLL, sem host.
- Ainda da prioridade 5: M12 (peças de malha tipo 2 acima de 1 024), M15 (plataformas), M16/M17/M19 (exit, câmera e
  água percorridos), M24 (texturas residentes) e R02 (65 535 vértices por peça).

## Próxima ação recomendada

Nota de coordenadas no framework (0.6.4) com teste; depois R02 e M12 com display lists próprias, no mesmo gerador.
