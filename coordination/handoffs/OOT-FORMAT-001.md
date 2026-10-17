# Handoff — OOT-FORMAT-001

## Estado

done — BaiterYamato/Shipwright#6 mergeado na integração em `b5ab20d820` (CI 7/7, com o primeiro `clang-format`
verde no fork) e BaiterYamato/link-span#55 em `154659da07`. O RC10 no layout `ee6449e8` foi validado e publicado pela
OOT-RELEASE-002. Fechada com o ok do mantenedor em 17/10/2026.

## Resultado

- clang-format 14.0.6 (wheel `clang-format==14.0.6` do PyPI, venv isolada) passado com o `./run-clang-format.sh` do
  host, partindo da ponta da integração `agent/OOT-UPSTREAM-001-78dc6d970-integration` (`0097e4e518`, RC9).
- 198 arquivos mudam. Só formatação: comparando sem espaços, 169 são idênticos e os outros 29 diferem só em
  comentários refluídos, `// namespace X` no fim de namespace, barras de continuação de macro e literais partidos em
  dois (`"a" "b"`). Nenhum token de código muda.
- O clang-format 14 não é idempotente no `soh/soh/mmaudio/mmseq/MmAudioTypes.h` (continuação de `UNK_TYPE1 unk_E4`):
  só chega ao ponto fixo na segunda passada, que vai num commit próprio. Uma terceira passada do script inteiro não
  muda nada, que é o que o job `clang-format` do CI confere.
- Overlay do fork NEI: o literal partido no `Message_DecodeChecked` do host deixava a adaptação em
  `overlay-extra/z_message_PAL.Message_DecodeChecked.c` velha (318 sobrepostas, 1 conflito, função fora da DLL sem
  erro). A assinatura foi regravada depois de conferir que o host antes e depois é o mesmo código; o overlay voltou a
  319 sobrepostas, 30 resolvidas, 3 adaptadas e 0 conflitos, como no RC9.
- Os headers de `soh/soh/native` entram no layout id do SDK nativo (`ExportSdk.cmake`): o layout muda e todos os mods
  nativos precisam ser recompilados. A tarefa abre o ciclo do RC10.

## Commits

Host (BaiterYamato/Shipwright, branch `agent/OOT-FORMAT-001-clang-format`, PR BaiterYamato/Shipwright#6 na integração):

- `f08c524b3f` — style: clang-format 14 no código do Link-Span em soh/
- `6cac57eb0a` — style: segunda passada do clang-format 14 no MmAudioTypes.h
- `bedff27262` — fix(nei): assinatura do overlay do Message_DecodeChecked depois da formatação

## Arquivos alterados

- 198 C/C++ do conjunto do `run-clang-format.sh` em `soh/` (40 em `soh/soh/native`, 22 em
  `soh/native-sdk/unbound-core`, 36 no `nei-core`, 10 em `soh/src/code`, 22 overlays de `soh/src`, o resto em
  `soh/soh`, `soh/native-sdk/{ww-style,example,mic-ocarina,tests}` e `soh/tests`).
- `soh/native-sdk/nei-core/fork/overlay-extra/z_message_PAL.Message_DecodeChecked.c` (só a assinatura).

## Validação executada

```text
./run-clang-format.sh (CLANG_FORMAT = clang-format 14.0.6), três passadas; a terceira não muda nada
overlay_formatacao.py <host> 0097e4e518: overlay.planejar com o host antes (git) e depois (working tree)
logs/rc-pipeline-oot-next/rc-build.ps1 -Tag oot-format-001-check (Windows 11, MSVC 14.44, Release x64)
```

## Resultado da validação

- Formatação: ponto fixo; o `git diff --exit-code` do CI passaria.
- Overlay do NEI: 319/30/3/0 antes e depois.
- `rc-build.ps1`: BUILD-OK no host `bedff27262` com link-span-examples `19e53f9`. Configure, build Release,
  `soh_symbols`, CTest 68/68, NEI (configure forçado + DLL), Unbound, WW, assets do NEI e as demos crash/shovel/
  unbound do link-span-examples contra o SDK novo; nenhum `FALHOU` nos logs
  (`build/oot-upstream-001/evidence/release-g/oot-format-001-check/`).
- `overlay-report.txt` do configure real do NEI: 319 sobrepostas, 3 adaptadas, 0 conflitos.
- Layout id novo `ee6449e8ff96…` (RC9: `7376240d`); `soh.exe` `0332742A…`.
- Não testado em jogo: formatação não muda código, e o RC10 roda a bateria inteira com os mods reempacotados.

## Pendências

- Resolvidas: o CI do #6 não rodava porque o GitHub tinha suspendido o Actions do fork; reativado pelo mantenedor,
  passou 7/7. O RC10 com todos os mods reversionados é a OOT-RELEASE-002.
- Pré-existente, sem efeito hoje: três resoluções em `overlay-merges/` têm assinatura velha desde antes da formatação
  (`code_800EC960.AudioOcarina_PlaybackSong`, `z_file_choose.FileChoose_UpdateQuestMenu` e
  `z_file_choose.FileChoose_UpdateRandomizerMenu`). Não aparecem como conflito porque a mescla automática dessas
  funções sai limpa; vale conferir se ainda são necessárias.

## Riscos

- O check `clang-format` só roda em PR: a prova final de que ficou verde é o primeiro PR depois desta branch.
- Arquivo novo de C/C++ do Link-Span precisa sair formatado com clang-format 14 (não com o do PATH, que é de outra
  versão), senão o job volta a falhar.

## Próxima ação recomendada

- Ver o job `clang-format` verde no BaiterYamato/Shipwright#6, mergear na integração e começar o RC10 sobre o
  layout `ee6449e8`.
