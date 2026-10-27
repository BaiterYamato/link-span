# RFC 0029 — Host limpo: conteúdo do MM em mods

Status: aceita em 19/10/2026 (HOST-SLIM-001). F1 implementada no core; F2 no host. Em 27/10, o conteúdo do MM saiu
do exe como cascas de mod fora do RC (F3/F4 parciais, ver as notas de implementação); F5 (NEI) e F6 pendentes.
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

## Fora de escopo

- A rota fixa OoT → MM do `ship.world.travel` no bootstrap. É infraestrutura de viagem entre jogos, não conteúdo do
  MM dentro do OoT; fica para outra tarefa.
- Formas e máscaras do MM que o fork do NEI traz desligadas.
- Renomear `unboundSongSeqId` e comentários em `soh/include` (mudaria o layout sem ganho).
