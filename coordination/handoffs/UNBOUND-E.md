# Handoff — UNBOUND-E (fase E: UNBOUND-002 a 012)

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

Framework `linkspan.unbound.framework` 0.4.0 sobre OOT-CORE-007/008, entregue num `.shipmod` único. Por etapa:

| Etapa | Entregue | Fora |
|---|---|---|
| 002 substrato | Matriz float, vértice s32 e sniffing JSON (CORE-007). Fábrica externa por `register_json_type` (CORE-008): `unbound/scene`, `room`, `collision/3` e `paths` carregam em jogo. | — |
| 003 tipos | Posições f32, colisão s32 e contadores u16 (CORE-007). Em jogo: 602 atores vivos, x=z=45 000, 45 000 polígonos. | Sanitizers não rodados |
| 004 VFS mergeável | Merge em todas as camadas, `null`, `$replace`, `$order` e `$schema`; erro por camada. Testes `json` e `format`. | — |
| 005 fábricas no framework | Transcodificadores JSON→XML na DLL do framework. Sem o pacote, o host usa o loader vanilla. | — |
| 006 conversor e cache | Base convertida no init (101 cenas, 388 salas, 835 arquivos, 226 ms; cache 34 ms). Proveniência em `.source.json`, gravação tmp + rename e CLI `linkspan_unbound_convert` com `--check`. | Invalidação por tamanho e data, não por hash |
| 007 SceneDB e entradas | Registro namespaced com ids automáticos (0x80–0x83), title card, exit por nome e `linkspan_travel` no console. Em jogo: entrar, ler, salvar, sair por exit nomeado e voltar. | Console `linkspan_travel` sem prova em jogo |
| 008 colisão e mundo amplo | Colisão v3 em bulk (22 801 vértices, 45 000 polígonos até ±60 000), fog e draw distance no mundo. Link anda em x=z=45 000. | DynaPoly e água não exercitados |
| 009 contagens | 600 atores numa sala e 602 vivos; coleta funciona. | Salas > 255, objetos > 128, transition actors e mesh entries |
| 010 textos | `text/<lang>/messages.json` acrescenta, troca e remove (`null`); `$replace` esvazia; truncamento acima de 8 192 com nota. Em jogo, a placa mostra a mensagem nova. | Placa com a mensagem trocada só pela sonda da tabela |
| 011 materiais | Todas as receitas transcodificam e carregam. Em jogo, `texCycle` no segmento 8: controle fixo contra alternância dia/noite. | scroll, two-scroll, color, lerp e non-linear sem prova visual (fora do quadro) |
| 012 pacote | `.shipmod` único com DLL, Lua, CLI, header, README e LICENSES. Manifesto validado, SHA-256 e `.contents.json`. | Instalação sobre cópia fresca com overlay; licença do Link-Span |

Correções achadas no playtest:

- **Host (`56193b774`):**
  - as fábricas rodam no pool de threads da libultraship, então o transcode e as leituras do VFS aceitam aquela
    thread, com um transcode por vez;
  - uma cena trocada que falha volta ao vanilla, em vez de entrar em recursão na Dodongo's Cavern.
- **Framework:**
  - a raiz do XML de cena passou a ser `<Room>`, porque o SoH só registra a fábrica XML de cena com esse nome;
  - o texto entra no primeiro frame, porque `OTRMessage_Init` roda depois do `game.ready`.

## Commits

- Host `56193b774` (native: transcode no pool, fallback vanilla e `linkspan_travel`).
- Host `68687c480` (unbound: framework 0.4.0, CLI, fixtures e pacotes).
- Branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`. Sem push.

## Arquivos alterados

- `soh/native-sdk/unbound-core/`:
  - `converter*`, `transcode*`, `unbound_docs*`, `unbound_format*`;
  - `unbound_core.cpp`, `scene_registry*`, `core-main.lua`;
  - testes, `CMakeLists.txt`, `README.md` e `e-fixtures/`.
- `tools/package-unbound-framework.ps1`, `package-unbound-e-fixtures.ps1` e `package-unbound-scene-demo.ps1`.
- `soh/soh/native/OotNativeJsonTypes.cpp`, `OotNativeEngine.cpp` e `OotNativeScenes.cpp`.
- `soh/soh/z_play_otr.cpp`, `soh/soh/ShipLuaBootstrap.cpp` e `soh/native-sdk/ExportSdk.cmake`.

## Validação executada

```text
cmake --build build/unbound-scene-native --config Release ; ctest (json, format, converter: 3/3)
build/x64 Release: soh + oot_native_engine_tests + oot_native_scenes_tests + registry (passam)
linkspan_unbound_convert <oot-extract> <saida.o2r>  -> verificados=734 recusados=0, falhas=0
tools/package-unbound-framework.ps1 ; tools/package-unbound-e-fixtures.ps1 -BaseArchive <oot-unbound.o2r>
Sessões em jogo s1..s9 (evidence/unbound-e-gate), soh.exe 8655C372, layout 31899f1f
```

## Resultado da validação

- **s9, Gate E completo, sem nenhuma linha `[error]` no log.** Capturas `g-01`…`g-14` e `gate-e-sheet.png`:
  - Hyrule Field pela base JSON (`scene=1 room=1 collision=1 paths=4`);
  - casa JSON com o título "LINK-SPAN UNBOUND";
  - placa com a mensagem 0x03FE vinda do JSON;
  - saída pela porta até `ENTR_HYRULE_FIELD_PAST_BRIDGE_SPAWN` (0x0CD);
  - mundo amplo com 600 corações visíveis. Andando, Link coleta 6 (600→594);
  - cavern fixa no dia e cavern_b alternando dia/noite: ~375 mil pixels de diferença entre quadros, contra ~2,6 mil
    do controle;
  - retorno à casa.
- **s4:** save gravado dentro da casa JSON (`Save File` na cena 0x82). **s5:** o save recarrega sem erro, com o
  respawn vanilla em Kakariko.
- Texto: `definidas=2 removidas=1 falhas=0`. A sonda confirma 0x03FE nova, 0x0301 trocada e 0x0345 removida.
- Save da cópia restaurado (`1788F5DD`). Os mods de teste foram para `mods-anteriores/unbound-e-gate-20260918`.

## Limites da fase E (2026-09-19)

Fixtures `lab` e `many` (host `0665b941c`), sessões s2 e s3 em `evidence/unbound-e-limites`, sem `[error]` no log:

- `many`: 300 salas, Link na sala 299, 203 objetos, 100 transition actors, 300 mesh entries e DynaPoly 60/64 com
  16 384 polígonos. Na sala 299 ele nada (`agua=1 nado=1`) e coleta o coração.
- `lab`: as 6 receitas de material animam (scroll, two-scroll, color, lerp, non-linear e `texCycle`), provadas por
  diferença de pixels entre quadros. O two-scroll passou a usar a textura grade, porque a listrada só varia em x.
- `many` reusa a DL do lab: sem as animações de material, o segmento 0x0D ficava sem textura.

## Review conjunta E+F (2026-09-19)

Três revisores: host, framework Unbound e NEI-002. Corrigido nos commits host `660bd3085` e `c0f1245b4`:

| Achado | Gravidade | Correção |
|---|---|---|
| Índice de vértice ou tipo de superfície fora da faixa na colisão lia além do array | alta | `CollisionHeaderFactory` zera o índice e avisa uma vez por recurso |
| JSON profundo estourava a pilha do nlohmann | alta | Recusa acima de 64 níveis antes do parse, em todos os pontos de leitura |
| Mapas do registro de cenas lidos no pool de recursos enquanto a thread do jogo registra | média | `shared_mutex` nos mapas |
| Título de cena menor que 144x24 era lido além do fim | média | Textura conferida uma vez por caminho |
| Iluminação com mais de 255 entradas estourava o contador u8 | média | Corte em 255 com nota |
| Recurso pedido por `LoadResourceAsync` dentro de transcode trava no mutex | baixa | Invariante documentada |

Não corrigido, de baixo impacto: o conjunto estático de caminhos de título cresce com o número de cenas,
`UnregisterOotJsonType` espera o transcode em andamento e o log do framework não tem limite de tamanho.

Regressão depois das correções (`evidence/unbound-ef-review`, s2, `soh.exe` `8C7BC455`, layout `31899f1f`): as 7
viagens das fixtures (house, wide, cavern, cavern_b, lab, many e house) sem erro, com as texturas do lab certas.
O `soh.symbols` passou a trazer 17 756 variáveis como `[data]`, para o escape hatch da fase F.

## Pendências

- Comparação com a release de referência do Unbound 0.6: não disponível.
- Licença do repositório Link-Span para o `LICENSES.md` do pacote. Decisão do usuário.
- Comentário de thread em `oot_resources.h`: corrigir na próxima mudança de layout.
- Instalar o `.shipmod` único numa cópia fresca com overlay (aceite 012).
- Cache da base por tamanho e data: trocar por hash se aparecer caso de arquivo alterado com a mesma data.
- Delta do Unbound de 2026-09-19 (`0c31899`): UNBOUND-013 a 015 no plano, na fase W.

## Riscos

- A base troca todas as 101 cenas vanilla pelo JSON. Um documento recusado cai no vanilla com log, mas um erro de
  transcodificação sem recusa (XML válido com semântica errada) só aparece em jogo.
- A fixture tem dados do jogo gerados da base local. Fica em `build/` e não deve ser publicada.

## Próxima ação recomendada

- Aceite da fase E pelo usuário. Os limites e a review estão fechados; o que resta é instalação limpa, licença e o
  delta do Unbound na fase W.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 56193b774 e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 6f2ec3b.
- Host: 56193b774 68687c480 (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: licença de terceiros (Unbound) não registrada; CC0 vale só para o código próprio (09/10).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
