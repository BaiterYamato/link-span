# Handoff — OOT-UPSTREAM-002

## Estado

Concluída: RC7 validado e não publicado (decisão do mantenedor em 11/10/2026; o rascunho fica no
BaiterYamato/Shipwright). PRs #1 (`19829d981d`), #2 (`783e5605b8`) e link-span-examples#1 (`feb389657e`)
mergeados com exceção do `clang-format`. O candidato publicado é o RC8 (OOT-UPSTREAM-003).
- Host: branch `agent/OOT-UPSTREAM-002-soh-930`, PR [BaiterYamato/Shipwright#2](https://github.com/BaiterYamato/Shipwright/pull/2)
  (rascunho), empilhado sobre o #1 (UNBOUND-032).
- Exemplos: branch `agent/OOT-UPSTREAM-002-rc7` (`9b21bcc`), PR
  [BaiterYamato/link-span-examples#1](https://github.com/BaiterYamato/link-span-examples/pull/1).
- RC7 `oot-0.5.0-rc7` montado em `build/release/oot-0.5.0-rc7/` e aprovado na bateria e nos 5 gates em 11/10/2026.
  Não publicado.

## Resultado

- Merge da tag Shipwright 9.3.0 (`ecd889c201`, "Dewey Alpha") sobre a develop `78dc6d970` com o Link-Span: 74 commits
  upstream, 32 arquivos em conflito resolvidos (37 trechos, 28 deles includes).
- Mantidos no merge: mutex de áudio/savestate com instância `inline` em `OTRAudio.h` (o 9.3.0 reverteu o self-pumping),
  hook `oot.save.saving` antes da cópia no `SaveManager`, `LoadFile` com `lock_guard`, conferência de tamanho no
  savestate e `DeinitOTR` esperando a geração do randomizer antes de descarregar os mods.
- Adaptações: `SaveFunc` com `const SaveContext&`, bootstrap com `SaveManager::SaveFile_Exist`, `CutsceneTime.cpp`
  resolvendo cenas registradas sem passar de `gSceneTable`.
- Regressão do 9.3.0 corrigida: Quick Bongo Kill (o hookify #7200 tinha mudado o comportamento com a opção ligada).
- Fork NEI no 9.3.0: 0 conflitos, 319 funções sobrepostas; `z_door_ana.c` ganhou os includes do GameInteractor e duas
  resoluções manuais vieram da UNBOUND-032 (`KaleidoScope_DrawInfoPanel`, `Message_DecodeChecked`).
- Unbound Framework grava `baseVersion`: a base convertida é reaproveitada quando só o build do host muda e recusada
  quando foi convertida por um framework mais novo.
- Layout `9a593a6e` → `7376240d`. NEI 0.3.22, Unbound Framework 0.6.12, DMR 0.2.21, Mic Ocarina 0.2.7, WW 0.1.3 e
  Shovel Demo 0.1.3 acompanham; NEI, DMR e Mic fixam o SHA-256 do `soh.exe`.
- `soh.exe` compilado de `17aa340e4e`, SHA-256 `8ab3cb309ddc21033ced605898c36ef4bd6019f93c8580fa88218b798e70dfbd`.
  Os commits seguintes (docs, CI, whitespace) não mudam o executável distribuído.
- Notas do RC7 corrigidas depois da bateria ("Two" → "Three" spawns); SHA256SUMS, inventário e varredura de conteúdo
  protegido regravados. Nenhum pacote mudou.

## Commits

- `8bb9b78591`: merge do SoH 9.3.0, adaptações e Quick Bongo Kill.
- `b24445ed54`: versões dos mods do RC7.
- `5e0b7f4e17`: fork NEI sobre o host 9.3.0.
- `c1e7b80470`: `baseVersion` da base Unbound.
- `17aa340e4e`: scripts de pacote para o 9.3.0 (fonte do executável distribuído).
- `d01e63fa0c`: documentação do host para o 9.3.0.
- `c442d28aef`: merge do `9def862628` (CI da UNBOUND-032).
- `771958503d`: espaços no fim da linha em `MmAudioTypes.h` (check-whitespace do 9.3.0).
- Exemplos `9b21bcc`: Shovel Demo 0.1.3.
- Core `4766532`; libultraship `d8d5d9a` (inalterada).

## Arquivos alterados

- Merge: `soh/soh/{OTRGlobals.cpp,OTRAudio.h,SaveManager.cpp,ShipLuaBootstrap.cpp}`, `soh/soh/Enhancements/savestates.cpp`,
  `soh/soh/native/OotNativeSaveGame.cpp`, `soh/soh/unbound/SceneFlagsExt.cpp`, headers de `soh/include/` e includes de
  atores.
- `soh/soh/Enhancements/TimeSavers/SkipCutscene/CutsceneTime.cpp`, `soh/src/overlays/actors/ovl_Boss_Sst/z_boss_sst.c`.
- `soh/native-sdk/nei-core/fork/` (`extract.txt`, `overlay-merges/`, `overlay-extra/`).
- `soh/native-sdk/unbound-core/` (`converter.{cpp,h}`, testes, `unbound_core.cpp`, `unbound_docs.{cpp,h}`, READMEs).
- Manifests e READMEs dos mods; `tools/package-*.ps1`, `tools/build-linkspan-{release,sdk-bundle}.ps1`.
- `NOTICE-LINKSPAN.md`, `docs/install-linkspan-overlay*.md`, `docs/licensing/{OVERLAY-NOTICE,THIRD-PARTY-NOTICES}.md`.
- Pipeline fora do git do host: `logs/rc-pipeline-oot-next/` (bateria parametrizada para RC7/RC6, base oficial 9.3.0,
  fixtures por layout, M2 recompilado, `matrix_check.py`, `vanilla001-boot.ps1`, `rc7-continuacao.ps1`); detalhes em
  `notes/BATTERY-CHANGES.md`.

## Validação executada

- Build Release, `soh_symbols`, CTest do host 68/68, NEI 6/6, Unbound 7/7; `rc-post-build` (reprodutibilidade, 10
  arquivos conferidos, scanner OK).
- Bateria do RC7 sobre o Shipwright 9.3.0 oficial (11/10/2026, 14:15–18:17), com as partes que falharam por motivo
  externo repetidas (`rc7-continuacao.ps1`) e os gates com controle conectado.
- Auditoria (Codex) dos 74 commits do 9.3.0 contra hooks e serviços nativos do Link-Span; comparação (Codex) dos
  pacotes RC6 → RC7. Material em `logs/rc8-preparo-2026-10-11/`.
- Evidência: `build/oot-upstream-001/evidence/release-g/rc7/` no host (`RESUMO-FINAL.txt`, `gates/`).

## Resultado da validação

- Instalação fresca: PROVA OK (0 mods, 371 comandos de cena XML).
- Sem mods (cópia fresca): 0 falhas, 0 erros no log, seções `linkspan` e `unbound` do save iguais.
- Matriz 9/9 (`01-none` na repetição; 02–09 na primeira passada).
- Ciclo de save: retirar e reinstalar o NEI sem perder dados.
- Atualização NEI 0.3.21 → 0.3.22: ATUALIZAÇÃO OK (repetição).
- Fixtures: pacote 0.1.12 e 9 cenários (completa, boot, savestate, m18, a8200, r04, r0809, água, luzes) com 0 falhas.
- Sonda do SDK: carregada e descarregada; autocontenção: 37 referências, 0 ausentes.
- Gates 5/5: add-ons, capability, recuperação de boot, seletor de pausa e rollback RC7 → RC6 → RC7.
- Primeira passada: sem mods, `01-none`, atualização e fixtures falharam por causa do harness ou de foco perdido, não do
  candidato. Correções em `BATTERY-CHANGES.md`: placeholder do 9.3.0 em `mods/`, stderr do checker no PS 5.1, autosave do
  reset antes da linha do comando.
- Pacotes RC6 → RC7: nenhuma entrada nova ou removida; 10/10 hashes conferem; conteúdo protegido OK.
- CI do PR #2: builds, OTR, JSON e whitespace verdes; `clang-format` falha por código antigo (ver UNBOUND-032).

## Pendências

- Resolvidas depois: merges com exceção do `clang-format`, RC7 não publicado, docs do core direto para o RC8.
- RC8 (`logs/rc8-preparo-2026-10-11/`):
  - `set_setting_int` e a API Lua de máscaras gravam CVar do SoH sem `ShipInit::Init`, e os hooks `COND_*` ficam com o
    valor do boot;
  - o skip da intro do Ganon chama `actor->update` fora do `oot.actor.update`.
  Os dois têm patch rascunhado.
- OOT-FORMAT-001 no começo do próximo ciclo (muda o layout id).
- Prova física de controle/câmera do DMR e microfone do Mic Ocarina.

## Riscos

- RC7 exige Shipwright 9.3.0 limpo; overlay e mods do RC6 não servem no layout `7376240d`.
- Com o DMR, descarregar o mod no meio da sessão deixa a primeira pessoa ligada até reiniciar, e o fix da Epona do 9.3.0
  (#7270) fica inativo quando o DMR liga o FreeLook depois do boot (não é regressão em relação ao RC6).
- Mod que use REPLACE em `ACTOR_DEMO_GJ` não controla os dois passos do skip da intro do Ganon.
- Áudio segue o 9.3.0 oficial, sem self-pumping; sem medição de desempenho sob carga.
- Mods nativos rodam no processo do jogo, sem sandbox; caminhos longos podem estourar MAX_PATH.

## Próxima ação recomendada

- Nenhuma nesta tarefa: merges feitos e RC7 substituído pelo RC8; o que resta do 9.3.0 segue na OOT-UPSTREAM-003.
