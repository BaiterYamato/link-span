# F5 — HOST-SLIM-001: NEI no host limpo e DMR na ABI 1.4

> Plano de execução, não implementação concluída. Levantamento de 30/10/2026, somente por objetos Git das refs abaixo. Nenhum repositório de entrada foi alterado; não houve commit, build MSVC nem teste em jogo nesta tarefa.

**Veredito:** a remesclagem das duas funções do NEI já entrou na F4 em cascas; falta regenerar e validar o NEI para o executável final, migrar o DMR e seus dois geradores de manifesto para a ABI 1.4, recompilar, empacotar ambos e comprovar a combinação em runtime/jogo.

**Objetivo:** concluir a linha F5 da RFC 0029 sem reintroduzir pontes de conteúdo no exe e sem mudar o layout id. O aceite separa fonte, geração, build, pacote, runtime e controle físico.

**Arquitetura:** o NEI continua copiando funções do fork e desviando o host pelo escape hatch, preso ao SHA-256 do exe. O DMR só consulta dois nomes públicos e passa a depender do layout, sem `soh.symbols` e sem autorização para patches.

**Tecnologia:** Python para extração/imports COFF; C/C++ e MASM x64; VS 2022/v143 e o compilador exato do SDK; CMake; PowerShell para pacotes. MSVC e jogo ficam com o integrador, seguindo o modelo do plano F3.

## 0. Repositórios, refs e limites

| Entrada | Caminho | Ref lida | SHA observado |
|---|---|---|---|
| Host principal | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-host` | `linkspan/agent/HOST-SLIM-001-f4-vanilla` | `759beaf60ef3f98c4c7ffb022bb46cb977a59669` |
| Host Unbound | mesmo caminho | `linkspan/agent/HOST-SLIM-001-f4-unbound` | `ff0cc508f016a552fb7731cc55e4ade0851d3374` |
| Core/documentação | `D:\Desenvolvimento\ship-lua-worktrees\HOST-SLIM-001-core` | `origin/main` | `d9e7089afb7c12e88351b427d672cd6381b43636` |

O gitlink `extern/ship-lua` do host aponta para `ee4d8fb6272ea10048ffa51c6695a5662f7bee02`. Conferido: `NativeProvider.cpp` e `ship_native_abi.h` não diferem do `origin/main` lido. Nenhum fetch foi feito: são os objetos locais dessas refs, sem afirmação sobre CI/PR remoto atual.

Comparação Unbound → Vanilla: **nenhuma diferença** em `soh/native-sdk/nei-core/`, `soh/native-sdk/example/`, `soh/include/`, `soh/soh/native/`, `tools/package-nei-core.ps1` e `tools/build-linkspan-sdk-bundle.ps1`. Não há remesclagem adicional dessas áreas a portar da branch Unbound.

Regras para a execução futura:

- Não editar nem acrescentar headers em `soh/include/`, `soh/soh/native/` ou `extern/ship-lua/include/shiplua/native/`. O glob do layout inclui as três árvores, além do compilador e flags: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/ExportSdk.cmake:7`–22. Se faltar contrato nessas superfícies, registrar bloqueio e parar a fatia dependente.
- Não assumir que a working tree dessas entradas corresponde às refs. O extrator **lê fontes da working tree**, portanto a execução exige cópia/checkout isolado fiel ao SHA aprovado; não apontar `sync.py` à worktree divergente.
- Não reaproveitar DLL do NEI de outro host, mesmo se o layout for igual. Não aceitar `NEI_ALLOW_OVERLAY_CONFLICTS=ON` nem `--permitir-conflitos` no aceite.
- F4/V ainda em integração devem estabilizar antes da extração/pacote finais. Não transformar F5 em implementação de formas/assets do MM ou fechamento da F6.
- A tarefa atual só produz arquivos nesta pasta. Os comandos de build e empacotamento abaixo são instruções futuras para o integrador em ambiente autorizado.

## 1. Estado real e evidências

### 1.1. NEI: origem e significado de “remesclar”

**Origem — feito no código.** Não existe uma cópia independente completa de `z_player.c` versionada na DLL. `sync.py` fixa o fork `c29262b` e sua base `783139310`, copia as árvores do fork e aplica `fork/patches/*.patch`: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/sync.py:26`–28 e :429–445. O overlay compara host atual, base e fork, seleciona funções alteradas e leitoras das tabelas alteradas, mescla e extrai o fecho de dependências: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay.py:3`–11 e :90–136; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/sync.py:344`–375.

As duas funções têm **resoluções manuais de overlay**, e não um patch em `fork/patches/`. O `git grep` das patches não encontrou os nomes delas nem as três pontes. “Remesclar” significa atualizar a resolução de três vias preservando mudanças do host e itens do fork, e recalcular sua assinatura quando algum dos três corpos mudar. Não significa copiar o host puro, apagar os recursos do NEI ou recriar ponte no exe.

| Item | Estado na ref | Evidência e interpretação |
|---|---|---|
| `Player_PlayVoiceSfx` | **Feito no fonte; binário final pendente** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay-merges/z_player.Player_PlayVoiceSfx.c:1`–15. Chama `LinkSpan_PlayerVoiceSfx` antes do ajuste de idade; respeita consumo e preserva `VB_PLAYER_VOICE_SFX` do fork. Não chama `ShipLua_TransformVoiceSfx` nem a voz de transformação do fork. A assinatura atual é `ca8140ab41a7 7f3954336f05 605ad5069db0`, recalculada e válida. |
| Contraparte da voz no host | **Feito** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/src/overlays/actors/ovl_player_actor/z_player.c:1756`–1765 e `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/OotNativeHooksGame.cpp:335`–337. O novo helper existe; encaminha para o despacho genérico. Não é uma chamada a símbolo removido. |
| `Player_PostLimbDrawGameplay` | **Feito no fonte; binário final pendente** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay-merges/z_player_lib.Player_PostLimbDrawGameplay.c:1`–5. Chama `LinkSpan_PlayerLimbDraw`; não contém `ShipLua_DrawMaskTransitionHand/Head`. Conserva os acréscimos do fork, por exemplo haste/arma, captura de mão e desenho de itens (:17–25, :198, :343–349). Assinatura `2d24df1ca871 f8ac85e6787f 2c2690a66ccf`, recalculada e válida. |
| Contraparte de limb no host | **Feito** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/src/code/z_player_lib.c:1803`–1808; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/OotNativeViewGame.cpp:294`–305. O dispatcher oferece a matriz do membro e restaura a pilha depois dos callbacks. |
| Desenho de máscara fora do pós-limb | **Feito no fonte** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay-merges/z_player.Player_DrawGameplay.c:90`–98. A resolução também foi adaptada: usa `LinkSpan_PlayerMaskDraw` e `Player_DrawVanillaMask`, preservando os vetos. Não limitar a auditoria às duas funções citadas na tabela da RFC. |
| Instalação dos desvios | **Mecanismo feito; prova final pendente** | O corpo extraído de player entra na TU da DLL em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/player_unit.c:119`. As tabelas são geradas em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/sync.py:377`–395; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay_glue.cpp:38`–49 resolve cada destino e instala o detour. Contagem e sucesso precisam ser observados no runtime. |

Os hooks mantêm os contratos existentes: voz TRANSFORM/OBSERVE e consumo antes da idade; máscara REPLACE/OBSERVE e `call_original`; limb OBSERVE. Evidências: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/oot_hooks.h:27`–30 e :202–214; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/OotNativeHooks.cpp:203`–211 e :241–251.

O conteúdo já está do outro lado desses hooks: `mm-base` registra `oot.player.voice_sfx` em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/mm-base/provider.cpp:124`; `mm-forms` define o mapa em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/mm-forms/provider.cpp:432`, desenha mão/cabeça pelo callback :469–479 e registra limb/mask em :729–732. Isso comprova o encadeamento do código; **não comprova áudio ou transformação real com `mm.o2r`**.

### 1.2. Extração, imports, stubs e keep

| Item | Estado | O que ainda fazer / o que não remover |
|---|---|---|
| `extract.txt` | **Feito; nenhuma limpeza mecânica necessária** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/extract.txt:1`–8 explica sementes/imports, e :12 contém as sementes explícitas do player. As duas funções chegam pela seleção automática do overlay; nenhuma das três pontes aparece nas 22 linhas do arquivo. Não acrescentar sementes duplicadas. |
| `stub-names.txt` | **Feito para retirada das pontes** | Não há entrada para nenhuma das três pontes. `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/stub-names.txt:1`–3 descreve stubs de subsistemas ausentes; :64–79 ainda contém nomes do fork como `TransformMasks_TryPlayMmVoice`. Não confundir esses stubs com símbolos do exe removidos, nem apagá-los em bloco. `sync.py:473`–486 já filtra os nomes que a extração passou a definir de verdade. |
| `nei_host_overrides.h` | **Sem adaptação adicional identificada** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/nei_host_overrides.h:13`–23 remapeia nomes do upstream e recursos; não há alias/reintrodução das três pontes. Nenhuma mudança proposta. |
| `gen_imports.py` | **Gerador existente; regeneração por fazer para o alvo final** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/gen_imports.py:215`–268 usa indefinidos COFF menos definições da DLL, cruzados com `soh.symbols`; :183 grava o SHA do arquivo de símbolos no fonte gerado. Não tem lista fixa das pontes a limpar. Dados, nomes qualificados e endereços de função ausentes geram erro; nome comum sem entrada pode ir para `ignored` (:258–262) e só estourar no link. Exigir também link limpo e auditoria dos relatórios. |
| Imports gerados antigos | **Obsoletos para o novo exe** | Regenerar `nei_host_imports.asm`, `nei-fork/nei_host_imports.c` e `.txt` a partir dos novos `.obj` e símbolos; regra em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/CMakeLists.txt:70`–80. Um thunk antigo de ponte removida falharia na resolução/link; um símbolo reintroduzido como stub esconderia o erro de arquitetura. Buscar as três pontes também nos gerados. |
| Keep e catálogo do host | **Limpeza de fonte feita; validade do PE final por provar** | As pontes estão ausentes de `tools/host-keep-extra.txt` (:1–26) e `soh/soh/native/OotNativeHostKeep.c` na ref. Não remover hooks genéricos válidos. O keep ainda retém `Actor_UpdateAll`: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/OotNativeHostKeep.c:373`. Rodar o gerador em `--check`, o teste de sincronismo e o teste do PE final; alvos em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/ExportSdk.cmake:173`–184. |

A busca `git grep` das três pontes retornou zero ocorrências em NEI, `soh/src`, `soh/soh` e keep-extra da ref. Não há hunk justificado para `extract.txt`, `stub-names.txt`, keep ou as resoluções manuais.

**Geração do overlay — parcial no ciclo F5.** O código já invalida a extração quando fontes mesclados, símbolos, patches ou resoluções mudam: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/CMakeLists.txt:19`–48; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/sync.py:411`–425. Conflito faz a função ficar sem desvio; sem a opção permissiva, o configure falha (:456–463). A assinatura de uma resolução só vale para os três corpos que a originaram: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/nei-core/fork/overlay.py:44`–61. Aqui foram verificadas 18 assinaturas das três unidades inspecionadas, todas válidas; isso não equivale a executar a extração completa.

**`Actor_UpdateAll` — reextração obrigatória.** O NEI também substitui essa função; o host tem `LinkSpan_ActorSettleDamage` em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/src/code/z_actor.c:2739` e :2781. A mescla isolada dos três corpos feita nesta análise foi limpa e preservou ambas as chamadas. O incidente real está descrito em `origin/main:coordination/handoffs/HOST-SLIM-001.md:1133`–1135 e :1193–1194: DLL antiga desligava o dano estendido mesmo com pacote novo. Verificar o corpo **extraído**, sua tabela e o desvio instalado, além das duas funções do player.

**Empacotamento NEI — por fazer para o alvo final.** `linkspan/agent/HOST-SLIM-001-f4-vanilla:tools/package-nei-core.ps1:100`–111 obtém layout/hash e recusa símbolos de outro exe; :124 copia `soh.symbols` para a entrega; :137–140 gera provider ABI 1.3 com fingerprint. Essa permanência em 1.3 é válida: F5 pede 1.4 para o DMR, e o NEI continua precisando de patches privados. O empacotador não recompila nem prova a origem da DLL escolhida.

### 1.3. DMR: ABI e dependências

**Localização confirmada:** `soh/native-sdk/example/`, projeto `DynamicMovementRemake`, alvo `dynamic_movement_remake`; `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/CMakeLists.txt:1`–10. Identidade `baiteryamato.dynamic_movement_remake`, versão `0.2.24`: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/manifest.toml:1`–8.

| Item | Estado | Evidência |
|---|---|---|
| Manifesto | **Por fazer** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/manifest.toml:14` declara `1.3`; já tem `core_extension` e `pre_game` (:7–8), mas falta gerar `[provider].host_layout_id`. |
| Descriptor | **Compatível no fonte, reconstrução pendente** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/provider.cpp:1370`–1373 usa `SHIP_NATIVE_ABI_MAJOR/MINOR`, sem literal 1.3. O header do core já define minor 4 em `origin/main:include/shiplua/native/ship_native_abi.h:20`; também confirmado no gitlink do host. Não mudar header nem layout do descriptor. O rascunho só acrescenta uma asserção para recusar SDK anterior à 1.4. |
| Resolução de símbolos | **Ainda pelo gate antigo; migração por fazer** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/provider.cpp:1248`–1261 só resolve `gItemIcons` e `OLib_Vec3fDiffToVecSphGeo`, com guard minor ≥3 e fallback se indisponíveis. Não há `install_patch`, `remove_patch` ou nome qualificado no provider. |
| Nomes públicos | **Feito no host** | Declarações diretas: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/include/variables.h:106` (`gItemIcons[256]`) e `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/include/functions.h:1051`. Catálogo: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/soh/native/OotNativePublicSymbols.c:2098` e :1286. Não falta export nem header novo. |
| Gerador local do manifesto | **Por fazer** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/CMakeLists.txt:25`–34 exige exe, calcula SHA e acrescenta fingerprint. Trocar por `OOT_NATIVE_LAYOUT_ID`, mantendo a dependência do manifesto para regeneração. |
| Gerador no bundle do SDK | **Por fazer** | `linkspan/agent/HOST-SLIM-001-f4-vanilla:tools/build-linkspan-sdk-bundle.ps1:227`–230 recria o CMake do exemplo e ainda injeta fingerprint/ABI 1.3. Usar `LinkSpanOotSdk_LAYOUT_ID` (:170). Corrigir só o CMake local deixaria o SDK distribuindo o caminho antigo. |

Hoje o DMR depende **operacionalmente** do gate privado/hash/`soh.symbols` por declarar 1.3, embora os dois nomes sejam públicos; a falta deles degrada ícones e sincronização da câmera, sem impedir o movimento. Depois da migração, o catálogo funciona sem `soh.symbols` e sem fingerprint se manifesto 1.4 + core extension pre_game + layout id de 64 hex igual ao host: `origin/main:src/native/NativeProvider.cpp:66`–74. O resolver tenta público primeiro, zera saídas em falha e só abre privado com seu gate separado (:544–573); patches seguem dependentes de fingerprint (:577–583). Layout errado no manifesto fecha a resolução pública; não pressupor rejeição automática de toda a carga, pois o DMR tem fallback. Layout incompatível dos serviços/structs é recusado pelo próprio Init: `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/provider.cpp:1222`–1227.

**Cuidado encontrado na fixture — por fazer antes do teste da migração.** `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/tests/EngineTests.cpp:92`–95 anuncia uma tabela `gItemIcons` de **um** elemento. Quando o DMR começar a resolver pelo público, `ItemSelectorIcons` indexará itens como arco=3 e hookshot=10 (:4812–4819); o acesso é direto em `linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/example/item_icons.h:17`. Isso cria risco concreto de acesso fora da fixture. O rascunho amplia para 256 elementos, mantendo o sentinela já usado no public-probe e zerando os demais. Faltará acrescentar a prova positiva dos caminhos de ícone e da geometria, porque o catálogo de teste atual só oferece `Math_SinS` e ícones (:93–97).

## 2. Divisão e ordem do trabalho

| Ordem | Responsável | Unidade verificável | Depende de |
|---|---|---|---|
| 0 | Integrador | T0: congelar base e identidade | F4/V integradas conforme governança |
| 1 | Implementador | T1: migrar DMR + geradores + capacidade da fixture | T0 |
| 2 | Implementador/integrador | T2: prova automatizada da ligação pública do DMR | T1 |
| 3 | Integrador MSVC | T3: host/SDK/símbolos definitivos | T2 e qualquer ajuste de teste |
| 4 | Integrador MSVC | T4: extração/overlay/imports e build NEI | T3 |
| 5 | Integrador MSVC | T5: build DMR e SDK distribuível | T3 |
| 6 | Integrador | T6: empacotar e auditar as identidades | T4 + T5 |
| 7 | Integrador/jogador | T7: runtime e jogo | T6 |
| 8 | Integrador | T8: evidências e fechamento da F5 | T7 |

São unidades de execução futura; não foram disparados subagentes nesta análise. T4 e T5 podem ser independentes após T3, desde que não escrevam nos mesmos diretórios.

## T0 — Congelar o alvo e o layout

**Arquivos/entradas:** refs acima, gitlink, `soh/native-sdk/ExportSdk.cmake`, SDK/build de destino e histórico de integração. **Exige:** Git/CMake; MSVC só para produzir o build.

- [ ] Confirmar o SHA aprovado que reúne F4/V e escolher diretórios isolados para host, NEI, DMR e runtime. Não usar como fonte a working tree divergente.
- [ ] Registrar SHA do host e do core, compilador, arquitetura, flags e layout completo do SDK de referência.
- [ ] Comparar os headers que entram no layout com a base aprovada. Se qualquer tarefa exigir mudá-los, registrar bloqueio com arquivo/contrato faltante.
- [ ] Confirmar que `c29262b` e `783139310` existem no Git usado pela extração. Se faltarem, registrar a entrada ausente para o integrador; não trocar o fork silenciosamente.

**Prova:** ficha de identidade reproduzível, diff de headers vazio e fontes extraíveis da ref escolhida.

## T1 — Migração mecânica do DMR

**Arquivos:** `soh/native-sdk/example/{manifest.toml,CMakeLists.txt,provider.cpp}`, `tools/build-linkspan-sdk-bundle.ps1`, `soh/native-sdk/tests/EngineTests.cpp`. **Exige:** edição e checks estáticos; MSVC em T2/T5.

- [ ] Revisar e aplicar o rascunho `f5.patch` sobre a base exata; se a base tiver avançado, refazer `git apply --check` antes da aplicação.
- [ ] Confirmar ABI 1.4, layout de 64 hex dentro de `[provider]`, ausência de `host_fingerprints` no DMR gerado e ausência de chamadas privadas/patches no provider.
- [ ] Gerar manifestos pelo CMake local e pelo template do bundle; exigir mesmo ABI/layout e o descriptor com minor ≥4.
- [ ] Manter o SHA do host no metadado geral do SDK: ele continua útil aos consumidores privados. Só tirar a autorização por hash do manifesto do DMR.

**Prova:** TOML parseável nas duas saídas, layout igual ao SDK e SDK antigo rejeitado pela asserção em compilação. Nenhum header do layout alterado.

## T2 — Exercitar o DMR pelo catálogo público

**Arquivos:** `soh/native-sdk/tests/EngineTests.cpp`, `soh/native-sdk/tests/test_independent.py` e, se necessário, teste isolado na mesma área de testes. **Exige:** MSVC x64, sem jogo.

- [ ] Usar a fixture com 256 posições; fornecer caminhos de ícone controlados para IDs realmente usados e uma função de geometria controlada com a assinatura existente. Manter a prova pública anterior válida.
- [ ] Rodar o DMR real sem backend privado/`soh.symbols`, com ABI 1.4 e layout correto: exigir os dois nomes resolvidos, caminho de ícone retornado e chamada da geometria observável na retomada da câmera. Movimento sozinho não prova essa migração.
- [ ] Repetir com fingerprint do host diferente e mesmo layout: a resolução pública deve continuar funcionando. Usar processos/policies de teste; não reconstruir exe só para fabricar esse negativo.
- [ ] Com layout declarado ausente/diferente, verificar `UNSUPPORTED` e endereço zero no caminho público; registrar o fallback do DMR. Manifesto malformado e ABI incompatível devem seguir as rejeições já existentes.
- [ ] Rodar `oot_native_independent_mod` e `oot_native_public_probe`; garantir que o novo teste não passe apenas por ícones vazios ou câmera em fallback.

**Prova:** relatório com os dois nomes e os gates observados; sem OOB na fixture e sem acesso ao privado. A fixture de um elemento é a causa identificada; a regra de regressão é que uma tabela pública fake ofereça toda a faixa que o consumidor indexa. Essa análise segue o diagnóstico da skill backprop; nenhuma escrita em SPEC de outro repositório foi feita.

O teste independente atual só compila velocidades 7/10, executa o provider e confere que o exe não mudou (`linkspan/agent/HOST-SLIM-001-f4-vanilla:soh/native-sdk/tests/test_independent.py:39`–54). É aproveitável, mas não prova sozinho os dois símbolos reais nem produz o pacote completo de assets.

## T3 — Produzir host, SDK e símbolos finais

**Arquivos/saídas:** build do host, `native-sdk/Release/OotNativeSdk.cmake`, `oot_layout_id.h`, `soh.exe`, `soh.symbols`. **Exige:** MSVC/SDK do host; sem jogo.

- [ ] Configurar e compilar todos os alvos de teste selecionados antes do CTest. Usar VS 2022 x64/v143 ou o gerador MSVC já validado pela CI, mantendo versão e flags do SDK.
- [ ] Rodar `python tools/gen-host-keep.py --root . --check`, `linkspan_host_keep_generator`, `linkspan_host_keep_sync` e `linkspan_public_symbols_pe`. Não aceitar teste de PE pulado como prova de retenção.
- [ ] Gerar símbolos **depois** do último link do exe; verificar `linkspan-symbols 1` e `sha256 <hash do exe>`, ausência das pontes e presença dos hooks/imports genéricos necessários.
- [ ] Confirmar layout igual ao congelado em T0 e registrar novo SHA-256 do exe. A partir daqui, qualquer rebuild do host reinicia T3–T7 para a identidade final.

Comandos de referência, a ajustar ao diretório de build escolhido:

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64 -T v143
# Compilar soh e os executáveis listados por ctest -N para o filtro abaixo.
cmake --build build/x64 --config Release --target soh --parallel
cmake --build build/x64 --config Release --target soh_symbols
ctest --test-dir build/x64 -C Release -R "oot_native|linkspan_host_keep|linkspan_public_symbols_pe|oot_actor_health" --no-tests=error --output-on-failure
```

O alvo de símbolos separado e o fluxo MSVC seguem `origin/main:docs/plans/host-slim-001/plano-f2-host.md:41`–49. Não fixar como atual nenhum hash/layout abreviado dos handoffs históricos.

## T4 — Reextrair e compilar NEI do alvo final

**Arquivos:** inicialmente nenhum fonte a mudar; se um corpo divergir, somente `soh/native-sdk/nei-core/fork/overlay-merges/*.c` e eventuais adaptações internas da DLL justificadas pelo relatório. **Exige:** Python, objetos Git do fork, MSVC C/C++/MASM e símbolos de T3.

- [ ] Configurar em diretório novo, com `OOT_NATIVE_SDK` e `NEI_HOST_SYMBOLS` explícitos. `NEI_ALLOW_OVERLAY_CONFLICTS=OFF`.
- [ ] Conferir o configure/`sync.py`: patches aplicados, zero conflitos, `Player_PlayVoiceSfx`, `Player_PostLimbDrawGameplay`, `Player_DrawGameplay` e `Actor_UpdateAll` presentes nas sobreposições. Salvar `overlay-report.txt`; comparar entradas e exclusões com o baseline, sem impor a contagem antiga 323/319/316 como contrato.
- [ ] Se houver resolução velha, usar `overlay_conflito.py` para as três versões e revisar semântica antes de atualizar a assinatura. Não basta trocar a primeira linha para aceitar um corpo antigo.
- [ ] Conferir os corpos extraídos: voz com hook/consumo/idade, limb com callback, máscara via dispatcher, `Actor_UpdateAll` com as duas chamadas de settle e mudanças recentes de lifecycle.
- [ ] Compilar `linkspan_nei_core`, regenerando `nei_host_imports.asm`, `nei_host_imports.c` e `nei_host_imports.txt` dos objetos atuais. Buscar as três pontes em extraídos, tabelas de import e relatório; exigir zero. Link sem indefinidos; revisar `ignored`.
- [ ] Rodar os seis testes `linkspan_nei_*_tests` registrados em `soh/native-sdk/nei-core/CMakeLists.txt:125`–130. Arquivar log, mapa da DLL, SHA da DLL e relatórios de geração.

```powershell
cmake -S soh/native-sdk/nei-core -B build/f5-nei -G "Visual Studio 17 2022" -A x64 -T v143 -DOOT_NATIVE_SDK=<SDK-final>/OotNativeSdk.cmake -DNEI_HOST_SYMBOLS=<host-final>/soh.symbols -DNEI_ALLOW_OVERLAY_CONFLICTS=OFF
cmake --build build/f5-nei --config Release --parallel
ctest --test-dir build/f5-nei -C Release --no-tests=error --output-on-failure
```

**Prova:** relatório completo, tabela dos quatro detours exigidos, imports sem pontes, link e testes verdes. Se faltar um helper genérico em `soh.symbols`, investigar geração/retenção existente antes de qualquer mudança; necessidade de header público é bloqueio.

## T5 — Compilar DMR e validar o bundle do SDK

**Arquivos/saídas:** `soh/native-sdk/example/`, `tools/build-linkspan-sdk-bundle.ps1`, builds isolados do DMR e do exemplo exportado. **Exige:** MSVC do SDK; sem jogo.

- [ ] Configurar DMR contra o SDK de T3, compilar e rodar `dynamic_movement_icon_tests`, `dynamic_movement_camera_tests` e `dynamic_movement_menu_tests`.
- [ ] Inspecionar `ShipNative_Query`/prova de carga para confirmar minor ≥4; conferir manifesto final com ABI 1.4 e layout completo do SDK.
- [ ] Gerar o bundle, extrair em pasta de teste e compilar `examples/example` contra o SDK exportado. Auditar o manifesto dessa segunda rota.
- [ ] Registrar a limitação do bundle: os assets do HUD não acompanham o exemplo; não entregar esse build como DMR completo. O template já explicita essa limitação em `tools/build-linkspan-sdk-bundle.ps1:232`.

```powershell
cmake -S soh/native-sdk/example -B build/f5-dmr -G "Visual Studio 17 2022" -A x64 -T v143 -DOOT_NATIVE_SDK=<SDK-final>/OotNativeSdk.cmake
cmake --build build/f5-dmr --config Release --parallel
ctest --test-dir build/f5-dmr -C Release --no-tests=error --output-on-failure
```

## T6 — Empacotar NEI e DMR e auditar a entrega

**Arquivos:** `tools/package-nei-core.ps1`, `tools/package-linkspan-mod.ps1`; manifestos, DLLs e assets dos dois builds. **Exige:** PowerShell; builds/inputs completos de T4/T5.

- [ ] Rodar o empacotador NEI com `ProviderDirectory`, `HostExecutable`, `HostSymbols`, `LayoutHeader`, `AssetsDirectory` e saída explícitos. Não depender dos defaults que apontam para outros builds.
- [ ] Verificar DLL do NEI igual à produzida em T4, fingerprint igual ao exe de T3 e cabeçalho de `soh.symbols` correspondente. Instalar os símbolos ao lado do exe no runtime isolado, como pede o pacote.
- [ ] Verificar se os assets previamente validados do NEI estão disponíveis. O script exige o manifesto de assets e a validação de origem/base/MM (`tools/package-nei-core.ps1:55`–65). Se faltarem, registrar bloqueio de empacotamento; não remover essa exigência nem buscar ROM/assets nesta tarefa.
- [ ] Empacotar `build/f5-dmr/mod` pela ferramenta genérica, incluindo `assets/`, Lua, licença e DLL. Conferir ABI 1.4, `host_layout_id` igual ao SDK, sem fingerprints no DMR e sem cópia de `soh.symbols` exigida por ele.
- [ ] Registrar lista de entradas e SHA-256 de exe, símbolos, DLLs e pacotes; testar rejeição de símbolos/hash trocados no NEI em cópia descartável.

**Prova:** dois pacotes auditados para este ciclo. A migração libera o **DMR 1.4** da obrigação de reempacotar a cada novo exe de mesmo layout; ainda deve ser recompilado/reempacotado nesta entrega para incorporar a migração. O **NEI** continua exigindo reextração/rebuild e novo fingerprint a cada host. Não confundir layout estável com hash estável.

## T7 — Prova no runtime e no jogo

**Arquivos:** pasta jogável isolada e relatório com identidades/cenários. **Exige:** jogo/assets autorizados; controle físico nos cenários marcados.

- [ ] Sem mods: inicialização, save de teste e movimentação vanilla; confirmar que não entrou conteúdo MM no exe/entrega por esta fatia.
- [ ] DMR sozinho, sem `soh.symbols`: carregar, confirmar ícones reais e retomada da câmera depois de menu, mira e transição. O teste positivo deve evitar aceitar o fallback como sucesso.
- [ ] NEI sozinho, com símbolos do mesmo exe: confirmar status de imports e de todos os detours, quatro funções exigidas, inventário/equipar, arma na mão e máscara vanilla. Zero recusados; somente “mod carregou” não basta.
- [ ] NEI + DMR: voz vanilla em criança/adulto, máscara Keaton/Bunny Hood, espada/escudo, ícones do seletor, itens estendidos, mira, troca de cena e reset. Confirmar retomada da câmera e input com **controle físico**.
- [ ] Dano estendido com NEI + consumidor de atributos/vida já validado: reproduzir redução da vida acima de 255 ou do cenário de regressão conhecido e lifecycle por troca de cena; comprovar que o `Actor_UpdateAll` da DLL mantém o settle.
- [ ] Com fixture de hook: comprovar voz substituída/consumida e fallback quando o observador/transformador sai; máscara replace/call_original e callback de mão/cabeça sem dupla execução. Pode usar fixture privada de teste com os headers existentes; não criar contrato novo no host.
- [ ] Prova real de voz MM e coreografia exige `mm-base`/`mm-forms` e `mm.o2r`. Se indisponíveis, deixar essa prova explicitamente pendente, separada da retirada das pontes e da migração DMR. As cascas seguem fora do RC conforme `origin/main:rfcs/0029-host-limpo-conteudo-mm-em-mods.md:263`–265 e :284–286.

**Prova:** logs, capturas e resultado por cenário com SHA do exe/DLL/pacote. Harness, mocks e build não substituem áudio real nem controle físico.

## T8 — Fechamento e critérios de pronto

**Arquivos futuros de governança:** `rfcs/0029-host-limpo-conteudo-mm-em-mods.md`, plano F5 no core, `coordination/handoffs/HOST-SLIM-001.md` e evidências vinculadas, pelo integrador autorizado.

- [ ] Atualizar a RFC: remesclagem adiantada na F4; geração/build/pacote finais e DMR 1.4 efetivamente validados. Não deixar a tabela parecer que as pontes ainda estão no NEI.
- [ ] Registrar fonte/SHA, layout completo, fingerprint final, versão do compilador, contagens de overlay/imports/desvios e hashes dos dois pacotes.
- [ ] Separar no handoff: implementado, build/testes, empacotado, runtime, jogo/controle físico, CI e publicação. Registrar qualquer prova MM pendente sem chamá-la de concluída.
- [ ] Só promover F5 quando o escopo de aceite acordado estiver comprovado. Nenhum commit, push, merge, tag, release ou RC é autorizado por este plano.

## 3. Rascunho `f5.patch`

Base: `linkspan/agent/HOST-SLIM-001-f4-vanilla` em `759beaf60ef3f98c4c7ffb022bb46cb977a59669`. Cinco arquivos; nenhum header do layout, alteração do exe ou remoção de conteúdo NEI.

| Hunk | Efeito |
|---|---|
| `example/manifest.toml` | Troca declaração de ABI 1.3 para 1.4; mantém identidade, versão e fase. |
| `example/CMakeLists.txt` | Retira requisito/hash do exe; exige layout do SDK e gera `host_layout_id` em `[provider]`; mantém regeneração por mudança do manifesto. |
| `example/provider.cpp`, comentário/guard | Documenta o caminho público e usa minor ≥4 na resolução, preservando os dois nomes e seus fallbacks. |
| `example/provider.cpp`, query | Asserção de compilação para SDK minor ≥4; descriptor continua usando as macros existentes, sem mudança estrutural. |
| `build-linkspan-sdk-bundle.ps1` | Faz o CMake do DMR exportado gerar o mesmo contrato 1.4 pelo layout do SDK distribuído. |
| `tests/EngineTests.cpp` | Amplia a tabela fake de ícones de 1 para 256, tamanho declarado pelo host; preserva o sentinela do public-probe e zera as outras posições. |

Não inclui prova positiva da geometria/ícones, atualização de docs históricas, bump de versão de distribuição ou qualquer remesclagem especulativa. Isso fica nas tarefas correspondentes; rascunho aplicável não significa migração validada no binário.

## 4. Verificações executadas nesta análise

- `git show/grep/ls-tree/diff` nas refs fixadas; comparação das áreas NEI/DMR/contratos entre Unbound e Vanilla sem diferença. Busca das pontes com zero ocorrências nos caminhos auditados.
- Recalculadas as assinaturas de 18 resoluções dos três fontes copiados, incluindo as duas exigidas e `Player_DrawGameplay`: todas válidas. Evidência: `_evidencias/assinaturas.json`.
- `git merge-file -p` sobre cópias dos três corpos de `Actor_UpdateAll`, nesta pasta: saída limpa, duas chamadas `LinkSpan_ActorSettleDamage` preservadas. Nenhum estado Git foi alterado.
- Os cinco arquivos de `_patch-base/` foram conferidos novamente contra os blobs da ref, com normalização apenas de EOL. `git -C <esta-pasta>/_patch-base apply --check --verbose <esta-pasta>/f5.patch`: **exit 0**, cinco arquivos verificados. Não foi executado `git apply` sem `--check`.
- Os blocos de geração dos dois manifestos modificados foram executados com `cmake -P`, usando layout sintético de 64 hex. Parse TOML confirmou ABI 1.4, core extension/pre_game, layout dentro de `[provider]` e ausência de fingerprints; as duas saídas são idênticas. Evidência: `_validacao/resultado.json`.
- Parser PowerShell aceitou o script de bundle modificado sem erro. O helper `_preparar_patch.py` permite reproduzir geração/check do patch e os dois checks de manifesto.
- Não executados: configure completo com SDK real, MSVC/MASM/link, CTest, retenção no PE final, empacotamento com assets nem jogo. `cl` e `ml64` não estavam no PATH desta sessão; isso não afirma ausência de instalação do VS.

**Riscos e pendências reais:** novo host exige nova geração do NEI; flags/compilador também afetam layout; os fallbacks do DMR podem mascarar falha do catálogo; o status do overlay pode mostrar recusados sem derrubar toda a extensão; empacotamento não atesta origem da DLL; assets/MM e controle físico precisam de ambiente próprio. Não há bloqueio de contrato/header identificado no código atual.
