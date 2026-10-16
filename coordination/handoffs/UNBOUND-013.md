# Handoff — UNBOUND-013

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review: peça de host na rodada W, lado framework, correções da revisão e fixture; prova em jogo no D3D11.

## Resultado

- **Registro de cenas:** a chave `horse` de uma cena JSON aceita `true` ou `pos`/`angle`; campo malformado é
  recusado com nota. O host consulta o registro em vez da lista fixa de cinco cenas, que continuam semeadas.
- **Host:** `Horse_CanSpawn` passa por `LinkSpan_HorseAllowed`; numa cena customizada a Epona nasce parada
  (`LinkSpan_HorseSpawn`, tipo 2, inativa) e só aparece quando a Epona's Song a chama, como no campo vanilla.
- **Cena gravada:** o save guarda o nome estável da cena da Epona (`linkspan.namespaces`), resolvido uma única vez
  em `Horse_InitPlayerHorse` — antes de a lista de cenas com cavalo ser consultada — e consumido em seguida; o id
  em `horseData` manda a partir daí. Carregar outro arquivo limpa o nome pendente antes de ler o novo.
- **Posição da chamada:** `EnHorse_PlaceAtCallPoint` grava `horseData.pos` e `angle` junto com a cena. O vanilla só
  grava a posição ao desmontar, então voltar à cena registrada punha a Epona na posição gravada em outra cena, fora
  do mapa.
- **Divisão do trabalho:** o Codex entregou o recorte do registro e o host inicial (`ub013`). Claude integrou na
  rodada W, corrigiu a ordem de resolução da cena, o consumo do nome pendente e a posição do ponto de chamada, e
  fez a fixture do lab/lab_b.

## Commits

Locais, no worktree `OOT-UPSTREAM-78DC-001` (nada foi empurrado):

- `f811f71d6` — rodada de host da fase W (peça de host da UNBOUND-013).
- `11d4afcbb` — lado framework das UNBOUND-013 a 015.
- `cd6691cef` — correções da revisão (ordem da resolução, nome pendente consumido, posição do ponto de chamada).
- `2a9ce1965` — fixture: lab_b, materiais do lab e o comando da Epona ao lado.

## Arquivos alterados

- soh/soh/native/OotNativeScenes.cpp, soh/soh/native/OotNativeScenes.h
- soh/soh/native/OotNativeSaveGame.cpp
- soh/src/code/z_horse.c
- soh/src/overlays/actors/ovl_En_Horse/z_en_horse.c
- soh/native-sdk/unbound-core/scene_registry.cpp
- soh/native-sdk/unbound-core/e-fixtures/{e_fixtures.cpp,main.lua,make_fixtures.py}

## Validação executada

- `oot_native_engine_tests`, `oot_native_registry_tests`, `oot_native_scenes_tests` (host) e
  `linkspan_unbound_json_tests`, `linkspan_unbound_format_tests`, `linkspan_unbound_converter_tests` (core).
- Jogo: `soh.exe` `43CAD3FC` (layout `ce528052`), cópia `OOT-teste-cenas-20260914`, pacotes
  `LinkSpan-Unbound-Framework-0.4.0-layout-ce528052.shipmod` e `...-E-Fixtures-0.1.0-layout-ce528052.shipmod`.
  Sessões `b`, `g`, `h`, `i`, `k` em `build/oot-upstream-001/evidence/unbound-w/`.

## Resultado da validação

- Testes: todos verdes.
- **Espera no `pos`:** ao chegar em `linkspan_e/lab` (cena 131), `cavalo=0.0,0.0,-120.0` — o `pos` registrado.
- **Chamada:** com a ocarina e a Epona's Song a égua aparece e passa a andar pela cena (`h`, `k`).
- **Transição montada (sessão `k`):** `montado=1` dentro do lab, e a saída física leva a `linkspan_e/lab_b`
  (cena 132) ainda com `montado=1` e a Epona ao lado (`cavalo=0.0,-0.0,263.4`, não o `pos` registrado); a captura
  `k-lab_b.png` mostra o Link sobre a Epona com o HUD de cenoura.
- **Sem `horse`:** nas demais cenas da fixture nenhuma Epona nasce.
- Save `file1.sav` restaurado (`1788F5DD`); `shipofharkinian.json` restaurado (`75D9DF11`).

## Riscos

- A Epona nasce inativa: numa cena customizada ela só existe depois da Epona's Song, como no campo. Uma cena que
  queira a égua já visível precisa de outra semente.
- O nome estável da cena é resolvido uma vez; se o mod que registrou a cena sumir entre dois saves, a Epona volta
  para a cena vanilla mais próxima do id gravado.
- A montaria automatizada depende do comando de teste `horse_here` da fixture (a égua anda sozinha). Isso é da
  fixture, não do host.

## Pendências

- Nenhuma dentro do recorte.

## Próxima ação recomendada

Review conjunta da rodada W (com UNBOUND-014, UNBOUND-015, CEL-002 e CEL-003).

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 11d4afcbb e outros na integração; RC3–RC9 (já no RC2 local). Coordenação/RFC na main.
- Core: commits na main que citam a tarefa: 92d5174.
- Host: 11d4afcbb 2a9ce1965 cd6691cef f811f71d6 (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
