# Handoff — OOT-COREEXT-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — host OoT ABI 1.1 compilado, ZIPs separados executados no Shipwright
9.2.3 e overlay instalável gerado, sem alterar a instalação normal do usuário.

## Resultado

- `soh.exe` incorpora uma única vez o loader ABI 1.1 e aceita core extensions
  e consumers como ZIPs independentes na pasta `mods`.
- O core de demonstração publicou um serviço C e carregou antes do consumer.
- O consumer consultou esse serviço e validou o retorno `42` no entrypoint Lua.
- O shutdown descarregou primeiro o consumer e depois a core.
- O processo já aberto em `RELEASE-UNIFICADA\OOT` não foi encerrado ou alterado.

## Commits

- `3b28204fa` em `agent/OOT-NATIVE-001-native-engine`, worktree
  `OOT-NATIVE-001`. Snapshot único que cobre OOT-NATIVE-001, OOT-MOVE-001, esta
  fatia e o trabalho interrompido de OOT-CORE-001; os arquivos se sobrepõem e
  não podem ser separados sem dividir hunks.
- `894ba40` em `agent/NATIVE-001-provider-runtime`, worktree `NATIVE-001` — a
  base ABI 1.1 incorporada aqui.
- `aba8e0db` em `agent/OOT-MOVE-001-transient-button-mappings`, submódulo
  `libultraship`.
- Sem push em nenhum repositório.

## Arquivos alterados

- `OOT-NATIVE-001/soh/soh/ShipLuaBootstrap.cpp`
- `OOT-NATIVE-001/tools/package-coreext-demo.ps1`
- `OOT-NATIVE-001/tools/package-linkspan-overlay.ps1`
- `OOT-NATIVE-001/docs/install-linkspan-overlay.md`

Fora do Git, por serem artefatos de build:
`OOT-NATIVE-001/build/coreext-packages-v1/**` e
`OOT-NATIVE-001/build/distribution/**`.

## Validação executada

```powershell
# host Release Windows x64, MSBuild com LinkToolPath x64
cmake --build ... --config Release --target soh
oot_native_engine_tests
git diff --check   # core e host
# playtest isolado sobre a release oficial Shipwright 9.2.3
```

- Instalação de teste:
  `OOT-NATIVE-001/build/official-9.2.3/fresh-coreext-abi11-20260912-0859`.
- Evidências:
  `OOT-NATIVE-001/build/runtime-evidence/coreext-abi11-20260912-0859/`.

## Resultado da validação

- Host Release Windows x64: aprovado. `soh.exe` SHA-256
  `516ADCEFAE6F09E852AC5647988FEDC54A6276B265811A8A6919596549FFA05F`.
- `oot_native_engine_tests`: aprovado após recompilação contra o SDK atual.
- Prova de mod independente: dois providers modificaram Player e o hash do host
  permaneceu inalterado.
- `git diff --check`: aprovado no core e no host.
- Sessão do playtest: `valid: true`, com executável, `oot.o2r` e os dois ZIPs
  inalterados. O log prova ABI 1.1, core antes do consumer, dois mods
  carregados e unload em ordem inversa.
- Artefatos e hashes:
  - overlay
    `LinkSpan-Shipwright-9.2.3-CoreExtensions-ABI1.1-Win64-overlay.zip` —
    `7C91F2A7E163CBADBED18976A9364EAA573347320CA724760537D574CD273A89`,
    reconferido em 2026-09-12 após o reboot da máquina;
  - core demo `linkspan-core-service-demo-1.0.0.zip` —
    `93070F302B93FF93513A697B74DCF0A8C99CE231A3F1CC1DF1468A4CED950A24`;
  - consumer demo `linkspan-core-consumer-demo-1.0.0.zip` —
    `74155634029EC74B328A7A36BD2F407FF956A3D18C62E0A8B85BD0E8CC76779D`.
- O overlay contém `soh.exe`, `soh.o2r`, metadados, checksums e Dynamic
  Movement Remake 0.2.3. Não contém ROM, `oot.o2r`, save ou configuração. Todos
  os cinco hashes internos foram recalculados sem divergência.

## Pendências

- Implementar os registries de engine exigidos pelos ports reais de Unbound e
  Not Enough Items, começando por recursos/arquivos, objetos, atores, itens e
  UI.
- O build ainda é prévia de desenvolvimento. Antes de uma release pública,
  remover do host OoT as sondagens experimentais de áudio MM, conforme a
  pendência já registrada em `OOT-OVERLAY-001`.

## Riscos

Herdados do artefato e já registrados em `OOT-OVERLAY-001`:

- Um provider nativo roda dentro do processo e pode causar crash ou acessar o
  mesmo estado que o jogo. Instalar somente mods nativos confiáveis.
- `soh.exe` e `soh.o2r` precisam ser instalados juntos; misturar um deles com o
  arquivo oficial causa falha no carregamento da interface.

Próprios desta fatia:

- O overlay embute Dynamic Movement Remake 0.2.3, cujo provider está acoplado ao
  fingerprint do SDK/host 9.2.3 e precisa ser recompilado quando os layouts
  internos mudarem.

## Próxima ação recomendada

- Retomar `OOT-CORE-001`, que já tem contrato e testes mas foi interrompida
  antes do playtest, e só então abrir os registries de objetos, atores, itens e
  UI.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 3b28204fa (RC2+); core 894ba40 na main.
- Core: na main: 894ba40; a branch agent/NATIVE-001-provider-runtime é ancestral da main.
- Host: 3b28204fa (RC3–RC9; já no RC2 local).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
