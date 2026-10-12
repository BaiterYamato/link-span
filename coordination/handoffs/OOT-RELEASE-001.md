# Handoff — OOT-RELEASE-001

## Estado

review — o **`oot-0.5.0-rc6`** passou na bateria em jogo e nos cinco gates em 2026-10-10/11, pela pipeline
`logs/rc-pipeline-oot-next`, mas não foi publicado: o host passou ao Shipwright 9.3.0. O RC7 (OOT-UPSTREAM-002)
também ficou só em rascunho, e o candidato publicado é o **`oot-0.5.0-rc8`**, pre-release de 2026-10-12 no
[BaiterYamato/Shipwright](https://github.com/BaiterYamato/Shipwright/releases/tag/oot-0.5.0-rc8) (ver `coordination/handoffs/OOT-UPSTREAM-003.md`). O RC5 foi validado e jogado pelo
mantenedor (2026-10-10), sem publicação; RC2 a RC7 ficam como histórico. Gate OoT (§18) 26/26.

## Resultado

### Release `oot-0.5.0-rc6` (2026-10-10/11, validada, não publicada)

Rascunho de pre-release no BaiterYamato/Shipwright (tag `oot-0.5.0-rc6` no `ee97c6752`), não publicado. Leva os 9 pacotes,
`RELEASE-NOTES.md`, `RELEASE.md`, `SHA256SUMS.txt`, `inventory.json` e `protected-content.txt`; o digest de cada
arquivo no GitHub confere com o `SHA256SUMS.txt`. O `protected-content.json` ficou de fora por trazer um caminho
local. Pasta: `OOT-UPSTREAM-78DC-001/build/release/oot-0.5.0-rc6`.

Host `soh.exe` SHA-256 `E05FC6D4DCACFEB30E3A70135C40B024B5D81588AF0B8AD5F3F1D64CB7784CF9`, compilado do `923aaa07c`
(CTest 68/68); o `ee97c6752` só sobe as versões nos manifestos e docs. Core `4766532` (main com a MENU-001), exemplos
`b9665bc`, layout `9a593a6e` sem mudança.

| Pacote | Versão | SHA-256 (16) |
|---|---|---|
| Overlay Shipwright 9.2.3 / upstream `78dc6d970` / layout `9a593a6e` | API 0.5.0 | `8800ffc65cd746d9` |
| Unbound Framework | 0.6.11 | `13f5f08ec8c93639` |
| NEI | 0.3.21 | `7aa85363f5fcf31b` |
| Dynamic Movement Remake | 0.2.20 | `06b1a55670e9b0f1` |
| Mic Ocarina | 0.2.6 | `2c1023d46d282fd4` |
| Wind Waker Style | 0.1.2 | `4672d809f400e8cf` |
| Shovel Demo | 0.1.2 | `3b840a1bd825a5fa` |
| Unbound09 Actors Demo | dados | `ff4d7e74b0d7a2b2` |
| SDK de autor (ZIP separado) | rc6 | `b5ae90e2735ee4f6` |

**O que muda contra o RC5:**
- mensagens do modloader, guias e guia de autor do SDK em inglês, com o português ao lado (`*.pt-BR.md`);
  `RELEASE.md` e metadados do SDK em inglês (host `230e93c55` e `923aaa07c`);
- exemplos e demos no [link-span-examples](https://github.com/BaiterYamato/link-span-examples), de onde saem o Shovel
  Demo e o Actors Demo (core `0c5e43d` e `b64e200`);
- core com a MENU-001 integrada (PR #48);
- NEI 0.3.21, DMR 0.2.20 e Mic 0.2.6 só acompanham o SHA do `soh.exe` novo (`ee97c6752`), sem mudança de jogo.

**Validação** (evidência em `build/oot-upstream-001/evidence/release-g/rc6`; status em
`logs/rc-pipeline-oot-next/rc6-battery.status`):
- empacotamento duas vezes, 8/8 iguais byte a byte; `-Verify` 10/10; scanner OK na release, no SDK e na cópia montada;
- instalação fresca: `PROVA OK`, 0 mods; sem mods: falhas 0, sem erro no log, save preservado;
- matriz §12.2: 9/9 linhas `ok=True`; ciclo de save: `CICLO OK`; atualização NEI 0.3.20 → 0.3.21: `ATUALIZACAO OK`;
- fixtures do Unbound 9/9; probe do SDK compilado só com o ZIP: carregado e descarregado no jogo;
- autocontenção: referências 9, sem resolver 0; em jogo 37 conferidas, 0 ausentes (NTSC-US 1.1);
- gates add-ons, capability, recovery, seletor e rollback RC6→RC5→RC6: todos `ok=true gateComplete=true`, todos com o
  `soh.exe` do RC6.

### Release `oot-0.5.0-rc5` (2026-10-09/10)

Candidato em `OOT-UPSTREAM-78DC-001/build/release/oot-0.5.0-rc5`. Host `soh.exe` com commit embutido `ddeffea`
(`ddeffeadf`), SHA-256 `A65950B1FFB0380FD6F242E0AD22BDE3B67BDFC55D125BD83D7936DDE144850B`, layout `9a593a6e` sem
mudança, CTest 68/68. Os pacotes saem do host `c0fad7399` (o `soh.exe` não foi recompilado depois de `ddeffeadf`).
Commits locais, sem push; nada publicado.

| Pacote | Versão | SHA-256 (16) |
|---|---|---|
| Overlay Shipwright 9.2.3 / upstream `78dc6d970` / layout `9a593a6e` | API 0.5.0 | `7eeacf420ffac5ca` |
| Unbound Framework | 0.6.11 | `151bd9353bcce880` |
| NEI | 0.3.20 | `4c1041a71b1f560d` |
| Dynamic Movement Remake | 0.2.19 | `53809f6251005ae9` |
| Mic Ocarina | 0.2.5 | `b7c542559d0630d6` |
| Wind Waker Style | 0.1.2 | `e8ff0af96951f0b6` |
| Shovel Demo | 0.1.2 | `50f7941a5922a398` |
| Unbound09 Actors Demo | dados | `ff4d7e74b0d7a2b2` |
| SDK de autor (ZIP separado) | rc5 | `7c6fbd73daf6e2ae` |

**O que muda contra o RC4:**
- **COREEXT-012 / RFC 0028 fase 1** (`45530b878`): setting `linkspan.pause.equip_selector`, helper
  `LinkSpan_PauseEquipSelectorEnabled`, símbolo com nome de mod removido; HUD do D-pad e seletor separados.
- **Logger** (`6bdb665b2`): correção do `ResourceLogging` agora commitada.
- **Extrator** (core `648ba8f`): retentativa do `rename` do staging; o `ub030-boot` do RC4 passou no RC5.
- **Camada Unbound sem framework** (`ddeffeadf`): recusada com diagnóstico; o mesmo commit traz o `crash-coremod` e o
  scene-demo 0.6.x para os gates.
- **Licenças** CC0 (host `1b9158a1c`, core `e6dd523`, coordenação `7be0d33`); **SDK** em ZIP (`37ed69386`); versões
  (`bf8715832`).
- **NEI 0.3.20** sem recompilar o host:
  - `10a9407cc`: pipeline do fork acompanha a chave sem mods (`/FI` de `linkspan_vanilla.h`, chave do host por
    ponteiro, defines só do host copiados pelo extrator) e os conflitos novos de overlay (`Message_Decode`,
    `Message_DrawMain`, `EnMThunder_SpinAttacking`, `Message_DecodeChecked`, assinatura do `KaleidoScope_DrawItemSelect`).
  - `b7570e79d`: replay da ocarina com o instrumento padrão (o stub gerado devolvia `OCARINA_INSTRUMENT_OFF` e a
    mensagem travava em `MSGMODE_DISPLAY_SONG_PLAYED`).
  - `29297d7d1`: ReDead amigável com Skull/Spooky Mask e realce `NightGSAlwaysSpawn`.
  - `c0fad7399`: cursor das setas de página do pause e descarte da foto da Pictograph Box.
  - Achados de três revisões do Codex: diff RC4→RC5 e auditorias dos 120 stubs com retorno e dos 106 stubs void.
    Ficam como limitação: cegueira do Gust Jar (modo Shadow) e magia infinita do Chateau Romani.
- **Release** (`7fc46b5bf`): tabela do `RELEASE.md` válida e NOTICE do overlay com os nomes instalados. Notas revisadas
  por auditoria (Codex): sem promessa de save abrindo na versão anterior, limite da proteção de boot, efeitos locais
  do Unbound e caminho curto da pasta do jogo.

**Decisões do mantenedor (2026-10-09):** CC0 1.0 para o trabalho próprio (ícone da Navi do DMR fora; Shipwright,
NEI/skijer, Unbound/roborich e Wind Waker Style seguem com os autores); RFC 0027 aceita e RFC 0028 fase 1 com áudio
por caminho e `IsUnboundLayerArchive` como dívida C aceita, saída antes da Fase H; segunda família de ROM coberta
pela prova de autocontenção (§10.5/§14.3); playtest dos itens NEI feito pelo mantenedor (2026-10-10: itens NEI, DMR e Mic Ocarina funcionando).

**Validação** (evidência em `build/oot-upstream-001/evidence/release-g/rc5`, bateria `rc5-battery.ps1`):
- empacotamento duas vezes, 8/8 iguais byte a byte; `-Verify` 10/10; scanner OK (21.872 arquivos);
- instalação fresca: `PROVA OK`, 0 mods;
- sem mods: falhas 0, sem erro no log, motor do upstream, save preservado;
- matriz §12.2: 9/9 linhas `ok=True`;
- ciclo de save: `CICLO OK`; atualização NEI 0.3.18 (save do RC4) → 0.3.20: `ATUALIZACAO OK`;
- fixtures do Unbound 9/9: completa, boot (`ub030-boot`), savestate, m18, a8200, r04, r0809, água e luzes;
- probe do SDK compilado só com o ZIP: carregado e descarregado no jogo;
- autocontenção: referências 9, sem resolver 0; em jogo 37 conferidas, 0 ausentes (NTSC-US 1.1);
- gates (`run-gates.ps1`, tentativa 3): add-ons, capability (pasta, solta e controle), recovery (crash do
  `linkspan.crash-coremod` atribuído, desativado pela lista e pacote preservado), seletor sem/com DMR (C-right retido
  após save e reinício) e rollback RC5→RC4→RC5: todos `ok=true gateComplete=true`.

Tentativas anteriores, arquivadas: `rc5-tentativa1-nei0319` (NEI que seria substituído), `rc5-tentativa2-foco`
(bateria com console minimizado tomava o primeiro plano), `gates-tentativa1/2` (harness: cópias em caminho
longo, caixa de crash do LUS não fechada, item esperado no slot 0). Nenhuma delas mudou código de jogo.

**Pendências:** publicação da release; restrições de redistribuição das partes de terceiros; queda do setting do
seletor com ele aberto (não exercitada em jogo). Playtest NEI e físico de DMR/Mic feitos pelo mantenedor em
2026-10-10 (relato: funcionando).

### Gate OoT (§18) — situação no RC5

| Critério | Situação | Evidência |
|---|---|---|
| overlay instala sobre 9.2.3 fresco | ✅ | `PROVA OK`, SHA do host, 0 mods |
| sem mods mantém vanilla | ✅ | sessão sem mods, motor do upstream; OOT-VANILLA-001 (sem A/B por pixel) |
| Unbound pacote único removível | ✅ | matriz 02/04/05/07/09, um `.shipmod` |
| NEI pacote único removível | ✅ | matriz 03/04/06/07/09, ciclo de save |
| NEI sem Unbound | ✅ | linhas 03/06 |
| NEI central sem SM64/Pikachu/Wolf Link/FleetShipCombo | ✅ | expansões fora do pacote, matriz |
| dependências opcionais em diretório privado | — | nenhuma dependência opcional distribuída |
| sem ROM SM64; recusa de formato | ✅ / — | scanner OK; componente SM64 ausente |
| Unbound sem NEI | ✅ | linhas 02/05 |
| ambos juntos | ✅ | linhas 04/09 |
| DMR junto dos dois | ✅ | linha 07 |
| instalar/atualizar não altera `soh.exe` | ✅ | SHA `A65950B1…` em todas as sessões, update e rollback |
| ondas de itens sem função específica no host | ✅ | RFC 0028 fase 1; NEI 0.3.20 sem recompilar o host; gate do seletor |
| add-ons de cena e item | ✅ | gate-addons |
| saves sobrevivem a update/desativação/reinstalação | ✅ | `CICLO OK`, `ATUALIZACAO OK`, gate-rollback |
| falha de coremod recupera sem apagar | ✅ | gate-recovery |
| scanner nos distribuídos | ✅ | release, SDK e cópia montada |
| testes sem ROM no Windows | ✅ | CTest 68/68 |
| playtest real dos limites do Unbound | ✅ | fixtures 9/9 |
| fixtures de fronteira do Prelude | ✅ | completa, boot, m18, a8200 |
| incompatíveis recusados sem a capability | ✅ | gate-capability |
| cenas resolvem recursos ou são autocontidas | ✅ | autocontenção (decisão do mantenedor) |
| conflito de cópias de cena antes do gameplay | ✅ | fixture boot |
| playtest real de todos os itens NEI | ✅ | mantenedor, 2026-10-10, cópia `OOT-playtest-nei-rc5` com NEI 0.3.20 |
| docs, exemplos, SDK e hashes | ✅ | release, SDK, probe em jogo |
| coremods sem recompilar o host (RFC 0027) | ✅ | NEI 0.3.20 sobre o mesmo `soh.exe`; rodadas após o RC4 enumeradas em COREEXT-011 |

### Release `oot-0.5.0-rc4` (2026-10-04, registrada em 2026-10-09)

Pasta `OOT-UPSTREAM-78DC-001/build/release/oot-0.5.0-rc4`. Host `soh.exe` `168A6885…`, commit embutido `e855b88`
(rodada OOT-VANILLA-001 `43cb7e01e` + versões), layout `9a593a6e` sem mudança.

| Pacote | SHA-256 (16) |
|---|---|
| `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-9a593a6e-Win64-overlay.zip` | `02eb16168f49ede2` |
| `LinkSpan-Unbound-Framework-0.6.10-layout-9a593a6e.shipmod` | `6afcb8d2928945c7` (igual ao RC3) |
| `LinkSpan-NEI-0.3.18-layout-9a593a6e.shipmod` | `d138e79a42c2e340` |
| `Dynamic-Movement-Remake-0.2.18-layout-9a593a6e.zip` | `da59398fc051c57e` |
| `LinkSpan-Mic-Ocarina-0.2.4-layout-9a593a6e.zip` | `c782476e9aa45c58` |
| `LinkSpan-WindWakerStyle-0.1.1.shipmod` | `1ab4ab84c753a1e4` (igual ao RC3) |
| `LinkSpan-Shovel-Demo-0.1.1-layout-9a593a6e-9f1e44e0.zip` | `f7bec07217b8457f` (igual ao RC3) |
| `LinkSpan-Unbound09-Actors-Demo.o2r` | `ff4d7e74b0d7a2b2` (igual ao RC3) |

**Validação** (`evidence/release-g/rc4`, `RESUMO.txt`): CTest 68/68 (teste novo da vanilla); dois empacotamentos
idênticos (8/8); `-Verify` 8/8; scanner sem achados.
- instalação fresca: `PROVA OK`, 0 mods, 371 comandos de cena em XML;
- sem mods: 0 falhas, 0 erros no log, namespaces `linkspan`/`unbound` iguais;
- matriz §12.2: 9/9 linhas, 0 falhas;
- ciclo de save: `CICLO OK`;
- atualização do NEI 0.3.17 (RC3) → 0.3.18 (RC4): `ATUALIZACAO OK`;
- fixtures do Unbound: 8/9 com 0 falhas. A `boot` acusou 9 falhas, todas na sessão `grafo`.

**A falha da `boot` não é regressão do RC4.** No log da sessão `grafo`
(`rc4/fixtures/boot/session-grafo/*.post.log:9`) o host recusou o framework no boot com `cannot commit extracted
package: Acesso negado`; sem o framework, nenhuma nota do grafo foi impressa, e as 9 falhas são consequência disso.
As outras 8 sessões da mesma fixture passaram. A causa é o `rename` único da pasta de staging em
`extern/ship-lua/src/manifest/PackageExtractor.cpp`, que falha quando outro processo (antivírus, indexador) ainda
segura um arquivo recém-extraído. É o mesmo flake visto na UNBOUND-028. Corrigido para o RC5 no core `648ba8f`
(nova tentativa por até ~2,3 s, com teste que reproduz o arquivo aberto no Windows).

### Release `oot-0.5.0-rc3` (2026-10-04)

Pasta `OOT-UPSTREAM-78DC-001/build/release/oot-0.5.0-rc3`. Host `soh.exe` `F645976A…`, construído na integração
`0ff8f2ff4`, com layout `9a593a6e`.

| Pacote | SHA-256 (16) |
|---|---|
| `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-9a593a6e-Win64-overlay.zip` | `fa94476ee8445b8b` |
| `LinkSpan-Unbound-Framework-0.6.10-layout-9a593a6e.shipmod` | `6afcb8d2928945c7` |
| `LinkSpan-NEI-0.3.17-layout-9a593a6e.shipmod` | `c73dd7a75253aea0` |
| `Dynamic-Movement-Remake-0.2.17-layout-9a593a6e.zip` | `7b919c0dbc527a0b` |
| `LinkSpan-Mic-Ocarina-0.2.3-layout-9a593a6e.zip` | `9584bc77dee65fe7` |
| `LinkSpan-WindWakerStyle-0.1.1.shipmod` | `1ab4ab84c753a1e4` (igual ao RC2) |
| `LinkSpan-Shovel-Demo-0.1.1-layout-9a593a6e-9f1e44e0.zip` | `f7bec07217b8457f` (igual ao RC2) |
| `LinkSpan-Unbound09-Actors-Demo.o2r` | `ff4d7e74b0d7a2b2` (igual ao RC2) |

**O que muda contra o RC2:**
- a rodada de host da RFC 0027 inteira: lote de 7, patches 8 a 11 e os opcionais 12 a 15, com o F2;
- o framework Unbound 0.6.10 (nota de luzes em dois níveis e as correções 0.6.1 a 0.6.9 das UNBOUND-018 a 030);
- NEI, DMR e Mic reempacotados com o fingerprint `f645976a…`.

**Validação:**
- **Build e testes:** CTest 67/67. Dois empacotamentos completos saíram idênticos byte a byte (8/8). O scanner de
  conteúdo protegido passou e o `-Verify` conferiu os 8 pacotes.
- **Instalação fresca** (`evidence/release-g/rc3/fresh-install`): `PROVA OK`, 0 mods, 371 comandos de cena em XML,
  commit embutido `0ff8f2f`.
- **Matriz §12.2** (`rc3/matrix`): as 9 linhas passaram, sem falhas, da 01 (sem mods) à 09 (WW + Unbound + NEI).
- **Ciclo de save** (`rc3/save-cycle`): `CICLO OK`.
- **Atualização do NEI de 0.3.16 para 0.3.17** (`rc3/update-nei`): `ATUALIZACAO OK`.
  - Fluxo: u1 no host do RC2 (`E278E9A3`, Unbound 0.6.0 + NEI 0.3.16) grava o save; u2 no RC3 (Unbound 0.6.10 +
    NEI 0.3.17) carrega esse save.
  - O `fork: active` e as duas versões aparecem nos logs, e o `nei.state` (`83741ffd1c4a`) chega idêntico.
  - A primeira tentativa (`update-nei-tentativa1`) reprovou só na u1: o arquivo abriu às 10:08:51, antes do primeiro
    input do roteiro. Houve input externo durante o teste.
- **Fixtures do Unbound, com o framework do RC** (`rc3/fixtures`): completa, boot, savestate, m18, a8200, r04,
  r0809, água e luzes, todas com 0 falhas.
  - A a8200 reprovou na bateria (`a8200-checagem-068`) porque a checagem esperava o aviso do host para 8 200 atores.
  - Desde o framework 0.6.9, o próprio framework grava só os primeiros 8 192 e deixa uma nota (`transcode.cpp:851`),
    então o aviso do host não é mais alcançável.
  - A checagem passou a aceitar também a nota do framework com a chegada em `atores=8192`, e a fixture refeita passou.
- **`ResourceLogging` ligado, sem mods, casa do Link** (`rc3/resourcelogging`): 2/2 sessões, 870 polígonos logados,
  6 deles no formato 1, e nenhum crash. No RC2, esse caminho caía no `LogMeshAsXML` (ver Riscos).
- **Auditoria "sem mods = vanilla"** (`rc3/vanilla-audit/AUDITORIA.md`, Codex só leitura, conferida por amostragem):
  o host sem mods **não** é equivalente estrito ao upstream.
  - colisão dinâmica arredonda com `lroundf`, onde o upstream truncava;
  - colliders e luzes passam a float;
  - matrizes em float (`GBI_FLOAT_MTX ON`) no lugar do ponto fixo s16.16;
  - mudanças no motor de texto (quicktext, B mantido, próxima mensagem);
  - heap +8 MiB;
  - savestate recusado depois de trocar a colisão da cena.
  - A matriz sem mods passa; a diferença é de precisão, memória e casos de borda, não de função visível comprovada.

### Gate OoT (§18) — situação no RC3

Só as linhas que mudaram contra a tabela do RC2 (mais abaixo); as demais seguem iguais.

| Critério | RC2 | RC3 | Evidência / motivo |
|---|---|---|---|
| overlay instala sobre 9.2.3 fresco | ✅ | ✅ | `rc3/fresh-install` |
| sem mods = vanilla | 🟡 | ❌ estrito, 🟡 funcional | auditoria de código acima; a linha 01 da matriz passa. Decisão do mantenedor: aceitar o alargamento do motor ou restringir as diferenças D ao caso com mod |
| nenhuma onda de itens exige função específica no host | 🟡 | 🟡 | RFC 0028 (rascunho, COREEXT-012) propõe o setting `linkspan.pause.equip_selector` no lugar do `LinkSpan_DynamicMovementInventoryMenuEnabled` |
| saves sobrevivem a update, desativação e reinstalação | 🟡 | ✅ | ciclo de save e atualização do NEI de 0.3.16 para 0.3.17, com Unbound de 0.6.0 para 0.6.10 junto |
| playtest real dos limites do Unbound | 🟡 | ✅ | E-fixtures refeitas no RC3 (9/9) |
| fixtures de fronteira do Prelude | ❌ | ✅ | fixture completa no RC3: m08, m09, m12, m15, m17, m19, m24, exits, edge, span |
| cenas afetadas por versão resolvem recursos | ❌ | ✅ | fixture boot, sessão `ub019-a` (`ausentes=3 recusados=1`) |
| conflito entre duas cópias da mesma cena antes do gameplay | ❌ | ✅ | fixture boot, sessão `ub018-abc` |
| depois do COREEXT-GATE só DLLs recompiladas | ❌ | 🟡 | rodada de host agrupada e aprovada pelo usuário (RFC 0027 v10); a RFC segue em rascunho na regularização dos 24 commits antigos e nas dívidas C |
| playtest real de todos os itens NEI | ❌ | ❌ | só harness; falta o playtest físico |

### Release `oot-0.5.0-rc2`

Pasta `OOT-UPSTREAM-78DC-001/build/release/oot-0.5.0-rc2`: `SHA256SUMS.txt`, `inventory.json`, `RELEASE.md` e
`protected-content.{json,txt}`. O `-Verify` confere 8/8.

| Pacote | Versão | SHA-256 (16) |
|---|---|---|
| `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-9a593a6e-Win64-overlay.zip` | overlay, API 0.5.0 | `f6b730078b2f5d48` |
| `LinkSpan-Unbound-Framework-0.6.0-layout-9a593a6e.shipmod` | 0.6.0 | `119853ff85b79687` |
| `LinkSpan-NEI-0.3.16-layout-9a593a6e.shipmod` | 0.3.16 | `4fe799bc6c67c81a` |
| `Dynamic-Movement-Remake-0.2.16-layout-9a593a6e.zip` | 0.2.16 | `4636b5840c96270a` |
| `LinkSpan-Mic-Ocarina-0.2.2-layout-9a593a6e.zip` | 0.2.2 | `2c2d2744245f9904` |
| `LinkSpan-WindWakerStyle-0.1.1.shipmod` | 0.1.1 | `1ab4ab84c753a1e4` |
| `LinkSpan-Shovel-Demo-0.1.1-layout-9a593a6e-9f1e44e0.zip` | 0.1.1 | `f7bec07217b8457f` |
| `LinkSpan-Unbound09-Actors-Demo.o2r` | recursos | `ff4d7e74b0d7a2b2` |

- **Host do overlay:** `soh.exe` `E278E9A3…`, com o commit embutido `fc27099` e o layout `9a593a6e`. Depois de
  `fc2709989` mudaram só o DMR, a sonda de teste, o ponteiro do submódulo (docs) e o construtor da release
  (`aa08cd9ed`), então o host é o mesmo do HEAD.
- **Layout:** overlay, Unbound, NEI, DMR, Mic e Shovel declaram `9a593a6e`; WW e Actors Demo não declaram (não usam o
  SDK nativo), e o `RELEASE.md` os mostra como "não declarado".
- **Reprodutibilidade:** dois empacotamentos completos (`build/rc3-a`, `build/rc3-b`) deram 8/8 pacotes idênticos
  byte a byte (`tools/test-reproducible.ps1`, relatório `evidence/release-g/reproducibility.json`).
- **Scanner de conteúdo protegido:** OK nos artefatos e na pasta montada (21.671 arquivos, 9 ZIPs contando os
  aninhados).
- **`host_fingerprints`:** NEI, DMR e Mic citam o `soh.exe` do overlay; o construtor recusa a release se não citarem.
  Unbound, WW e Shovel não usam o escape hatch.
- **UNBOUND-012 fechada:** os schemas JSON do formato 2 (`schemas/unbound/*.schema.json` + `validate_schemas.py`)
  estão no pacote do framework.
- **Documentação:** guia de autor na API 0.5 (`docs/writing-mods.md`), contrato nativo do OoT
  (`docs/native-providers.md`), guia do SDK (`soh/native-sdk/README.md`) e `RELEASE.md` com instalação,
  atualização e recuperação.
- O RC1 (`build/release/oot-0.5.0-rc1`, host `D8A161F0`) foi superado: tinha o gatilho de áudio MM no host e o DMR
  0.2.15 que não carrega fora do fingerprint.

### Defeitos achados pela fase G e corrigidos

| Defeito | Efeito | Correção |
|---|---|---|
| `ZipFile.CreateFromDirectory` em 12 scripts de demo e no das E-fixtures | entradas com `\`; o construtor da release recusou o shovel-demo | `New-LinkSpanZip` nos 13 (`c46fcb006`); o `package-unbound-actors-demo.ps1` já usava `ZipArchive` próprio, com nomes normalizados e horário fixo (`ee97866a3`) |
| manifesto de mod nativo gerado no configure sem depender do `manifest.toml` | RC1 com DMR/Mic nomeados 0.2.15/0.2.2, manifesto 0.2.14/0.2.1 e `host_fingerprints` do host anterior (`42f86324`) | `manifest.toml` em `CMAKE_CONFIGURE_DEPENDS`; o construtor recusa nome × manifesto e fingerprint × `soh.exe` do overlay, provado com dois casos negativos (`d1983a5ba`) |
| `package-unbound-framework.ps1` com `GetFullPath` relativo | o pacote falhava fora do diretório do repo | caminhos pelo `Get-Location` (`d1983a5ba`) |
| construtor da release | stderr do scanner virava erro no PS 5.1; `.o2r` de recursos era recusado; `SHA256SUMS` saía fora de ordem; o texto citava um `mods-disabled/` que não existe | `d1983a5ba` |
| gatilho de teste do áudio MM no host (OOT-AUDIO-001) | sem nenhum mod, tentava tocar som do MM 2 s depois de entrar em jogo e a cada D-pad esquerda | removido (`fc2709989`); host recompilado |
| DMR 0.2.14 exigia `gItemIcons`/`OLib_Vec3fDiffToVecSphGeo` no init | fora do `host_fingerprints` o mod não carregava; `oot_native_independent_mod` falhava desde o commit do Codex | símbolos opcionais (DMR 0.2.16) e sonda no contrato 0.2.14 (`fea9ce74c`) |
| construtor cortava no primeiro `-layout-` e exigia Python 3.11 sem checar (revisão do Codex sobre `aa08cd9ed`) | nome como `LinkSpan-layout-converter-0.2.16-…` perdia a versão e aceitava manifesto velho; sem `tomllib`, todo manifesto parecia inválido | só o sufixo `-layout-<hex>` do fim sai; `tomli` como alternativa e `--check` antes de montar (`f1dc7eab8`); 12 casos forjados |
| construtor lia o `manifest.toml` por regex (revisão do Codex sobre `d1983a5ba`) | hash citado em comentário aprovava fingerprint errado; chave entre aspas ou `# ]` no array pulava a checagem; versão do nome perdia o pré-release SemVer | `tools/read-linkspan-manifest.py` com `tomllib` e as regras do `ManifestParser`; versão completa cortada no `-layout-` (`aa08cd9ed`). RC2 refeito sai idêntico; 9 casos de pacote forjado |

### Integração dos trabalhos de 27/09–01/10

O trabalho do Codex foi commitado por área em 2026-10-02. Os SHAs estão nos handoffs de cada tarefa (CEL-006,
BIND-002, OOT-NEI-017, OOT-MIC-004, OOT-MOVE-007 no link-span; SKIJER-001, NEI-UI-001, OOT-CI-001 e SCOPE-OOT-001
no core). A RFC duplicada foi resolvida: o randomizer hints entrou no core já como `rfcs/0026` (`4a75770`), e o
`8182f84` ajusta o `item_count`/`item_info` para responder em qualquer save.

## Commits

RC6 (2026-10-10/11):
- host `ee97c6752` (`agent/OOT-UPSTREAM-001-78dc6d970-integration`): versões do RC6; enviado ao BaiterYamato/Shipwright
  em 2026-10-11, a pedido do usuário;
- link-span, branch `agent/OOT-UPSTREAM-003-core-docs` (PR para a main): este handoff e o UPDATE do RC6 em
  `docs/agents/Plans.md`. O README e a wiki em `docs/wiki` passaram direto do RC5 para o RC8 (OOT-UPSTREAM-003).

Até o RC3:

Locais, sem push; autorizados pelo usuário em 2026-10-02 ("Faça o que é necessário").

RC3, no host (`agent/OOT-UPSTREAM-001-78dc6d970-integration`):
- `7e1e5bec3` e `288665520`: e-fixtures 0.1.11 e 0.1.12;
- `14d240116`..`33f3c7c6f`: cherry-pick `-x` dos 14 commits do candidato da rodada da RFC 0027 (aprovada em
  2026-10-04);
- `2a074fe60`: framework 0.6.10;
- `0ff8f2ff4`: NEI 0.3.17, DMR 0.2.17 e Mic 0.2.3.

No libultraship, `agent/OOT-UPSTREAM-001-linkspan-extensions` foi avançado direto até `f2e87310`. No core, `55ece4b`
traz a RFC 0028 em rascunho.

- host `OOT-UPSTREAM-78DC-001`, branch `agent/OOT-UPSTREAM-001-78dc6d970-integration`:
  - `5827e580a` DMR 0.2.15 e Mic 0.2.2 (correções da revisão do Codex);
  - `ee97866a3` UNBOUND-012;
  - `4c45d5c3a` camada Unbound de mod rejeitado fora do VFS e re-registro de tipo de ator descarregado;
  - `c1dec2eb6` e `4ae2fc4ed` ferramentas de release;
  - `75b7d1a57` guia do SDK;
  - `c46fcb006` ZIP dos demos;
  - `d1983a5ba` checagens do construtor e configure do DMR/Mic;
  - `fc2709989` remoção do gatilho de áudio MM;
  - `fea9ce74c` DMR 0.2.16 e sonda;
  - `82a916ef4` submódulo;
  - `aa08cd9ed` construtor lê o manifesto com `tomllib` (achado da revisão do Codex);
  - `f1dc7eab8` sufixo de layout do fim e checagem do leitor de TOML (achado da revisão do Codex sobre `aa08cd9ed`).
- core `extern/ship-lua`, branch `agent/MENU-001-mod-menu`:
  - `8182f84` RFC 0026 (`item_count`/`item_info` em qualquer save);
  - `45d5162` guia de autor;
  - `13482b1` SHAs nos handoffs do core.
- link-span: este claim, este handoff e o UPDATE em `docs/agents/Plans.md`.

## Arquivos alterados

- **RC6:**
  - host: `soh/native-sdk/{example,mic-ocarina}/manifest.toml`, `soh/native-sdk/nei-core/README{,.pt-BR}.md` e
    `tools/package-nei-core.ps1` (`ee97c6752`);
  - link-span: `README{,.pt-BR}.md`, `docs/wiki/{Getting-Started,Home,Native-Mods,Example-Mods,Troubleshooting}`
    (EN e pt-BR), este handoff e `docs/agents/Plans.md`;
  - a pipeline do RC6 fica fora do git, em `logs/rc-pipeline-oot-next`.
- **Host — ferramentas:**
  - `tools/linkspan-zip.ps1`, `tools/build-linkspan-release.ps1`, `tools/read-linkspan-manifest.py`,
    `tools/scan-protected-content.py` e `tools/test-reproducible.ps1`;
  - `tools/package-{nei-core,unbound-framework,unbound-actors-demo,linkspan-mod}.ps1`;
  - os 12 `tools/package-*-demo.ps1` de `c46fcb006` e `package-unbound-e-fixtures.ps1`;
  - `soh/native-sdk/{ww-style,cel-render-demo}/tools/package-*.ps1`.
- **Host — código:**
  - `soh/soh/ShipLuaBootstrap.cpp`;
  - `soh/native-sdk/example/{provider.cpp,manifest.toml,CMakeLists.txt}` e
    `soh/native-sdk/mic-ocarina/CMakeLists.txt`;
  - `soh/native-sdk/tests/EngineTests.cpp`;
  - `soh/native-sdk/mic-ocarina/{manifest.toml,provider.cpp,remake_ui_tests.cpp}` (`5827e580a`);
  - `soh/soh/native/OotNativeActorModels.cpp` (`4c45d5c3a`);
  - `soh/native-sdk/ww-style/manifest.toml` (`c1dec2eb6`);
  - `extern/ship-lua` (ponteiro do submódulo: `4c45d5c3a`, `75b7d1a57`, `82a916ef4`).
- **Host — Unbound e SDK:** `soh/native-sdk/unbound-core/{schemas/**,README.md,actor_registry.cpp}` e
  `soh/native-sdk/README.md`.
- **Core:** `docs/writing-mods.md`, `docs/native-providers.md`, `rfcs/0026-…` e os handoffs SKIJER-001, NEI-UI-001,
  OOT-CI-001 e SCOPE-OOT-001.
- **Gerados (não versionados):**
  - `build/release/oot-0.5.0-rc{1,2}` e `build/rc{2,3}-{a,b}` com as pastas `-flat`;
  - a cópia fresca `build/oot-upstream-001/smoke-rc2-20261003`;
  - a evidência em `build/oot-upstream-001/evidence/release-g/`.
- `coordination/STATUS.md` não foi alterado; `RELEASE-UNIFICADA\OOT` não foi tocada.
- **Sobreposição apontada pelo gate:** CEL-005, COREEXT-010, OOT-MIC-004 e OOT-NEI-017, todos em review. Foram
  mudanças de integração: ZIP determinístico, configure do manifesto e docs, sem mudar o comportamento dessas
  tarefas.

## Validação executada

**RC6** (Windows 11, MSVC 19.44 / toolset 14.44, Release x64, PowerShell 5.1), pela pipeline `logs/rc-pipeline-oot-next`:

```text
rc-build.ps1 -Tag rc6                                            # host 923aaa07c, soh_symbols, CTest 68/68, NEI, mods, demos
rc-post-build.ps1 -Tag rc6 -ReleaseNotes rc6-sdk/RELEASE-NOTES.md   # 2 empacotamentos, pacotes dos gates, SDK, probe, -Verify
rc-battery.ps1 -Tag rc6 -PrevTag rc5 -PrevSmoke build\oot-upstream-001\smoke-rc5-20261009 -GamepadConnected
```

**RC2/RC3:**

Windows 11, MSVC 14.44, Release x64, PowerShell 5.1.

```text
cmake build/x64 && cmake --build build/x64 --config Release --target soh soh_symbols    # host E278E9A3
ctest --test-dir build/x64 -C Release                                                    # 67/67
tools/test-reproducible.ps1 -FirstDirectory build/rc3-flat-a -SecondDirectory build/rc3-flat-b
tools/build-linkspan-release.ps1 -ArtifactDirectory build/rc3-flat-a -Version oot-0.5.0-rc2 -AllowLarge '*-Win64-overlay.zip'
tools/build-linkspan-release.ps1 -Verify -Version oot-0.5.0-rc2
tools/test-linkspan-overlay.ps1 -Overlay <rc2 overlay> -Base build/oot-upstream-001/base-9.2.3-nei016 -ExpectedMods 0 -ResourceLogging
scratchpad matrix-all.ps1 / save-cycle.ps1 (harness soh-runtime-playtest; 2 sessões por linha)
```

Matriz: cada linha limpa `mods/`, copia só os pacotes da linha e restaura a fixture `file1.sav` (criança,
entrada 219). A sessão 1 faz boot, abre o arquivo 1 pelo console (`shiplua_goto_file_select`,
`shiplua_open_file 0`), entra em gameplay, dá `reset` com Autosave, abre de novo e fecha pela janela. A sessão 2
reinicia o jogo, abre o arquivo e fecha. O ciclo de save mantém o arquivo entre c1 (Unbound+NEI), c2 (sem NEI) e
c3 (NEI reinstalado) e compara os blocos de namespace.

O harness só manda input com o PC ocioso há 4 min (`GetLastInputInfo`), traz a janela com `AttachThreadInput` e
segura uma trava por cópia de teste (mutex nomeado). A trava entrou depois que duas baterias rodaram juntas na mesma
cópia (02:08 de 03/10): uma limpou a `mods/` da outra (a c1 subiu sem o NEI, e o host não registrou rejeição porque o
pacote nem estava lá), dois `soh.exe` abriram na mesma pasta e o foco pulou entre as janelas. Essas rodadas foram
descartadas.

Duas revisões do Codex (`gpt-6.1-sol`, esforço alto) sobre o harness acharam 7 problemas altos, 5 médios e 1 baixo, e
na re-revisão mais 4: prova por totais de log, erro no log e exit ≠ 0 que não reprovavam, repetição que apagava falhas
reais, entrada do usuário tomada como do harness e digitação que seguia depois de perder o foco. A bateria desta
evidência rodou antes dessas correções e foi rechecada com os critérios novos: `matrix_check.py` (um boot por log, uma
linha de carga, checkpoints em ordem, erro e exit reprovam) e `save_ns.py --cycle`. O harness corrigido foi
validado com a linha 04 (`evidence/release-g/harness2-check`, 06:29–06:32): 2/2 sessões com checkpoints em ordem, e
a s2 começou 4 s depois da s1.

O `soh.exe` aberto gera entrada sintética contínua: o `GetLastInputInfo` fica em 0 com o cursor parado e volta a
contar quando o jogo fecha. Com o jogo rodando, a presença do usuário só aparece pela janela da frente, pelo cursor e
pela checagem de foco a cada caractere, antes do clique e antes da captura (script da skill `soh-runtime-playtest`).
A marca de entrada própria é o fim do processo da sessão, e só vale quando o input foi completo.

## Resultado da validação

- **RC6:** bateria completa e cinco gates verdes; detalhes na seção do RC6 em Resultado.
- **Testes sem ROM:** CTest raiz 67/67 com o host `E278E9A3` (`build/x64/Testing/Temporary/LastTest.log`). O
  `oot_native_independent_mod` falhava desde o DMR 0.2.14 e foi corrigido; o `LastTestsFailed.log` ainda lista essa
  falha da execução anterior.
- **Instalação fresca (rc2):** overlay sobre a cópia limpa da 9.2.3 (`base-9.2.3-nei016`, com o `oot.o2r` do usuário,
  que não sai da máquina). `PROVA OK`, 0 mods, 371 comandos de cena em XML com `ResourceLogging` e nenhuma linha do
  gatilho de áudio MM (`evidence/release-g/fresh-install-rc2`). O RC1 também passou (`fresh-install-rc1`).
- **Matriz §12.2 — boot, reinício e fechamento** (`evidence/release-g/matrix-rc2-boot`): 9 configurações, 2 sessões
  cada. Todas as 18 sessões são válidas no harness (SHA do host e fixtures rastreadas inalterados), sem erro no log.

  | Linha | Pacotes | Mods carregados (s1/s2) | Fechamento (s) |
  |---|---|---|---|
  | 01 sem coremods | — | 0/0 | 1,2/1,3 |
  | 02 Unbound | U | 1/1 | 2,1/1,9 |
  | 03 NEI | N | 1/1 | 2,0/1,9 |
  | 04 Unbound+NEI | U N | 2/2 | 2,5/2,7 |
  | 05 Unbound+DMR | U D | 2/2 | 2,5/2,4 |
  | 06 NEI+DMR | N D | 2/2 | 3,0/2,4 |
  | 07 os três | U N D | 3/3 | 4,2/3,0 |
  | 08 WW | W | 1/1 | 1,7/1,5 |
  | 09 WW+Unbound+NEI | W U N | 3/3 | 2,8/2,7 |

  Os logs dessa rodada foram movidos para `matrix-rc2-boot/<linha>/`; o `archivedPostLog` dos resultados ainda aponta
  o caminho antigo em `matrix-rc2`. Nessa rodada o PC estava em uso, e o Windows recusou o foco à janela do jogo. Então não houve gameplay nem
  save/load, e o ciclo de save dessa rodada não gravou nada (descartado).
- **Matriz §12.2 — gameplay e save/load** (`evidence/release-g/matrix-rc2`, 03/10 05:31–06:00): 9 linhas, 2 sessões
  cada, 18/18 válidas. Na s1 o jogo abre o arquivo 1, o `reset` grava com o Autosave (`Save File` e `Save File
  Finish` entre o reset e a volta à seleção) e o arquivo abre de novo. Na s2 o jogo reinicia e abre o arquivo gravado.
  Todas as sessões fecham pela janela em até 1 s, com exit 0, nenhuma linha de erro, um boot por log e nenhuma
  repetição. As capturas `s1-02-gameplay.png` e `s1-04-reload.png` mostram o Link na casa dele.

  | Linha | Pacotes | Mods carregados (s1/s2) | Fechamento (s) |
  |---|---|---|---|
  | 01 sem coremods | — | 0/0 | 0,3/0,3 |
  | 02 Unbound | U | 1/1 | 0,6/0,6 |
  | 03 NEI | N | 1/1 | 0,3/0,4 |
  | 04 Unbound+NEI | U N | 2/2 | 0,7/0,7 |
  | 05 Unbound+DMR | U D | 2/2 | 0,9/0,9 |
  | 06 NEI+DMR | N D | 2/2 | 0,7/0,7 |
  | 07 os três | U N D | 3/3 | 1,0/1,0 |
  | 08 WW | W | 1/1 | 0,3/0,3 |
  | 09 WW+Unbound+NEI | W U N | 3/3 | 0,6/0,7 |

- **Ciclo de save** (`evidence/release-g/save-cycle-rc2`): o mesmo arquivo passa por c1 (Unbound+NEI, 2 mods), c2 (só
  Unbound, 1 mod) e c3 (NEI reinstalado, 2 mods). As 6 sessões passam nos mesmos critérios da matriz. Em
  `namespaces-cycle.txt`, o `nei.state` gravado na c1 (`83741ffd1c4a`) chega idêntico à c2, com o NEI ausente e o jogo
  gravando 16 vezes, e continua na c3. Os quatro namespaces da fixture (`linkspan-demo.save`, `linkspan.entrances`,
  `linkspan.horse.scene`, `linkspan.scenes`) ficam iguais nas quatro fotos do save. Resultado: `CICLO OK`.
- **Rechecagem estrita** (`evidence/release-g/recheck-rc2.json`): 12/12 configurações aprovadas, conjunto completo.

### Gate OoT (§18) — situação no RC2

Legenda: ✅ comprovado no RC · 🟡 parcial ou comprovado antes do RC · ❌ aberto ou violado.

| Critério | Situação | Evidência / motivo |
|---|---|---|
| overlay instala sobre 9.2.3 fresco | ✅ | `fresh-install-rc2`: `PROVA OK`, 371 comandos de cena em XML |
| sem mods = vanilla | 🟡 | gatilho de áudio MM removido (`fc2709989`); linha 01 da matriz; sem comparação quadro a quadro com o 9.2.3 oficial |
| Unbound é um pacote removível | ✅ | um `.shipmod`; linhas 02 e 03 da matriz |
| NEI é um pacote removível | ✅ | um `.shipmod`; linhas 02 e 03 da matriz; a c2 do ciclo de save tira o NEI sem perder o estado dele |
| NEI sem Unbound | ✅ | linhas 03 e 06 |
| NEI sem SM64/Pikachu/Wolf Link/FleetShipCombo | ✅ | por construção: o pacote não traz expansões (NEI-015 adiada) |
| dependências opcionais em diretório privado | — | não há dependência opcional no RC (NEI-015 adiada) |
| nenhuma ROM SM64; formatos recusados | 🟡 | scanner OK; a recusa de formato não se aplica sem SM64 |
| Unbound sem NEI | ✅ | linhas 02 e 05 |
| ambos juntos | ✅ | linhas 04 e 09 |
| DMR com os dois | ✅ | linha 07 |
| instalar/atualizar pacote não altera `soh.exe` | ✅ | SHA do `soh.exe` conferido em cada uma das sessões |
| nenhuma onda de itens exige função específica no host | 🟡 | as ondas do NEI vão por escape hatch; dívida: `LinkSpan_DynamicMovementInventoryMenuEnabled` (nome do DMR, mecanismo genérico `dpadHudOwned`) e o desvio no `z_kaleido_item.c` |
| cenas e itens por add-ons | ✅ | shovel-demo (item V2), actors demo (Unbound 0.9), cenas do Unbound |
| saves sobrevivem a update, desativação e reinstalação | 🟡 | desativação e reinstalação comprovadas no ciclo de save (`nei.state` idêntico com o NEI ausente, `CICLO OK`); atualização de versão N→N+1 não testada (não há N+1 do NEI) |
| falha de coremod recupera sem apagar arquivos | 🟡 | COREEXT-009 provada antes do RC (`.shiplua-disabled`); não refeita no RC |
| scanner de conteúdo protegido | ✅ | `protected-content.txt` OK |
| testes sem ROM no Windows | ✅ | CTest raiz 67/67 |
| playtest real dos limites do Unbound | 🟡 | Gate E (2026-09-19); as E-fixtures não foram refeitas no RC |
| fixtures de fronteira do Prelude | ❌ | faltam salas > 255, objetos > 128, DynaPoly e água |
| mod Unbound incompatível recusado sem a capability | 🟡 | camada de mod rejeitado fora do VFS (`4c45d5c3a`); sem prova em jogo no RC |
| cenas afetadas por versão resolvem recursos | ❌ | não verificado no RC |
| conflito entre duas cópias da mesma cena antes do gameplay | ❌ | não verificado no RC |
| playtest real de todos os itens NEI | ❌ | só harness; os assets do fork NEI (fonte sem licença) não são distribuídos e não houve controle físico |
| documentação, exemplos, SDK e hashes | ✅ | `docs/writing-mods.md`, `docs/native-providers.md`, `soh/native-sdk/README.md`, `RELEASE.md`, `SHA256SUMS.txt`, `inventory.json` |
| depois do COREEXT-GATE só DLLs recompiladas | ❌ | o host foi recompilado em várias rodadas (CEL-004A, NEI-HOST-*, OOT-NEI-017, rc2) sem RFC de ABI para cada uma |

### Depois do RC2 (2026-10-03): o que mudou nas linhas abertas

Provado em jogo na cópia de smoke do RC2 (`build/oot-upstream-001/smoke-rc2-20261003`, host `E278E9A3` sem
recompilar), com o framework Unbound e a e-fixtures novos, ainda fora de um RC:

| Critério | Antes | Agora | Evidência |
|---|---|---|---|
| conflito entre duas cópias da mesma cena antes do gameplay | ❌ | 🟡 | UNBOUND-018 (framework 0.6.1): delta mesclável, cópia idêntica e `conflito:` no `game.ready`, 3/3 em jogo; regressão em todas as baterias até a 028 |
| cenas afetadas por versão resolvem recursos | ❌ | 🟡 | UNBOUND-019 (0.6.2): cada recurso do XML conferido no VFS no `game.ready`, ausente/recusado em `warning`, resumo com a versão do `oot.o2r`; 4/4 |
| fixtures de fronteira do Prelude | ❌ | 🟡 | UNBOUND-020 a 028: salas até 32 767 (`m19`), DynaPoly 64→128 e 16 384→32 768 (`m15`, 026), água em qualquer sala (`m19`), exits/câmeras (`exits`, `m17`), 8 193 texturas (`m24`), limite do mundo (`edge_*`, `span`), notas de teto e grafo cena ↔ colisão/salas (0.6.3–0.6.8). Objetos > 128 já vinham do Gate E (203 no banco, 602 atores vivos); as fronteiras de 1 024 objetos (M09) e 8 192 atores vivos (M08) só na checagem do `game.ready` (UB020-A) |
| saves sobrevivem a update | 🟡 | 🟡 | atualização N→N+1 do framework (0.6.0→0.6.1) provada na UNBOUND-018 (`update-cycle.ps1`, `ATUALIZACAO OK`); falta um N+1 do NEI |
| depois do COREEXT-GATE só DLLs recompiladas | ❌ | 🟡 | desde o RC2 o host não foi recompilado (018–028 só mudam DLL/fixtures); a RFC 0027 (COREEXT-011) regulariza as rodadas anteriores e define a próxima, com os patches em `evidence/rfc0027-host` à espera da aprovação |

Continua aberto: playtest de todos os itens NEI (físico), licença, segunda família de ROM e, para virar ✅, refazer
essas provas num RC com o framework 0.6.8 (o host pode ficar o mesmo: só a DLL do Unbound muda).

Fora do checklist, e bloqueando a publicação: a **licença** do Link-Span (não há `LICENSE`), do WW Style (o
`NOTICE.md` não concede licença) e dos assets do fork NEI.

## Pendências

**No RC6 (validado, não publicado; o RC8 o substitui, ver OOT-UPSTREAM-003):**
- partes de terceiros sem licença publicada (Shipwright, NEI/skijer, Unbound e Wind Waker Style/roborich): a release
  declara isso e não concede licença sobre o código portado; nenhuma permissão dos autores está registrada aqui;
- o playtest físico de DMR e Mic Ocarina foi no RC5 (2026-10-10); no RC6 esses mods só mudaram o SHA fixado;
- queda do setting do seletor com ele aberto, ainda não exercitada em jogo;
- a wiki publicada (`link-span.wiki`) ainda não recebeu as páginas de `docs/wiki`, agora atualizadas para o RC8.

**No RC3 (2026-10-04)**, decisões do mantenedor:
- licença;
- playtest físico;
- segunda família de ROM;
- push;
- "sem mods = vanilla": aceitar o alargamento do motor ou restringir as diferenças D;
- RFC 0027 inteira e RFC 0028.

Além disso, **commitar a correção de `SceneCommandLoggers.cpp`**, que está no `soh.exe` do RC3 (ver Riscos). As
pendências abaixo são do RC2 e seguem valendo quando não foram resolvidas acima.

- **Licença:** não há `LICENSE` do Link-Span. O `NOTICE.md` do WW Style não concede licença, e os assets do fork NEI
  vêm de fonte sem licença. Bloqueia qualquer publicação; a decisão é do mantenedor.
- **Playtest físico** com controle e microfone (Mic Ocarina, DMR, itens NEI): só o usuário pode fazer.
- **Gate §18:** fixtures de fronteira do Prelude, resolução por versão, conflito de cenas, playtest de todos os
  itens NEI e a regra "só DLLs depois do COREEXT-GATE" (ver tabela).
- **Atualização de pacote** (versão N → N+1) sem teste automatizado; o procedimento manual está no `RELEASE.md`.
- **Harness da matriz** (scratchpad, não versionado: `matrix-run.ps1`, `matrix_check.py`, `save_ns.py`,
  `recheck_rc2.py`): depende do caminho da skill na máquina do usuário. Limites conhecidos da re-revisão: alias por
  junction/symlink gera outra trava, e a captura por `CopyFromScreen` não detecta janela sobreposta sem foco.
- **Push** e eventual PR: só com autorização.

## Riscos

- **RC6 — mesma versão, bytes diferentes:** Unbound Framework 0.6.11, Wind Waker Style 0.1.2 e Shovel Demo 0.1.2 têm
  o número de versão do RC5, mas SHA-256 diferente (RC5 `151bd935…`, `e8ff0af9…` e `50f7941a…`; RC6 `13f5f08e…`,
  `4672d809…` e `3b840a1b…`). Foram recompilados no RC6, e o Shovel agora sai do link-span-examples. Como o RC5 não
  foi publicado, nenhum jogador tem as duas cópias; o próximo RC deve subir a versão de todo pacote que mudar de bytes.
- **RC3: o `soh.exe` tem uma mudança sem commit.** É `soh/soh/resource/logging/SceneCommandLoggers.cpp`, gravada em
  2026-10-03 às 10:25, cujo `.obj` foi recompilado às 10:28 e entrou nos builds seguintes. Ela corrige o crash do
  `ResourceLogging`:
  - o `6fa3f2a2a` (UNBOUND-017) deixou de gravar `polygon1.multi.list` no formato 1, porque gravar ali sobrescrevia
    o `single` da union;
  - o `LogMeshAsXML` do upstream ainda lia `multi.list->unk_00/id` nesse formato, e o ponteiro inválido derrubava o
    jogo;
  - a mudança passa a logar o formato 1 pelo `single`.

  Assim, o crash do RC2 não era bug do upstream, e no RC3 a prova com a opção ligada passou. Até o commit, o host do
  RC3 é o `0ff8f2ff4` mais essa mudança, e a modificação é do usuário (não commitada por mim).
- **Fingerprint do host:** NEI, DMR e Mic fixam o SHA do `soh.exe`. Toda recompilação do host exige reempacotar os
  três (o construtor da release agora recusa se esquecer). Fora do fingerprint, o DMR 0.2.16 carrega sem ícones nem
  sincronia de câmera, e o NEI desliga o fork.
- **Mesma versão, conteúdo diferente:** o NEI 0.3.16 do RC2 difere do RC1 só no `manifest.toml` (fingerprint). O Mic
  0.2.2 difere também na `mic_ocarina.dll`, recompilada contra o host novo (mesmo tamanho; a diferença binária não foi
  analisada). O RC1 não saiu da máquina.
- **Dívida no host:** `LinkSpan_DynamicMovementInventoryMenuEnabled` tem nome de mod (o mecanismo é o genérico
  `dpadHudOwned`), e o desvio fica no `z_kaleido_item.c`. Generalizar por RFC.
- **Logs ruidosos sem mods:** "ShipLua não validou o asset OoT …" (OotWorldAdapter) e o aviso de Soundfont do MM no
  boot.
- **`ResourceLogging` derruba o jogo na casa do Link** (`evidence/release-g/crash-resourcelogging-rc2`): o
  jogo cai no `SOH::LogMeshAsXML` (código do upstream) ao logar mesh tipo 1 formato 1. Com a opção de desenvolvedor
  desligada (padrão) não acontece, e a matriz roda com ela em 0. A prova do overlay continua usando `-ResourceLogging`
  só na cena de abertura. A origem não está isolada: o importador `SetMeshFactory.cpp` tem mudanças locais do
  Link-Span anteriores à fase G. Falta reproduzir no upstream limpo; tarefa à parte sugerida.

## Próxima ação recomendada

Depois do RC6 (não publicado; o publicado é o RC8):

1. Acompanhar o retorno da pre-release `oot-0.5.0-rc8` (issues no BaiterYamato/Shipwright).
2. Publicar a wiki com as páginas de `docs/wiki` do RC8, se o mantenedor autorizar.
3. Próximo RC só com mudança de código, pela pipeline `logs/rc-pipeline-oot-next` (README lá), subindo a versão de
   todo pacote que mudar de bytes.

No RC3:

1. Commitar a correção do logger. Se o commit não for igual ao arquivo atual, o host tem de ser recompilado e
   reempacotado.
2. Playtest físico com `build/release/oot-0.5.0-rc3`.
3. Decidir a licença, a linha "sem mods = vanilla" e as RFCs 0027/0028.

Histórico do RC2:

1. O usuário decide a licença e faz o playtest físico no RC2: instalação em cópia nova da 9.2.3 com o overlay e os
   pacotes de `build/release/oot-0.5.0-rc2`.
2. Abrir as tarefas do que falta no Gate: fixtures do Prelude (salas > 255, objetos > 128, DynaPoly/água), conflito
   de cenas, a RFC que regulariza as recompilações do host depois do COREEXT-GATE e a generalização do
   `dpadHudOwned`.
3. Com o Gate verde e a licença decidida, autorizar o push e a publicação.
