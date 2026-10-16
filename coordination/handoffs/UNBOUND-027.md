# Handoff — UNBOUND-027

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

Em review. Commit local no host (`517b0ac16`), sem push.

## Resultado

M24 da matriz de limites do Prelude (prioridade 5, a última linha), provado em jogo na e-fixtures 0.1.7, sem mudar o
host. Com ela a prioridade 5 fecha.

O cache de texturas do Fast3D (`TEXTURE_CACHE_MAX_SIZE` 8 192 no SoH [Unbound], antes 1 024) usa como chave o
endereço, as paletas e fmt/siz/palette_index/size_bytes. Um `LoadTile` 1×1 de um atlas RGBA16 128×128 dá um
endereço por pixel, então um atlas só (`textures/linkspan_e/m24_atlas`) gera as 8 193 chaves dentro do limite do
pacote (1 024 entradas, 200:1).

| Cena | O que desenha | Em jogo (DirectX 11) |
|---|---|---|
| `m24_seq` | 8 193 células 3×3 numa grade de 128 colunas, uma janela do atlas por célula | **todas as 8 193 células com a cor do atlas**, nas duas capturas (3 s entre elas); a primeira verde, a última vermelha |
| `m24_slots` | A (pixel 0, verde) no slot 0 e no painel de referência; 8 192 chaves no slot 1 (faixa de cima); o painel de resultado volta ao slot 0 sem nova carga | **resultado vermelho**, referência verde, faixa vermelha (a última chave, B), estável entre os quadros |

- **O que o `m24_slots` mostra:** a 8 193ª chave (B) expulsa a LRU, que é A. `TextureCacheLookup`
  (`libultraship/src/fast/interpreter.cpp:512-536`) devolve o ID de A à lista livre, anula `mTextures[0]` e
  reaproveita esse ID na mesma hora para B. O slot 0 da GPU continua ligado ao ID, que agora tem a imagem de B. O
  draw final no slot 0 não faz nova consulta e sai vermelho. É o reuso de ID previsto pelo Codex no OpenGL, e ele
  também acontece no DirectX 11. Só aparece com mais de 8 192 chaves num quadro e um slot mantido durante a troca;
  o jogo vanilla não chega perto disso. Vai para a rodada de host da RFC 0027.
- **Orientação:** a câmera olha para +z, então o x positivo do mundo aparece à esquerda da tela. O resultado
  (x 20..190) é o painel da esquerda nas capturas, e a referência (x −190..−20) é o da direita. A conferência da
  `m24_seq` confirma isso: o canto do mundo (−192, 215) cai em cima e à direita.

Desenho e gerador do Codex (`codex-m24-design`, gpt-6.1-sol alto: análise do cache no libultraship, harness do
cache e da DL, previsão de vermelho pelo reuso imediato do ID da LRU). Integração minha sobre a 0.1.6 da
UNBOUND-026 (os hunks conflitavam com a `m15`), mais o roteiro de prova e a conferência por célula.

## Revisão

Revisão própria do diff (só fixture; o framework e o host não mudam). Depois da prova, só os comentários de
orientação dos painéis mudaram (`main.lua` e `make_fixtures.py`). O pacote provado (`85E5D937`) tem o comentário
antigo do `main.lua`, e o código é o mesmo.

## Commits

- Host `517b0ac16` (branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, sobre `e86e643ee`), mensagem
  "UNBOUND-027: M24 em jogo (cache de texturas)".

## Arquivos alterados

- `soh/native-sdk/unbound-core/e-fixtures/make_fixtures.py`: `make_m24` e helpers (`m24_texture`, `m24_tile`,
  `m24_load`, `m24_combine`, `m24_quad`, `m24_draw`).
- `soh/native-sdk/unbound-core/e-fixtures/main.lua`: `m24_seq` e `m24_slots` no roteiro.
- `tools/package-unbound-e-fixtures.ps1`: 0.1.7.

## Validação executada

- E-fixtures 0.1.7 `85E5D937` (19 cenas, 137 entradas, razão máxima 48,6; tem dados do jogo, só em build/).
  Framework 0.6.6 `8E302656`.
- Prova em jogo (`ub027-proof.ps1`, derivado do `ub026-proof.ps1`, mais duas viagens com K e duas capturas por
  cena da `m24`).
- Conferência por célula (`check_m24_seq2.py`): homografia dos quatro cantos da grade, refinada por busca de ±2 px
  nos cantos; cor no centro de cada célula comparada com o atlas gerado pela mesma semente.
- Roteiros em `evidence/unbound-027/scripts/`.

## Resultado da validação

Sessão `ub027-20261003-165519` (framework 0.6.6 + e-fixtures 0.1.7): falhas 0, sessão válida, exit 0. As regressões
da 023/024/026 (M16, R02, M12, M17, M19, M15) continuam passando na mesma sessão.

- `m24_seq`: 8 193/8 193 centros certos em `10-m24-seq-a.png` e `-b.png`.
- `m24_slots`: nas duas capturas, resultado (255,0,0), referência (0,255,0), faixa (255,0,0).

## Riscos

- Só no backend DirectX 11 desta máquina. OpenGL foi previsto pelo harness do Codex, não rodado.
- O tempo de quadro com 8 193 cargas não foi medido.

## Pendências

- Rodada de host da RFC 0027: no `TextureCacheLookup`, não reaproveitar na mesma consulta o ID de uma textura ainda
  ligada a um slot (ou religar o slot ao expulsar). Prova pronta: o `m24_slots` tem de mostrar o resultado verde.

## Próxima ação recomendada

1. Fechar a UNBOUND-025 (boot com as notas herdadas da base fora).
2. Prioridade 5 completa: seguir o plano do Gate OoT (família de ROM, playtest do NEI) e juntar os itens de host
   para a RFC 0027.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 517b0ac16 e outros na integração; RC3–RC9. Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: da72d13.
- Host: 517b0ac16 e86e643ee (RC3–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
