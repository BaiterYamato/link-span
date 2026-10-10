# Handoff — MM-NATIVE-001

## Estado

review — integração, build Windows, prova independente e playtest real concluídos;
sem commit, push ou alteração da instalação normal.

## Resultado

- O 2Ship2Harkinian usa o SDK Link-Span 0.5 por override CMake, preservando o
  submódulo gravado, e carrega providers em pasta, `.zip` ou `.shipmod`.
- O host publica `linkspan.mm.engine` v1 com PlayState, Player, SaveContext,
  spawn e kill. A mecânica e seus nomes ficam inteiramente dentro da DLL do mod.
- O SDK local exporta headers, defines e fingerprint; layout incompatível é
  recusado antes de converter ponteiros.
- O exemplo registra J via Lua e altera `player->actor.velocity.y` na DLL, sem
  linkar o mod a shiplua ou ao executável.

## Arquivos alterados

- Host MM: `CMakeLists.txt`, `mm/2s2h/ShipLuaBootstrap.cpp`,
  `mm/2s2h/native/**`, `mm/native-sdk/**`, `docs/native-mods.md`.
- Core: RFC 0013, claim e este handoff. O restante de NATIVE-001 pertence ao
  incremento base e às integrações anteriores.

## Commits

Nenhum. Commit e push não foram autorizados.

## Validação executada

- MSVC 19.44, Visual Studio 2022 x64, Release.
- `mm_native_engine_tests`: passou; validou ausência de gameplay, thread,
  ponteiros reais, SaveContext, spawn/kill, troca de cena e layout incompatível.
- Build completo: `x64/Release/2ship.exe`, SHA-256
  `220313389b82ce6cdfb598e70e5527ebbcab70eb4c392c4367a0b36661dc0f8f`.
- Providers separados de impulso 10 e 18 modificaram o Player; o executável
  manteve o mesmo hash. Pacotes em `build/native-mm/native-packages/`.
- `native-jump-18.zip`: SHA-256
  `7ae2ddaa7b50b5b68ff2dff3b0da087b160e68c3d430441934e4479354bf03e8`.
- Playtest isolado: o ZIP carregou como `[native]`, J produziu
  `velocidade Y -4.00 -> 18.00`, a captura mostrou Link elevado, e unload e
  shutdown foram normais. Executável, ZIP e cópia dos saves ficaram inalterados.
- O resultado do gate de runtime é `valid: true`, sem achados.
- `git diff --check` aprovado.

## Evidências

- `MM-NATIVE-001/build/native-mm-host-build.log`
- `MM-NATIVE-001/build/native-mm-independent-proof.log`
- `MM-NATIVE-001/build/native-runtime/runtime-evidence/2 Ship 2 Harkinian.mm-native-real-001.post.log`
- `MM-NATIVE-001/build/native-runtime/runtime-evidence/native-jump-early.png`
- `MM-NATIVE-001/build/native-runtime/runtime-evidence/native-jump-air.png`
- `MM-NATIVE-001/build/native-runtime/runtime-evidence/native-jump-later.png`
- `MM-NATIVE-001/build/native-runtime/runtime-evidence/runtime-session-mm-native-real-001.result.json`

## Resultado da validação

Build do host, bridge, providers independentes e playtest real estão verdes.
O host normal em `RELEASE-UNIFICADA/MM` permaneceu intocado.

## Riscos

- DLL nativa pode escrever na memória do processo e derrubar o jogo.
- Ponteiros não sobrevivem a cenas/frames e só podem ser usados na thread do jogo.
- O fingerprint detecta divergência conhecida, mas não estabiliza layouts internos.
- A configuração recebeu HTTP 503 ao baixar `gamecontrollerdb.txt`; isso gerou
  apenas o erro já conhecido de mapping SDL no runtime e não afetou o provider.

## Pendências

- Integrar/commitir o core e atualizar os submódulos quando houver autorização.
- Tornar os SDKs de headers relocáveis para distribuição aos autores.
- Ampliar as tabelas de serviços por versão quando mods reais precisarem chamar
  mais rotinas internas, sem adicionar funções específicas de cada mod ao loader.

## Próxima ação recomendada

Empacotar um SDK relocável comum com os headers de ABI e kits separados de OoT
e MM, para que autores compilem providers sem apontar para a árvore fonte do host.
