# Handoff — OOT-COREEXT-001

## Estado

review — host OoT ABI 1.1 compilado, ZIPs separados executados no Shipwright
9.2.3 e overlay instalável gerado; sem commit, push ou alteração da instalação
normal do usuário.

## Resultado

- `soh.exe` incorpora uma única vez o loader ABI 1.1 e aceita core extensions
  e consumers como ZIPs independentes na pasta `mods`.
- O core de demonstração publicou um serviço C e carregou antes do consumer.
- O consumer consultou esse serviço e validou o retorno `42` no entrypoint Lua.
- O shutdown descarregou primeiro o consumer e depois a core.
- O processo já aberto em `RELEASE-UNIFICADA\OOT` não foi encerrado ou alterado.

## Build e testes

- Host Release Windows x64: aprovado com MSBuild e `LinkToolPath` x64.
- `soh.exe`: SHA-256
  `516ADCEFAE6F09E852AC5647988FEDC54A6276B265811A8A6919596549FFA05F`.
- `oot_native_engine_tests`: aprovado após recompilação contra o SDK atual.
- Prova de mod independente: dois providers modificaram Player e o hash do host
  permaneceu inalterado.
- `git diff --check`: aprovado no core e no host.

## Playtest isolado

- Instalação:
  `OOT-NATIVE-001/build/official-9.2.3/fresh-coreext-abi11-20260912-0859`.
- Sessão: `valid: true`, com executável, `oot.o2r` e os dois ZIPs inalterados.
- Log prova ABI 1.1, core antes do consumer, dois mods carregados e unload em
  ordem inversa.
- Evidências:
  `OOT-NATIVE-001/build/runtime-evidence/coreext-abi11-20260912-0859/`.

## Artefatos

- Overlay:
  `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-CoreExtensions-ABI1.1-Win64-overlay.zip`
  — SHA-256
  `7C91F2A7E163CBADBED18976A9364EAA573347320CA724760537D574CD273A89`.
- Core demo:
  `OOT-NATIVE-001/build/coreext-packages-v1/linkspan-core-service-demo-1.0.0.zip`
  — SHA-256
  `93070F302B93FF93513A697B74DCF0A8C99CE231A3F1CC1DF1468A4CED950A24`.
- Consumer demo:
  `OOT-NATIVE-001/build/coreext-packages-v1/linkspan-core-consumer-demo-1.0.0.zip`
  — SHA-256
  `74155634029EC74B328A7A36BD2F407FF956A3D18C62E0A8B85BD0E8CC76779D`.
- O overlay contém `soh.exe`, `soh.o2r`, metadados, checksums e Dynamic
  Movement Remake 0.2.3. Não contém ROM, `oot.o2r`, save ou configuração.
- Todos os cinco hashes internos do overlay foram recalculados sem divergência.

## Pendência seguinte

Implementar os registries de engine exigidos pelos ports reais de Unbound e
Not Enough Items, começando por recursos/arquivos, objetos, atores, itens e UI.
