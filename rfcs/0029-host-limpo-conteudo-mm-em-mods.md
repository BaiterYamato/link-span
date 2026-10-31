# RFC 0029 — Host limpo: conteúdo do MM em mods

Status: aceita em 19/10/2026 (HOST-SLIM-001). F1 implementada no core; F2 no host. Em 27/10, o conteúdo do MM saiu
do exe como cascas de mod fora do RC (F3/F4 parciais, ver as notas de implementação). F5 (NEI) concluída em 30/10;
F6 em andamento: em 31/10 o NEI ganhou o perfil `oot` sem MM, e falta a bateria do RC11.
Inventários e contratos propostos: [docs/plans/host-slim-001](../docs/plans/host-slim-001/README.md).

## Motivação

O `soh.exe` do Link-Span deve levar só o que um modloader precisa para modificar o jogo, no modelo do Forge e do
NeoForge: loader, API, hooks e serviços genéricos. Conteúdo de um mod não entra no executável.

Medido contra o upstream `ecd889c20`, o host do RC10 (`e9da99b515`) tem +97 mil linhas em 904 arquivos. A maior
parte é papel de modloader (bootstrap, `soh/soh/native`) ou não entra no exe (`soh/native-sdk`, ~47 mil). Mas o
exe também compila conteúdo do Majora's Mask:

- motor de áudio do MM (`soh/soh/mmaudio`, ~7,9 mil linhas) ligado ao funil de som em `OTRGlobals.cpp`;
- forma Goron (`soh/soh/mmform`, ~2,5 mil) e a parte Goron do corpo customizado em `ShipLuaBootstrap.cpp` (bola,
  espinhos, ground pound, socos, escudo, tambores, água/void, coreografia da máscara);
- bonecos (`ShipLuaPuppet.*`: Link criança e estátua da Elegia) na lista de atores do host;
- a política de montagem do `mm.o2r` (`MmCrossWorldArchive`, `MountCrossWorldArchives`), incluindo a tradução de
  display lists do MM aplicada a qualquer arquivo de mod;
- a política do Unbound no bootstrap (ID do framework, leitura de `unbound.json`, ordem das camadas).

Sem mods tudo isso fica desligado pela chave da OOT-VANILLA-001, mas está compilado.

Um segundo problema: hoje um mod nativo que só **chama** uma função do jogo precisa do escape hatch da RFC 0023 e
fica preso ao SHA-256 do `soh.exe`. Mods de Java (Fabric, Forge) chamam métodos do jogo pelo nome e só dependem da
versão do jogo. Para o conteúdo sair do exe sem que cada mod novo fique preso ao hash, a chamada por nome precisa
de um gate próprio.

## Decisões

### 1. O que fica no exe e o que sai

| Fica no exe | Sai para mod |
|---|---|
| Loader, menu de mods, hotkeys, HUD | Motor de áudio do MM e o mapa de voz |
| Adaptador do jogo, serviços e hooks da API | Forma Goron inteira e a coreografia da máscara |
| Extensões de motor genéricas: mundo amplo, colisão ampliada, materiais animados, flags de cena estendidas, texto estendido, luz, música de cena por caminho, importadores de recurso | Bonecos (Link criança, estátua da Elegia) |
| Corpo customizado genérico (esqueleto, animações, segmentos, lifecycle) | Localização do `mm.o2r`, prefixo `mm/`, aliases e tradução de display lists do MM |
| `set_mask`, `attach_model`, `set_held_item_model`; primitivas do Link (pulo, velocidade, peso, imunidade, câmera, luz); troca de item recebido | Política do Unbound no bootstrap (vai para o `unbound-core`) |

Extensões de motor ficam porque qualquer mod de mapa grande ou de cena no formato estendido depende delas; mover
para uma DLL obrigaria o escape hatch e prenderia o Unbound ao hash.

### 2. Ligação por nome presa ao layout (ABI 1.4)

O escape hatch passa a ter dois gates independentes:

| Operação | Gate |
|---|---|
| Chamar função ou ler variável declarada diretamente em `soh/include/functions.h` ou `variables.h` | **layout id** declarado no manifesto igual ao do host |
| Desviar função (`install_patch`/`remove_patch`) e resolver símbolo fora desses headers (`static`, `soh.symbols`) | SHA-256 do executável em `host_fingerprints`, como na RFC 0023 |

- O layout id já cobre `soh/include/*.h` (`soh/native-sdk/ExportSdk.cmake`), então as assinaturas públicas estão
  dentro do contrato.
- O host compila um **catálogo público** nome → endereço, gerado só das declarações diretas dos dois headers. Hoje
  `OotNativeHostKeep.c` é um array anônimo que só impede o LTCG de descartar corpos, e o endereço vem do
  `soh.symbols`. O catálogo novo não lê `soh.symbols`, PDB nem o hash do exe.
- Adesão explícita: manifesto `abi_version = "1.4"`, descriptor com minor ≥ 4, `kind = "core_extension"`,
  `load_phase = "pre_game"` e o campo novo `[provider].host_layout_id` (64 dígitos). O core compara o layout antes
  de consultar o catálogo; ausente ou diferente, `UNSUPPORTED`.
- Um endereço obtido pelo caminho público não autoriza patch. `host_fingerprints` passa a ser aceito em 1.3 e 1.4
  e só autoriza o caminho privado e os patches.
- Saída zerada em falha; nome qualificado (`arquivo!nome`) nunca entra pelo caminho público.
- `ShipNativeRuntime` e `ShipNativeDescriptor` não mudam de layout; a 1.4 muda só a semântica do `resolve_symbol`.

Efeito nos mods de hoje: o **DMR** (resolve só `gItemIcons` e `OLib_Vec3fDiffToVecSphGeo`, sem patch) se solta do
hash ao migrar para 1.4. **NEI** (323 desvios) e **Mic Ocarina** (3 desvios e 2 variáveis `static`) continuam
presos ao hash.

### 3. Hooks genéricos novos

Pontos nativos (RFC 0021). Payloads com `size`; ponteiros válidos só durante o callback.

| Ponto | Modo | Onde dispara | Para quê |
|---|---|---|---|
| `oot.audio.mix` | transform, observe | `AudioPlayer_Play`, antes de `AudioPlayerPlayFrame`, **na thread de áudio** | Somar PCM próprio (32 kHz, estéreo, s16 intercalado) |
| `oot.player.voice_sfx` | transform, observe | `Player_PlayVoiceSfx`, antes do ajuste de idade | Trocar ou consumir a voz do Link |
| `oot.player.roll_allowed` | transform (só veto) | `Player_TryRoll` | Vetar o rolamento; `set_roll_blocked` passa a usar este ponto |
| `oot.player.ledge_grab_allowed` | transform (só veto) | ação de agarrar borda, junto do veto existente | Vetar agarrar borda |
| `oot.player.action` | replace, observe | `VB_EXECUTE_PLAYER_ACTION_FUNC` em `Player_Update` | Assumir só a ação do Link, sem trocar o update inteiro |
| `oot.player.update_end` | observe | fim de `Player_Update`, no lugar de `OnPlayerUpdate` | Acompanhar o Link depois do update |
| `oot.player.mask_draw` | replace, observe | `VB_DRAW_PLAYER_MASK` | Omitir ou trocar a máscara desenhada |
| `oot.play.destroy` | observe | antes de liberar o `PlayState` | Restaurar estado enquanto os ponteiros valem |

`oot.audio.mix` não usa o `NativeHookRegistry::Dispatch` atual (que recusa outra thread): ganha um despacho próprio
com lista imutável publicada pela thread do jogo, sem Lua, sem alocação por bloco e com espera de quiescência antes
do unload. Sem registro: zero cópia e zero chamada.

### 4. Versões novas de serviços

| Serviço | Versão | Acrescenta |
|---|---|---|
| `linkspan.oot.player-body` | V1 (novo) | O corpo customizado para DLLs, compartilhado com a API Lua; controlador exclusivo; consultas de animação (iniciar, existe, frame atual, último frame); root motion XZ; callback por membro |
| `linkspan.oot.skeletons` | V2 | Esqueleto com animação do Link (`SkelAnime_InitLink`), recursos tipados e callback por membro |
| `linkspan.oot.movement` | V3 | Assumir e soltar a ação do Link com arbitragem de dono; restaurar Idle e a animação vanilla |
| `linkspan.oot.camera` | V2 | Rotação (roll), epoch e restauração |
| `linkspan.oot.colliders` | V2 | Quads de ataque e de escudo do Player |
| `linkspan.oot.resources` | V4 | Montagem de O2R sob prefixo, resolução de referências pelo namespace de origem, aliases e transformação de bytes opcionais escolhidos pelo mod |

Todas estendem por prefixo binário compatível. Nada no nome cita Goron, MM ou um mod. O contrato completo de cada
uma fica para a fase F2 e entra neste documento como seção de implementação.

### 5. Mods novos

| Mod | Tipo | Conteúdo | Repositório |
|---|---|---|---|
| **MM Base** | core extension `pre_game` | Acha o `mm.o2r` (`SHIPLUA_MM_ROOT` e layouts conhecidos), monta em `mm/` com aliases `objects/` e `textures/`, traduz display lists do MM só nesse arquivo, motor de áudio do MM, serviço `linkspan.mm.audio` | Host, `soh/native-sdk/mm-base`; entra em todo RC |
| **MM Forms** | core extension `pre_game` com `entrypoint` Lua | Forma Goron completa, coreografia da máscara, mapa de voz; absorve o exemplo `goron-form` | `link-span-examples` |
| **Bonecos** | core extension `pre_game` com `entrypoint` Lua | Link criança e estátua da Elegia como tipos registrados por `linkspan.oot.actors`; absorve `kafei-puppet` e `elegy-statue` | `link-span-examples` |
| Unbound (existente) | provider | Recebe a política hoje no bootstrap: ID, `unbound.json`, ordem das camadas | `soh/native-sdk/unbound-core` |

Os três são core extensions porque a ligação por nome da ABI 1.4 vale só para elas; MM Forms e Bonecos trazem
`entrypoint` Lua no mesmo pacote, como o DMR e o Mic Ocarina já fazem. `ship.native.call` só chama a DLL do próprio
mod, então cada exemplo passa a viver dentro do mod que o implementa. Isso não exige mudança no core.

Serviço `linkspan.mm.audio` (mínimo): `play_sample(path)`, `play_font_sfx(font, index)`, `play_sfx_id(mm_id)`,
`set_voice_map`/`clear_voice_map`, `query_sfx_table`, `get_status`. O sequenciador do MM fica opcional e
experimental (hoje não há chamador de `MmSeq_Init`).

### 6. API Lua retirada

Exceção registrada à regra de depreciação do `AGENTS.md` §8: as funções são todas `experimental`, a API ainda é
0.x e só os exemplos do projeto as usam. Saem no RC11, sem ciclo de depreciação.

| Sai | Substituto |
|---|---|
| `ship.oot.audio.play_sfx`, `dump_sfx_table`, `set_voice_map` | Funções do MM Base/MM Forms pelo `ship.native.call` do próprio mod |
| `ship.oot.player.play_mask_on_animation`, `set_roll_mode` | MM Forms |
| `ship.oot.player.set_bunny_hood` | O exemplo `bunny-hood` usa `set_mask` + `set_speed_multiplier` (perde o pulo extra do MM) |
| `ship.oot.spawn_dog` | `ship.actor.spawn("oot.en_dog", ...)`, como o exemplo `actor-spawn` |
| Nomes lógicos `oot.link_child_puppet`, `oot.mm_elegy_statue` | Mod Bonecos |

- Capabilities retiradas: `oot.audio`, `oot.spawn_dog`, `oot.player.bunny_hood`.
- Ficam, com descrição corrigida: `oot.player.mask` (só `set_mask`), `oot.player.roll` (só `set_roll_blocked`),
  `oot.player.custom_body` (não garante habilidades).
- `set_body`/`get_body` perdem os campos com semântica Goron (`models.roll*`, `roll_offset`, `shield`,
  `water_void`, `block_ledge_grab`, estados de bola, socos e tambores). O tipo `any` do schema não acusa isso; a
  documentação passa a listar o subconjunto genérico.
- `ship.mm.*` (host do MM) não muda.

### 7. Chave "sem mods = vanilla"

A auditoria achou diferenças do upstream mesmo sem mods. Ficam corrigidas na fase F4:

- **V2** banco de objetos: limite efetivo de 128 sem mods (hoje 1024 sempre);
- **V3** texto: tabelas e IDs novos só com a chave ligada;
- **V4** `LightContext_DestroyList`: comportamento upstream sem mods;
- **V5** `Camera_GetWaterBoxDataIdx`: usar `LINKSPAN_BGCHECK_Y_MIN` nas duas saídas (hoje entrega -2147483648 onde o
  upstream entregava -32000);
- **V7** salas, transições e meshes: limites efetivos antigos sem mods.

Aceitas e documentadas como exceções estruturais: **V1** heap maior e tipos ampliados, **V6** sentinelas de
colisão novas, **V8** correções de robustez para dados inválidos ou saves corrompidos.

A seção de save `unbound` mantém o nome: mudar o formato persistido pediria migração.

## Fases

| Fase | Entrega | Repositório |
|---|---|---|
| F0 | Esta RFC e os inventários | Core |
| F1 | ABI 1.4 (parser, `NativeProvider`, schema do manifesto, `shipmod`, testes); retirada das 7 funções e 3 capabilities do schema da API; regeneração de tipos, docs e mocks | Core |
| F2 | Catálogo público, os 8 hooks, os 6 serviços, despacho de áudio. **O layout id muda uma vez, aqui** | Host |
| F3 | MM Base, MM Forms, Bonecos; exemplos migrados (`dog-spawner`, `bunny-hood`) | Host e exemplos |
| F4 | Sai do exe: `mmaudio`, `mmform`, `ShipLuaPuppet`, parte Goron do corpo, política MM e Unbound; bindings Lua retirados; lacunas V2–V5 e V7 | Host e `unbound-core` |
| F5 | NEI: remesclar `Player_PlayVoiceSfx` e `Player_PostLimbDrawGameplay` sem as pontes retiradas, regenerar overlay e imports, reempacotar para o exe novo; DMR para 1.4 | Host |
| F6 | RC11: bateria, gates, matriz com as linhas novas, docs, wiki e notas | Todos |

F4 não toca headers de `soh/include` nem de `soh/soh/native`. Se precisar, a mudança volta para a F2, para o layout
mudar uma vez só.

## Testes e critério de pronto

- **Exe:** sem `mmaudio`, `mmform`, `ShipLuaPuppet`, `MmCrossWorldArchive`, `unbound.json` e
  `linkspan.unbound.framework` no fonte compilado e nos símbolos.
- **Sem mods:** gate `semmods` verde e V2–V5, V7 conferidas.
- **ABI 1.4:** matriz da seção 5.3 de `analise-ligacao-por-nome.md` (negociação, layout igual e diferente, fronteira público/privado,
  gates independentes, sem `soh.symbols`, lifecycle, gerador do catálogo, host real, ferramentas).
- **Montagem genérica:** fixture sintética com prefixo arbitrário, colisão, referência por hash, `.meta`,
  listagem, cache, unload/reload e isolamento de `mod/<id>/`; montar `mm/` não altera `portVersion` nem a base
  do Unbound.
- **MM Base:** com e sem `mm.o2r`; ordem antes dos consumidores; texturas do NEI que apontam para o MM.
- **Áudio:** efeito por fonte, sample, mapa de voz com fallback, contribuição de PCM real e unload seguro; com NEI e
  DMR juntos.
- **MM Forms:** capturas antes/depois de entrada e saída, câmera, flash, sons, bola com espinhos e magia, quique,
  ground pound, combo, escudo, tambores, água/void, bordas, dano, troca de cena, save/load e restauração do Link.
- **Bonecos:** spawn/despawn, recurso ausente, troca de cena; estátua com e sem MM Base.
- **Bateria do RC:** fixtures E e M2 continuam; a matriz ganha linhas com os mods novos; gates de capability e
  recovery ganham negativos com MM Base ausente.

## Compatibilidade

- O layout muda na F2: todo mod nativo é reempacotado no RC11, como em qualquer layout novo.
- Pacotes do RC10 que usam as funções retiradas param de funcionar no RC11.
- Mods 1.2 e 1.3 publicados mantêm o comportamento antigo; um mod 1.4 num host 1.3 é recusado na negociação.

## Riscos

- **Forma Goron:** depende da ordem interna do update do Link (ação, PAUSE, quads, animação, root motion). Os
  hooks e o `movement` V3 precisam reproduzi-la; capturas A/B a cada passo.
- **Thread de áudio:** unload com callback em voo; o despacho próprio precisa garantir quiescência.
- **Overlay do NEI:** a remesclagem pode tirar funções do overlay sem erro; conferir `overlay-report.txt`.
- **Ligação por nome:** mesma assinatura com semântica diferente e tipos de fora de `soh/include` passam pelo
  layout. O layout dá compatibilidade, não isolamento.
- **Tamanho:** é a maior mudança do projeto até aqui; seis fases com PRs próprios.

## Notas de implementação

### F1 (core)

- `api_version` fica em `0.5.0`: a 0.5.0 só saiu em pre-releases, e a retirada da §6 acontece antes da versão final.
- O core zera `*address` em toda falha de `resolve_symbol`, em qualquer minor.
- `host_layout_id` é normalizado para minúsculas; a carga C++ direta compara sem diferenciar maiúsculas.
- Layout diferente não recusa a carga; o caminho público fica `UNSUPPORTED` e o `init` decide.
- Falha do catálogo (exceção ou status diferente de OK/UNSUPPORTED) não cai no caminho privado.
- O schema do manifesto passa a recusar `host_fingerprints` e `host_layout_id` fora de core extension, como o parser.
- Plano: [docs/plans/host-slim-001/plano-f1-core.md](../docs/plans/host-slim-001/plano-f1-core.md).

### F2i (core e host): evento Lua de dano de ator

- Motivação: portar o mod leveled (RPG) como mod, sem política de RPG no host. O host só abre o ponto central de
  dano de colisão. Plano: [plano-f2-host.md](../docs/plans/host-slim-001/plano-f2-host.md), decisão 9 e §5.1.1.
- F2i-0 registra `hook.oot.actor.damage` no core; a F2i emite o evento no host OoT, depois do ponto nativo
  `oot.actor.damage`.
- Contrato: `kind: transform`, `phase: hook_bridge`, `support: ["oot"]`, `capability: hooks.bridge`,
  `cancellable: false`.

| Campo | Tipo | Semântica |
|---|---|---|
| `target_id` | integer | `Actor.id` do dono do AC |
| `target_category` | integer | categoria do alvo |
| `attacker_id` | integer | `Actor.id` do dono do collider AT que acertou o elemento; -1 sem atacante |
| `attacker_category` | integer | categoria do atacante; -1 sem atacante |
| `attacker_is_player` | boolean | o AT tem `AT_TYPE_PLAYER`, com ou sem atacante |
| `dmg_flags` | integer | `toucher.dmgFlags` do elemento AT (u32 sem perda) |
| `damage_effect` | integer | efeito de dano do alvo |
| `base_damage` | number | dano calculado pelo jogo, antes dos hooks nativos |
| `damage` | number | dano depois dos hooks nativos, antes do Lua |

- OoT: um evento por elemento acertado em `CollisionCheck_ApplyDamage`, no ramo sem `AC_HARD`, antes da soma em
  `colChkInfo.damage` e do multiplicador do modo cooperativo. Fica fora o dano que não passa por ali: débitos de chefes
  que não usam o dano central, agarrões com `play->damagePlayer`, empurrões com dano, queda, fogo e armadilhas.
- `ship.hooks.result(n)` com inteiro ou decimal troca o dano. O host corta em 0..255 e, quando algum hook trocou o
  valor, no que falta para 255 no acumulador `u8` do alvo, antes do multiplicador cooperativo. Sem resultado, ou com
  booleano ou texto, mantém `damage`. Não cancela. Com vários ouvintes, vale o último resultado; `event.damage` não
  muda entre eles.
- Erros: `ship.hooks.result` fora de transform, com `nil`, tabela, NaN ou infinito dá erro Lua (binding atual). Falhas
  seguem a política de isolamento e de desativação depois de falhas seguidas.
- MM: sem emissão. `support` é metadado; o core ainda aceita a inscrição em contexto MM, e esta fase não muda isso.
- Compatibilidade: adição durante a pré-release 0.5.0; `api_version` fica em 0.5.0. O host precisa do core com o
  schema e os bindings regenerados.
- Segurança: só valores escalares chegam ao Lua, nenhum ponteiro. A ponte roda na thread do jogo e não deixa exceção
  C++ atravessar a fronteira C.
- Testes: schema e metadados gerados (RED/GREEN); `ModHost` e Lua reais com os nove campos, inteiro e decimal,
  ausência de resultado, tipos não numéricos, erros, escopo e `off`. No host: despacho nativo, contrato Lua, cortes,
  caminho sem mods bit a bit e prova em jogo.
- Depreciação: nada sai nem é depreciado.

### F3/F4 em cascas (host, 27/10/2026)

Decisão do mantenedor: o exe não leva nada que não seja do SoH, e os assets do MM ficam de lado por enquanto. O
conteúdo do MM sai do exe agora, mas como **casca**: mods nativos em `soh/native-sdk/` que compilam contra o SDK do
host, ficam fora do RC e não têm prova em jogo até o MM voltar.

| Casca | Id | Conteúdo |
|---|---|---|
| `mm-base` | `linkspan.mm.base` | Motor de áudio do MM, montagem do `mm.o2r` em `mm/` com tradução de DL só nesse arquivo, serviço `linkspan.mm.audio`, hooks `oot.audio.mix` e `oot.player.voice_sfx` |
| `mm-forms` | `linkspan.mm.forms` | Forma Goron, coreografia da máscara e mapa de voz; é o mod das máscaras e formas, separado do NEI |
| `puppets` | `linkspan.oot.puppets` | Link criança e estátua da Elegia como tipos registrados por `linkspan.oot.actors` |

Diferenças em relação à §5 e às fases:

- MM Forms e Bonecos ficam no host, ao lado do MM Base, e não no `link-span-examples`, para o CTest
  `oot_native_mod_shells` compilá-los a cada CI. Os exemplos `goron-form`, `kafei-puppet` e `elegy-statue` saíram do
  repo de exemplos; `bunny-hood` e `dog-spawner` usam só a API genérica.
- O NEI perdeu as três pontes do exe (`ShipLua_TransformVoiceSfx` e `ShipLua_DrawMaskTransitionHand/Head`); parte da
  F5 entrou junto. O mapa de voz e a máscara chegam pelos hooks da F2.
- Os `.o2r` soltos em `mods/` continuam sob `mod/<id>/`, com alias por arquivo, mas sem a tradução de DL do MM, que
  corrompia os opcodes 0x44/0x45 do dialeto do SoH.
- O corpo customizado genérico mantém a neutralização da raiz e as animações convencionais de escada
  (`climb_start_a/b`, `climb_up_l/r`, `climb_down_l/r`, `climb_wait`); saem só os campos Goron.
- Lacunas registradas nas cascas: Bonecos registra os tipos, mas o host não resolve os nomes lógicos em
  `ship.actor.spawn`; MM Forms usa `ResourceMgr_LoadAnimByName` e MM Base `ResourceGetDataByName` pelo caminho privado
  (presos ao SHA do exe). Antes de entrar num RC, cada casca precisa da prova da seção "Testes e critério de pronto".
- Ficam para depois: a política do Unbound no bootstrap (vai para o `unbound-core`) e as lacunas V2–V5 e V7.

### F4: política do Unbound no `unbound-core` (host, 30/10/2026)

Saiu do bootstrap o que restava do Unbound no exe: a leitura do `unbound.json`, o id `linkspan.unbound.framework`, a
fila das camadas e a montagem delas na raiz. Como previa o plano da F2 (decisão 4), nenhum header muda e a ordem fica:

- o host trata a camada como qualquer `.o2r` de mod: montagem privada sob `mod/<nome>/`, com alias por arquivo de
  `objects/` e `textures/`;
- depois da carga dos mods, o host dispara o `oot.mods.archives` com os archives aceitos (soltos em `mods/` e os de
  mods carregados);
- o `unbound-core` lê `mod/<nome>/unbound.json` pela montagem privada, aplica a regra antiga do host
  (`IsLayerManifest`: objeto JSON estrito, `"format": "unbound"`, `"formatVersion": 2`, até 64 KiB) e monta a camada
  na raiz com `resources.mount_archive`, ainda depois dos providers e consumidores e antes do `game.ready`. Desmonta
  no shutdown, em ordem inversa.

Diferenças em relação ao comportamento anterior:

- sem o framework, a camada fica na montagem privada e não sombreia nada; o host não loga mais o aviso de camada
  ignorada (o framework registra as montagens em `logs/linkspan-unbound.log` e na linha `camadas:` do
  `unbound_report`);
- a camada ganha a montagem privada com alias, que antes não tinha. O alias só entra em caminho livre, então não
  muda o que o jogo já tem nem a conversão da base;
- com dois `.o2r` de mesmo nome em pastas de mod diferentes, o framework escolhe o `unbound.json` do próprio archive
  pela leitura em camadas, não pelo prefixo compartilhado;
- o framework novo num host anterior a esta fatia (mesmo layout) não monta de novo o archive que já fornece o
  `unbound.json` da raiz;
- o archive da pasta de um mod que não carregou sai do VFS logo depois da carga e fica fora do hook (antes só a
  camada Unbound era adiada e descartada; agora vale para qualquer archive de pasta);
- durante o despacho do `oot.mods.archives`, o `mount_archive` de um archive da lista o promove: a raiz monta e
  depois a montagem privada sai, para o mesmo arquivo não aparecer duas vezes nas leituras por camada. Se a raiz
  falhar, a privada fica. Fora do despacho o `mount_archive` continua só acrescentando, então um provider que monta o
  próprio archive na raiz mantém o `mod/<nome>/`.

Alias residual, corrigido depois (Shipwright#32 e BaiterYamato/libultraship#2): quando um mod é rejeitado, o alias
que o archive dele ocupava no boot passa ao primeiro archive sobrevivente com o mesmo caminho, na ordem original. O
plano (`ModAssetAliasPlan`) reabre só os wrappers que mudam, e o host publica o lote pelo `PublishArchiveBatch` do
libultraship antes de `oot.mods.archives` e `game.ready`. A publicação invalida no cache os nomes afetados
(inclusive `.meta` e `alt/`) e avança um epoch, para que uma carga em andamento não regrave o recurso antigo. Se a
redistribuição falhar, os rejeitados saem pelo mesmo lote sem redistribuir; o boot segue sempre. Sem rejeitados,
nada muda. Ficam dois limites: durante o `Init`, o mod depois rejeitado ainda expõe o alias, e um recurso derivado
já retido continua apontando para ele.

A política ficou em `layer_archives.h/.cpp` do `unbound-core`, com teste sem jogo (`linkspan_unbound_archives_tests`,
33 casos: extensão e prefixo, manifestos recusados, mesmo stem, archive de mesmo stem acima dos limites, archive já na
raiz, falha de montagem, payload truncado, stride maior, ordem e desmontagem inversa).

### F4: chave sem mods = upstream (V2–V5, V7) (host, 30/10/2026)

A série V2 → V3 → V4 → V5 → V7 fecha as lacunas da §7 nos consumidores internos, usando a chave
`LinkSpan_EngineExtended()` já fixada no boot. Não muda a detecção de mods nem toca `soh/include/` ou
`soh/soh/native/`; o único header alterado é o interno `soh/src/code/linkspan_vanilla.h`, fora dos insumos do
fingerprint. Upstream de comparação: `ecd889c2019b87e78b0a3d942a6a3fe98b7efaed`.

- **V2, objetos:** sem mods, limite efetivo de 128 vagas, contando keeps e demais reservas; com mods, ficam as
  1024 vagas e o fluxo anterior. O ajuste da revisão conserva a particularidade do laço C++ upstream: a vaga 127
  entra na contagem, mas não recebe `OTRfunc_800982FC`. Os incrementos continuam, e o laço para em 128. O caminho C
  não ganha essa exclusão. Com mods, a última vaga continua sendo preenchida.
- **V3, texto:** sem mods, tabelas dimensionadas pelo recurso base, bytes/tamanhos originais, sem acrescentar
  `0x02`; overrides só substituem IDs existentes, e as operações de mutação devolvem zero sem operar. Capacidades
  efetivas: 1280 bytes raw e 200 decoded (640/100 unidades largas), incluindo JPN, créditos e AskToEquip. Com mods,
  ficam as tabelas extensíveis/hash, IDs novos, normalização, mutações e capacidades 8192/1024.
- **V4, luzes:** sem mods, volta literalmente o laço upstream de `LightContext_DestroyList`, sem a recusa por luz
  de ator ocupada. Com mods, ficam a recusa atômica e a iteração segura.
- **V5, câmera:** sem mods, as duas saídas sem waterbox/natação escrevem `-32000.0f` e retornam `-1`; com mods,
  fica a sentinela ampla. Sucesso e retorno `-2` não mudam.
- **V7, salas e meshes:** sem mods, voltam as conversões `u8/s8/s16`, posições XML por `IntAttribute`, índice de
  transição pelos seis bits de `params` (`transitionIndex = -1` e soma upstream) e orçamento de sort de 64 meshes
  tipo 2. Entradas XML usam sala `u8`, não `s8`; o binário já entregava esse resultado. Com mods, ficam contagens
  amplas, posições fracionárias, índice independente, packing mascarado e sort até 1024.

Diferenças aceitas que permanecem:

- **V1:** heap, arrays e representações físicas continuam ampliados; os gates restauram os limites efetivos,
  não os tamanhos/offsets antigos.
- **V6:** continuam as sentinelas e os contratos de colisão/bgId aceitos na §7; a V5 corrige apenas aquelas duas
  atribuições da câmera.
- **V8:** continuam recusas/clamps para dados inválidos, guardas de decode, busca limitada ao tamanho real da
  tabela, tratamento de recursos ausentes, leitura binária de contagem de mesh por `ReadUByte` e correções de
  ownership de paths/união de mesh tipo 1. Não se reproduzem os acessos inválidos correspondentes do upstream.
- A V3 usa vetores no lugar do `malloc` antigo e mantém o init único herdado da base. Não há hot-load vanilla
  demonstrado que dependa da tentativa posterior do upstream; não se promete equivalência nesse cenário.

**V4 restaura de propósito um defeito upstream:** `RemoveLight` já avança a cabeça, e o segundo avanço pode
desreferenciar NULL ou abandonar nós/ocupação. Nenhuma chamada vanilla alcança essa função nas revisões examinadas:
a revisão encontrou declaração/definição e registros de endereço, sem chamada direta ou callback vanilla. Uma
chamada artificial/externa pode reproduzir o defeito; atravessar portas ou observar luzes não prova esse caminho.

**V3 tem risco condicionado, igual ao upstream:** as tabelas emprestam bytes dos recursos mantidos no cache do
ResourceManager, sem reter seus `shared_ptr`. Se um unload retirar a última referência, os ponteiros ficam pendentes
(por exemplo, texto alternativo sob `alt/*`). Não foi encontrado unload dos textos base no fluxo vanilla normal.
Esta série não garante segurança após unload de textos.

Verificações:

- `SERIE.md` e `REVISAO-V.md`: checks de aplicação e composição, mais probes MSVC de recortes. Não substituem o host
  integrado.
- Host (Shipwright#29, exe `2F8844BA`): build MSVC e CTest 85/85.
- Em jogo, sem mods e com mods: cenas, texto da placa e troca de sala por EnHoll saem iguais.
- Alemão, francês, JPN e créditos ficam por ver, porque o `oot.o2r` disponível é NTSC-US.

O gate `semmods` continua com o mantenedor.

### F5: NEI regenerado e DMR na ABI 1.4 (host, 30/10/2026)

A F4 em cascas já tinha remesclado `Player_PlayVoiceSfx`, `Player_PostLimbDrawGameplay` e `Player_DrawGameplay` do
NEI: voz, callbacks de limb e desenho de máscara passam pelos hooks genéricos, sem as três pontes retiradas do exe
(`ShipLua_TransformVoiceSfx` e `ShipLua_DrawMaskTransitionHand/Head`). A F5 regenera o NEI para o host limpo e
migra o DMR; não recoloca conteúdo do MM no exe.

- **DMR:** o manifesto passa à ABI 1.4 e grava `[provider].host_layout_id` a partir do SDK, sem
  `host_fingerprints`, tanto pelo CMake local quanto pelo exemplo exportado no bundle. O provider pega `gItemIcons`
  e `OLib_Vec3fDiffToVecSphGeo` pelo catálogo público (minor ≥ 4), sem `soh.symbols` e sem autorização de patches.
  A fixture de ícones do `EngineTests` passa de 1 para 256 posições, o tamanho que o DMR indexa.
- **NEI:** a V3 mudou `Message_DecodeChecked` e envelheceu a resolução em `overlay-extra`. A mescla de três vias saiu
  limpa e o NEI volta a configurar com `NEI_ALLOW_OVERLAY_CONFLICTS=OFF`. O `fork/readapt.py` reproduz essa
  readaptação quando só o host muda (19 testes, `linkspan_nei_readapt_tests`).

O contrato que fica: o DMR depende só do layout; o NEI segue pelo escape hatch, preso ao SHA-256 do exe, e cada host
novo pede reextração, build e pacote do NEI, mesmo com o layout igual. Nenhum header dos insumos do layout mudou
(`875d1455…`).

Verificações (Shipwright#33, exe `1d020088`):

- Host: build MSVC, `soh_symbols` e CTest 87/87. NEI: 319 funções do host sobrepostas, nenhuma ponte no fork gerado,
  `Actor_UpdateAll` copiado com `LinkSpan_ActorSettleDamage`, CTest 7/7. DMR: CTest 3/3.
- Em jogo, pelo MCP de playtest: sem mods; só o DMR sem `soh.symbols` no runtime; só o NEI (`fork: active`,
  `host: 316/316 desvios`, kaleido com 3); NEI e DMR juntos, com pausa e retomada.
- Ficam por ver: ícones do seletor e câmera do DMR com controle físico (o seletor lê o gamepad SDL) e voz/coreografia
  do MM, que dependem das cascas e do `mm.o2r` adiados.

### F6: NEI do RC11 sem assets do MM (host, 31/10/2026)

Decisão do mantenedor: no RC11, item ou recurso do NEI que pede asset do MM fica indisponível (não aparece, não é
obtido, não é usado), e o save guarda os dados MM intactos para quando o MM voltar.

- **Perfil `oot` do pacote:** core + SSBB, conferido contra `oot.o2r` e `soh.o2r`; o `mm.o2r` nunca entra. A
  auditoria de código lê o fecho da DLL (`tools/nei-code-closure.py`). Referência MM só pode ficar num ramo que o RC11
  desliga, e cada uma é declarada com o motivo em `fork/code-allow-oot.txt`.
- **Fork (`patches/0013-rc11-sem-mm.patch`):** Picto, Keg, espadas e upgrades MM, Pendant, Ikana, Statue da Cane,
  Shadow da Wand, conteúdos MM de garrafa, troca MM e a página de máscaras MM saem da posse e da exibição sem apagar
  bit nem id. Leitura filtrada não vira espaço livre para escrita (projeção de sessão nos slots), e os escritores de
  Wand/Cane só mexem nos bits nativos. Deku Leaf, Clawshot, Climb/Roc Boots e garrafas OoT seguem iguais.
- **Armadilha:** a DLL compila `extracted/nei_save.c`, que o sync tira do blob do fork sem os patches. Mudança no
  `nei_save.cpp` vai pelo `extracted-fixes.txt`.
- **Host:** `OceffWipe_Draw` fica fora do overlay (a versão do fork pedia o frustum do MM).

Verificações (Shipwright#36, exe `9f8fd335`): NEI com CTest 7/7 e a fixture RC11 sobre os corpos extraídos; gerador
`oot` com 52 referências inativas e `--verify-assets` ok. Em jogo: 27 itens, 315/315 desvios, 2 páginas de inventário,
nenhum `Failed to load file`, e um save com dados MM volta intacto depois de salvar no jogo. Limitação anterior ao
RC11: o `ClearEquips` do `linkspan.oot.items` derruba de botão C um id `0xA0–0xEF` sem registro, como o `0xDE`
(cogumelo MM); o item fica na garrafa.

## Fora de escopo

- A rota fixa OoT → MM do `ship.world.travel` no bootstrap. É infraestrutura de viagem entre jogos, não conteúdo do
  MM dentro do OoT; fica para outra tarefa.
- Formas e máscaras do MM que o fork do NEI traz desligadas.
- Renomear `unboundSongSeqId` e comentários em `soh/include` (mudaria o layout sem ganho).
