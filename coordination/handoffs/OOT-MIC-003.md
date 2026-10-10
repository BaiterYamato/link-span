# Handoff — OOT-MIC-003

## Estado

review — o Mic Ocarina 0.1.4 está compilado, testado ROM-free, empacotado e instalado na pasta de teste, com boot check OK. Falta o reteste com microfone. O 0.1.3 foi validado com microfone real às 06:20 de 14/09 (Saria's Song reconhecida).

## Resultado

- **Sintoma relatado em 14/09:** no host develop (`OOT-UPSTREAM-001`), o cantarolar não produzia as notas em sequência e o reconhecimento quase não acontecia.
- **Investigação:**
  - O Mic 0.1.2 (WIP do Codex, OOT-MIC-002) é o mesmo código nas DLLs `bceacd08` (host antigo) e `16ff0d5a` (host develop):
    - o `provider.cpp` é de 13/09 14:29, antes dos dois builds, e as strings das DLLs são idênticas;
    - o ZIP "funcionando" que o usuário mandou dos Downloads é, byte a byte, o da `RELEASE-UNIFICADA`.
  - No host develop, o serviço `linkspan.oot.ocarina` é equivalente ao antigo:
    - `OCARINA_BTN_*` e `OCARINA_NOTE_*` têm os mesmos valores;
    - o gancho em `AudioOcarina_Update` é equivalente;
    - `fontTbl` e o texto do HUD não mudaram, e as capturas mostram o HUD com texto.
  - A build de diagnóstico (`build/oot-upstream-001/mic-diag`), no teste das 05:54, mostrou:
    - **o que estava certo:** headset Corsair a 48 kHz, 0 overruns, flags `0x07FF` e os 12 padrões corretos;
    - **Zelda's Lullaby:** os dois Lá saíram partidos em duas notas (`-1992,-1744,-2277,-2200,-1995,-1740,-2374,-2322` cents), e a melhor candidata ficou a 211–239 cents, acima dos 120 exigidos;
    - **Saria's Song:** as tentativas foram cortadas por pausas de cerca de 1 s (reancoragem).
  - O protótipo `MicOcarina.cpp` (2Ship/MM, `MicOcarina base.rar` do usuário) tinha duas partes que o port para o OoT removeu:
    - a junção de notas partidas como hipótese alternativa (`BuildMergedPhrase` e `SuffixError`);
    - o rolo de notas (janela ImGui `MicOcarinaMeterWindow`).
- **Correção (Mic Ocarina 0.1.3), só no mod, sem recompilar o host:**
  - `pitch_matcher`: `BuildMergedPhrase` e `kContourDeadbandCents = 90`. O `RankSuffixes` testa a frase como foi segmentada e uma janela duas notas mais larga, com as notas partidas juntadas.
  - `provider.cpp`: âncora de tom na primeira nota aceita e histórico do rolo (24 notas, janela de 6 s), enviados ao Lua em `anchor=` e `roll=`.
  - `main.lua`: rolo de notas abaixo da tarja, no lugar da onda.
    - A barra fica branca até a nota contar e verde quando ela entra na frase.
    - Sem tom marcado, aparece a dica "SEGURE UMA NOTA PARA MARCAR O TOM".
  - `tests.cpp`: a frase real da Zelda's Lullaby, com uma e com duas notas partidas, e a repetição legítima da Serenade.
  - `manifest.toml` passou para 0.1.3, e o `README.md` descreve o rolo e a pausa que encerra a frase.
- **Retorno do usuário com o 0.1.3 (14/09):** F6 aparecia como F5, e D6, E6, G6 e A6 também saíam na oitava 5. C6 e as notas abaixo saíam certas.
- **Causa:** o detector YIN, igual ao do protótipo, só procurava períodos a partir de 48 amostras, um teto de 1000 Hz.
  - O período de D6 (40,9 amostras) e das notas acima ficava fora da busca, e o primeiro vale aceito era o dobro dele: uma oitava abaixo.
  - O C6 (45,9 amostras) escapava por acaso: a interpolação parabólica, sem limite, extrapolava até o vale fora da faixa.
- **Correção (Mic Ocarina 0.1.4), só no mod:**
  - `pitch_matcher.h/.cpp`: o detector saiu do `provider.cpp` para `MicOcarina::EstimatePitch`, que dá para testar sem microfone.
    - O teto passou para 2400 Hz (`kTauMin = 20` amostras), acima do C7.
    - O ajuste parabólico fica limitado a ±1 amostra.
    - Se o vale fica antes do início da faixa, o detector não devolve nota.
  - `provider.cpp` usa o detector compartilhado.
  - `tests.cpp`: assobio sintético de C6 a C7 e voz com harmônicos de 110 a 587 Hz.
  - `manifest.toml` passou para 0.1.4, e o `README.md` cita a faixa de 80 a 2400 Hz.
- **Pacotes:**
  - 0.1.3: `OOT-UPSTREAM-78DC-001/build/distribution/Mic-Ocarina-0.1.3-layout-16ff0d5a.zip`, SHA-256 `A87FA5221EA1EBFF45AE78A4EB9AE5B51237D68935457CAC9A42BD790D430B1C`. Ficou em `RELEASE-UNIFICADA/OOT-teste-save-20260913/mods` até 14/09 10:45 e agora está em `mods-originais`, com o 0.1.2 original e o de diagnóstico;
  - 0.1.4: `OOT-UPSTREAM-78DC-001/build/distribution/Mic-Ocarina-0.1.4-layout-16ff0d5a.zip`, SHA-256 `8E5360BAB8FA0CDAB577E7385F57ED5FF1B84EEBB2D3C81BD6D106F1D11E2027`, instalado na mesma pasta às 10:45, junto com o host e o DMR do OOT-MOVE-006.

## Commits

- `7a8701d82` em `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`, só local, sem push. Inclui o WIP 0.1.2 do Codex (OOT-MIC-002), base das correções; o backup do WIP continua em `build/oot-upstream-001/mic-ocarina-0.1.2-wip`.

## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/provider.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/main.lua`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/pitch_matcher.h`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/pitch_matcher.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/tests.cpp`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/manifest.toml`
- `OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/README.md`
- `coordination/claims/OOT-MIC-003.md` e este handoff

## Validação executada

```text
cmake -S soh/native-sdk/mic-ocarina -B build/mic-ocarina-0.1.3 -G "Visual Studio 17 2022" -A x64 -DOOT_NATIVE_SDK=build/x64/native-sdk/Release/OotNativeSdk.cmake -DSDL2_DIR=<sdl2 estático do vcpkg>
cmake --build build/mic-ocarina-0.1.3 --config Release
ctest --test-dir build/mic-ocarina-0.1.3 -C Release --output-on-failure
powershell.exe -File tools/package-linkspan-mod.ps1 -Source build/mic-ocarina-0.1.3/mod -OutputPath build/distribution/Mic-Ocarina-0.1.3-layout-16ff0d5a.zip
shiplua_manifest_validator.exe build/distribution/Mic-Ocarina-0.1.3-layout-16ff0d5a.zip
prepare-runtime-session.ps1 + boot de 45 s na pasta de teste + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict

cmake -S soh/native-sdk/mic-ocarina -B build/mic-ocarina-0.1.4 (mesmos argumentos)
cmake --build build/mic-ocarina-0.1.4 --config Release --target mic_ocarina_tests   # prova vermelha, com kTauMin = kSampleRate / 1000
build/mic-ocarina-0.1.4/Release/mic_ocarina_tests.exe
cmake --build build/mic-ocarina-0.1.4 --config Release                              # kTauMin = kSampleRate / 2400
ctest --test-dir build/mic-ocarina-0.1.4 -C Release --output-on-failure
powershell.exe -File tools/package-linkspan-mod.ps1 -Source build/mic-ocarina-0.1.4/mod -OutputPath build/distribution/Mic-Ocarina-0.1.4-layout-16ff0d5a.zip
shiplua_manifest_validator.exe build/distribution/Mic-Ocarina-0.1.4-layout-16ff0d5a.zip
prepare-runtime-session.ps1 + boot de 45 s na pasta de teste + finish-runtime-session.ps1 -RequireTrackedUnchanged -Strict   # host 137C907E do OOT-MOVE-006
```

## Resultado da validação

- **0.1.3, build e testes:** build sem erro; ctest 1/1 (`mic_ocarina_matcher`), com os três casos novos.
- **0.1.3, validador:** id `baiteryamato.mic_ocarina`, versão 0.1.3.
- **0.1.3, boot** (`evidence/mic-013-boot-20260914-0611`, sessão válida):
  - `loaded mod 'Mic Ocarina' v0.1.3 [native]`, `carregou 2 mod(s)` e `mic-ocarina: pronto`;
  - vivo aos 45 s e fechado com exit 0;
  - saves, `.o2r` e mods inalterados.
- **Evidência do diagnóstico:** capturas do HUD em `evidence/mic-diag-hud-20260914-0552` e o log da pasta de teste das 05:54–05:55.
- **Teste do usuário com o 0.1.3** (log da pasta de teste; capturas em `evidence/mic-013-hud-20260914-0612`):
  - `audio-input-on` às 06:20:21 e `música reconhecida: 6` (Saria's Song) às 06:20:42;
  - o jogo tocou a música com as notas em sequência na pauta;
  - as capturas mostram a dica "SEGURE UMA NOTA PARA MARCAR O TOM" antes do tom e barras verdes das notas aceitas no rolo.
- **0.1.4, prova vermelha com o teto antigo:**
  - D6 → 587,36 Hz, E6 → 659,30, F6 → 698,50, G6 → 784,09, A6 → 880,05 e B6 → 987,85: todos a −1200 cents, o mesmo erro de oitava do relato;
  - C7 → 697,72 Hz (−1902 cents);
  - C6 sem nota, pela guarda nova de vale fora da faixa (o código antigo acertava o C6 com a extrapolação).
- **0.1.4 com o teto novo:**
  - C6 a C7 com erro de no máximo +1,5 cent;
  - A2, D3, A3, A4 e D5 com cinco harmônicos a 0,0 cent, sem salto de oitava;
  - ctest 1/1; validador com id `baiteryamato.mic_ocarina`, versão 0.1.4.
- **0.1.4, boot** (`evidence/move006-mic014-boot-20260914-1045`, sessão válida, host `137C907E` do OOT-MOVE-006):
  - `loaded mod 'Mic Ocarina' v0.1.4 [native]`, `carregou 2 mod(s)` e `mic-ocarina: pronto`;
  - vivo aos 45 s e fechado com exit 0;
  - saves, `.o2r` e mods inalterados.

## Pendências


- Reteste do usuário: notas da oitava 6 e músicas, cantaroladas e assobiadas.
- Push do commit, só quando o usuário pedir publicação.
- Levar a mesma correção para a `OOT-NATIVE-001`, se o host antigo continuar em uso, e para a `RELEASE-UNIFICADA/OOT`, onde o usuário atualiza os mods sozinho.

## Riscos

- A reancoragem depois de cerca de 1 s de silêncio é igual à do protótipo. Pausas longas entre notas recomeçam a frase; agora isso aparece no rolo.
- A junção é uma hipótese alternativa avaliada por música. Pode aproximar músicas de contorno parecido, mas o erro máximo de 120 cents e a margem de 40 cents continuam valendo.
- O rolo acrescenta até cerca de 30 retângulos por frame ao orçamento de 400 do HUD.
- Com o teto de 2400 Hz, um chiado agudo e periódico pode virar nota. O limiar do YIN, a clareza mínima de 0,55, o gate de ruído e o mínimo de 9 hops por nota continuam filtrando.

## Próxima ação recomendada

- Com o reteste do usuário (notas da oitava 6 e músicas), levar o 0.1.4 para a `RELEASE-UNIFICADA/OOT` junto com o host do OOT-MOVE-006 e decidir a `OOT-NATIVE-001`.
