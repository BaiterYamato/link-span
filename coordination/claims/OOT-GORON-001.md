# OOT-GORON-001

- Status: review
- Agent: codex-windows
- Platform: Windows 11 / Codex
- Branch: local worktree (branch to be confirmed before integration)
- Started: 2026-07-22T00:00:00-03:00
- Depends on: existing custom-body bridge
- Files:
  - ../shipwright-limpo/Shipwright/soh/soh/ShipLuaBootstrap.cpp
  - ../shipwright-limpo/Shipwright/soh/src/overlays/actors/ovl_player_actor/z_player.c
  - examples/goron-form/main.lua
  - examples/goron-form/manifest.toml
  - schema/events.yml
  - generated/docs/api-reference.md
  - generated/docs/compatibility-matrix.md
  - generated/include/shiplua/generated/ApiBindings.h
  - generated/lua/shiplua.lua
  - generated/lua/shiplua_mock.lua
  - generated/lua/shiplua_validate.lua
  - generated/tests/api_contract.lua
  - generated/tests/mock_contract.lua
  - docs/wiki/Research-External-Item-Systems.md
- Goal:
  - Aproximar as interações de água do Goron ao comportamento de MM: enrolar,
    afundar e executar o void-out nativo, sem alterar corpos que não optarem
    explicitamente por essa regra.

## Implementação

- `ground_offset` do exemplo Goron: `-90.0` → `-150.0`.
- A alteração desloca somente o corpo ereto 60 unidades para baixo; `roll_offset`
  continua em `12.0` para manter o centro da bola acima do chão.
- Escopo ampliado: ligar o skeleton opcional de defesa (`shield`) da spec de
  corpo genérica aos assets Goron equivalentes do MM e expor o estado no getter.
- O exemplo usa `hook.oot.player.health_change` para mostrar a textura
  `gLinkGoronEyesSurprisedTex` por 20 frames após dano.

## Validação

- `cmake --build build/x64 --config Release --target soh -- /m:1` — Release compilado.
- O `soh.exe` compilado foi copiado para `x64/packages/oot/Release/hosts/oot/soh.exe`; os dois arquivos têm SHA-256 `1DE32D1D82A5A107D9A6F94AFD1A3D855B217B9DDE756A82B5E0AE498812F142`.
- `py -3 tools/shipmod.py validate examples/goron-form` — válido, versão 0.1.12.
- `py -3 tests/conformance/PackageHelloWorld.py examples/goron-form x64/packages/oot/Release/mods/goron-form.shipmod` — pacote gerado.
- `py -3 tools/shipmod.py validate x64/packages/oot/Release/mods/goron-form.shipmod` — válido, versão 0.1.12.

## Animacoes de ar e dano 0.1.16

- O hook de selecao de animacao agora expõe `falling` e `landing`.
- O exemplo usa os headers compartilhados de queda, pouso e dano; pouso/dano
  sao one-shots e queda e selecionada enquanto o Goron desce.
- Host SHA-256: `FA595A22B10262922B5E2B8CE7B8524F86B319C9A60FEE0E077AE6E7EB54F0BD`.
- Mod SHA-256: `AF9F5B8F9A7EF8DE215E9AC361F329933B1959AD71F65A851792DD50323FFB58`.

## Limpeza de mascara 0.1.15

- A mascara temporaria usada pela animacao de entrada e removida antes de Link
  voltar a desenhar na destransformacao; nao fica mais presa no rosto.
- `goron-form.shipmod` 0.1.15 validado (SHA-256
  `1E560100E2DBC4B3C303FF21CB8CC15DF25D9D5E1EF48FE0EE3D9EF9C7AC719F`).

## Defesa fisica 0.1.14

- R agora preserva a postura Goron e atualiza `player->shieldQuad` frontal e
  `player->shieldMf`, reutilizando bloqueio/reflexao nativos do OoT sem
  invencibilidade continua ou collider 360 graus.
- `goron-form.shipmod` 0.1.14 validado; host Release copiado para o launcher.
- Host SHA-256: `81458C188490137AFE9D83E9324CBEC0038BB6F8B4EEF1089AD4F0D80B6B7DE2`.
- Mod SHA-256: `FD48FBF8AC7ED963336DB9C9B2C8E4012DD0E0C2394A20E68E5497E7D8B66A73`.

## Root motion dos socos

- `punch_a`, `punch_b` e `punch_c` agora usam a translacao da junta raiz por
  `SkelAnime_UpdateTranslation`, como o Player usa internamente.
- O primeiro frame usa `ANIM_FLAG_NOMOVE` para nao herdar a origem de outra
  animacao; nos seguintes, o delta e aplicado ao Player e ao ator hospedeiro
  no mesmo frame. A junta volta a `baseTransl`, evitando desvio visual da
  malha.
- O Release foi recompilado e o host do pacote foi atualizado. SHA-256:
  `CB5DFC03323A33F3704C715883A951202F18FBC5F8B6785CE2D748D04CEA787E`.

## Recuperação dos socos 0.1.17

- O exemplo declara os seis headers `pg_punchA/B/Cend` e `pg_punchA/B/CendR`.
- Ao terminar A/B/C, o host toca a recuperação em movimento quando aplicável,
  com fallback para a recuperação parada. O combo e o recoil de parede ainda
  dependem de uma máquina de ação dedicada.
- Host/pacote SHA-256: `48E0496BEA945612912241360F91406940C7FC00A67666F06ABF357181711D5E`.
- Mod 0.1.17 SHA-256: `0F7955690D219B20BDA47D013E66258585B2BBFFABF47318AAEAE90130BAFE83`.

## Tambores Goron 0.1.18

- O payload `body_anim_select` ganhou `instrument`, fechado aos dois ItemActions
  de ocarina durante `PLAYER_STATE1_IN_ITEM_CS`.
- `gakkistart`/`gakkiplay` e os seis display lists de tambor do torso foram
  adicionados ao exemplo; a saída usa o novo modo `reverse_once`.
- A curva de escala dos tambores agora replica a tabela `D_801C0428` do Player
  de MM nos tres eixos, inclusive ao usar `reverse_once` para guardar o
  instrumento.
- Host/pacote SHA-256: `DD57D7B081D8543F2D2D8E7B3F50C3A4AC03B9534F2F1D9AA7DD7277A1DC0E01`.
- Mod 0.1.18 SHA-256: `769D815CFD441030D022267CBC3C74436F74002EAD83CBB6A168150DA68F1ED0`.

## Transição de máscara 0.1.21

- `play_mask_on_animation()` e o one-shot `mask_off` agora bloqueiam input e
  movimento durante suas animações. Nos seis frames finais, o host constrói
  um flash branco e só inicia o fade depois que o skeleton trocado é visível.
- A interrupção por `set_mask("none")`, troca de cena e falha do callback Lua
  limpam o estado; o último caso possui timeout de 15 frames para não deixar o
  Player travado.
- `reverse_once` agora também limpa o override como one-shot, evitando que a
  animação de guardar os tambores fique presa.
- Host/pacote SHA-256: `39BCA5E3DA6C9DB1BA34640C3B04EA88F9B69B36180A8A150C3B381746FF59A3`.
- Mod 0.1.21 SHA-256: `76790578310345B53EC932903EC2CEC27541CA64FB8D770B679F1AF25246673E`.

## Combo Goron 0.1.22

- Cada ataque começa em `punch_a`. Um novo B enquanto `punch_a`/`punch_b`
  está além do frame 0 é armazenado, como o `actionVar2` nativo de MM, e
  encadeia `punch_b`/`punch_c` no fim da animação sem acionar a recuperação.
- `get_body()` agora retorna `punch_combo_queued`; o exemplo deixou de alternar
  A/B/C artificialmente em Lua. Root motion e janelas de dano existentes são
  preservadas para cada etapa.
- Host/pacote SHA-256: `D5AA658C98E4543047C2BF1AE681288129AF02448CEF0F5D4E442040E9DD758F`.
- Mod 0.1.22 SHA-256: `841F988CC3E279ABC1A8722D5632237B369859D62901275E41AB23914C617E85`.

## Bounce da bola 0.1.23

- O hook de bonk agora detecta bola Goron rápida, reflete `yaw` por `wallYaw`,
  preserva a velocidade com perda de 15% e religa `Player_SetupRoll` no mesmo
  frame. Quatro frames de cooldown impedem rebatidas repetidas.
- Um `AT_HIT` do cylinder armado suprime a reflexão para priorizar a colisão de
  alvo/dynapoly. `get_body()` expõe `roll_bounce_active` e
  `roll_bounce_frames`.
- Host/pacote SHA-256: `624B1F75023DB2939D85D53F55F1CE0F89DDBA39CFDD779CB72C8A9061DDF422`.
- Mod 0.1.23 SHA-256: `E4AB5A76245C25ADB2D544A10EA9D4F94C0649FA2E90025909043D25D03F1496`.

## Água funda 0.1.24

- `water_void = true` é uma opção explícita da spec do corpo; quando a
  profundidade passa de 20 e não há item pendente, o bridge bloqueia input,
  toca `roll_enter`, muda para a DL enrolada sem giro, afunda por 15 frames e
  chama `Play_TriggerVoidOut`.
- A regra não altera corpos que não a declaram. `get_body()` mostra
  `water_void`, `water_void_active`, `water_void_phase` e `water_void_frames`.
- Host/pacote SHA-256: `62094DD512088D2457D964947218A886503B47923D4C721C715526FF445004B8`.
- Mod 0.1.24 SHA-256: `23DEFD7E752EDFFF20F2E2A30300C2FE3D2DF4E42BD95301875509ED186837C1`.

## Notas dos tambores 0.1.25

- A tabela de animações declara `pg_gakkiwait` e as cinco variantes
  `pg_gakkiplayA/L/D/U/R`. O payload do seletor informa a nota pressionada no
  frame pelo mesmo mapeamento de `ocarinaButtonIndex` do Player de MM.
- `CustomBodyIsGakkiAnimation` inclui repouso e todas as notas, preservando os
  seis display lists no torso durante toda a execução.
- Host/pacote SHA-256: `36F098C6D4E87244E8AC416BF29F01CF3BEA3463CCC481E26316D572784C161F`.
- Mod 0.1.25 SHA-256: `5B47DE0E5B345C9C2692015409F055CACEF53DC4ED70D3FFACEDA266EA9568DE`.

## Notas dos tambores 0.1.26

- A nota recebida não é mais uma seleção de um único frame: o Lua inicia
  `gakkiplayA/L/D/U/R` como one-shot e só depois retorna a `gakkiwait`. Isso
  preserva a pose completa, como a troca de animação do Player de MM.
- Host/pacote: `36F098C6D4E87244E8AC416BF29F01CF3BEA3463CCC481E26316D572784C161F`.
- Mod 0.1.26: `2ECDFDCE138FCD79B853267831C0B82B2A33A51EECF15D2495CE4585AD3E7AD1`.

## Efeito visual dos socos 0.1.27

- `gLinkGoronGoronPunchEffectDL` foi declarado como `punch_effect`. Durante as
  janelas de dano já existentes, ele é desenhado em Xlu vermelho na mão esquerda
  de A, direita de B e cintura de C, sem mudar hitbox ou dano.
- Host/pacote SHA-256: `CB63CF0C6C735A904D05A197F91A95B9506DF1A713D73708D54F162523AB9C68`.
- Mod 0.1.27 SHA-256: `73D8A886A3AAE6E944CFD1D4F373310766EC72F4D14B68C6F6F18168396BBE34`.

## Bloqueio de borda 0.1.28

- `block_ledge_grab = true` chama `ShipLua_ShouldBlockLedgeGrabs()` dentro do
  gate nativo que inicia o ledge grab do Player. O predicado é independente do
  estado global `DisableLedgeGrabsActive` de Crowd Control.
- Host/pacote SHA-256: `4F722AE6127ED0285F261B637D73A4C9A35AEC5B9FB67EEBC91ECB7DE718C63D`.
- Mod 0.1.28 SHA-256: `B4E9C69BC80102D19F114AE7245E70C8703D767913A92A4DEBA2C635E03F0C58`.

## Correcao de desenho da bola na agua 0.1.29

- O estado `water_void` ja calculava a escala, a altura e a fase `Ball`, mas o
  draw selecionava a display list enrolada somente durante `Player_Action_Roll`.
  Agora `Ball` seleciona explicitamente a mesma DL do Goron enrolado, sem giro,
  espinhos, energia ou carga, antes do `Play_TriggerVoidOut`.
- Host/pacote SHA-256: `1E07E5205523F90A13BE2FFECC4EC0DE64112F3C644B2D2A7B7F38CEB5257F0F`.
- Mod 0.1.29 SHA-256: `698CD7AF638B1CC2A5E3B69B649FB728F4807CA155A4375C9A73BCDC72B7FB00`.

## Escalada Goron 0.1.30

- O hook de selecao de animacao agora informa `climbing`, `climb_direction`,
  `climb_step` e `climb_starting`, baseados em `PLAYER_STATE1_CLIMBING_LADDER`,
  input vertical e na alternancia nativa de passos do Player.
- A spec generica aceita `reverse_anims = { nome = true }`; somente chaves ja
  declaradas em `anims` sao aceitas. O getter devolve a configuracao efetiva.
- O exemplo declara os starts, passos e finais `pg_climb_*` do MM. A descida
  usa `pg_climb_upL/R` em loop reverso, como a tabela nativa do Player de MM.
- Host/pacote SHA-256: `7FD2B267A2BF2C99F53F73C45BA4AD12A96C2EA92DBBAC4623B73646A9453DB5`.
- Mod 0.1.30 SHA-256: `0A07863CD06CA5F4B3DFAFE3152A9281BF36F4FD819C15E0DA04E67BD600C91D`.

## Porta e bau Goron 0.1.31

- `door_opening` so fica verdadeiro em `Player_Action_80845EF8` para porta com
  macaneta; `door_direction` escolhe `pg_doorA_open` ou `pg_doorB_open`.
  Portas de correr, falsas e entre cenas permanecem no fluxo vanilla.
- `chest_opening` exige simultaneamente `GETTING_ITEM` e `ACTOR_EN_BOX`, para
  nao confundir um bau com outro get-item. `pg_Tbox_open` toca uma unica vez;
  o item, a camera e a transicao continuam no OoT.
- Host/pacote SHA-256: `80B256AB2F7876516352B22DC83AF0752CBCAC27A412A26F933F0900092F55D8`.
- Mod 0.1.31 SHA-256: `462DD1BC896722FF9D9D3E781763E092FB8450EA629D20B7F808DED4C582F470`.

## Getter completo da spec 0.1.32

- `get_body()` agora devolve `segments` e `models` alem de `anims`, `shield` e
  `reverse_anims`. Isso reflete os caminhos atuais, inclusive uma textura
  alterada por `set_body_segment`, sem expor os espelhos internos `__OTR__`.
- Host/pacote SHA-256: `0429EBBC16ED667D66711002851A9235773B2B45E25DD0B2F7EE5046E820CDE0`.
- Mod 0.1.32 SHA-256: `FE586C62B8196CB3F7A6C966DA02540C12252B9F33F202419DF616EDA6F43527`.

## Preflight completo de animacoes 0.1.33

- A ativacao agora exige que cada header em `anims` exista e carregue pelo
  Resource Manager. A spec e atomica: nenhum corpo externo entra ativo com
  um subconjunto de animacoes que poderia falhar durante um estado posterior.
- O inventario local confirmou 31 headers `gPlayerAnim_pg_*` no MM e os 31
  estao declarados pelo `goron-form`.
- Host/pacote SHA-256: `19D853EFC66DD46CAC768ED6AAC6EDF724F2EA28EAE00DFEE8A5449224161C94`.
- Mod 0.1.33 SHA-256: `9FA54D5AACADC4038FFEDDD9C11B378BCC86CD8EAE04F18BBD52601BEEB642E4`.

## Acoes com dono: soco, defesa e instrumento 0.1.34

Referencia usada: `skijer/Shipwright` @ `Not-Enough-Items`,
`soh/mods/transformation_masks/` (`mm_player_form.cpp`, `mm_form_combat.c`).
E o mesmo problema — forma do MM dentro do OoT — ja resolvido. OoTMM e
ComboShip foram reconferidos e seguem descartados como referencia (ver
`handoffs/OOT-GORON-002-port-mm.md`).

Os quatro sintomas relatados pelo usuario (animacoes que nao vao, soco
inexistente, ocarina sem tambores travando o corpo, R deixando o Goron em
postura de defesa deformada) tinham a mesma raiz: o corpo so tinha o caminho
"perguntar ao Lua qual animacao tocar", com override one-shot por cima, e a
action func do Player do OoT continuava rodando por baixo.

- O host ganhou uma maquina de acoes (`None/Punch/Shield/Gakki`) que toma posse
  do corpo e reafirma `PLAYER_STATE3_PAUSE_ACTION_FUNC` a cada frame. O flag ja
  existia e ja era respeitado (`z_player.c:12099`, limpo em `:12193`); nao foi
  preciso tocar no Player.
- Soco: a entrada passou a ser o B do `Input`. A versao anterior dependia de
  `player->meleeWeaponState`, que so o swing de espada escreve e a forma nunca
  produz — por isso nao existia soco. Combo A/B/C indexado por `comboStep`,
  com as recuperacoes e as janelas de dano ja existentes.
- Instrumento: fases 1 `gakkistart` / 2 `gakkiplay` em loop / 3 a mesma ao
  contrario. A nota vem de `Audio_OcaGetPlayingStaff()->noteIdx` (durante a
  ocarina o OoT ja consumiu os botoes, e por isso a leitura por `press.button`
  quase nunca via nota). Os tambores passaram a ser desenhados pela FASE e nao
  pelo nome da animacao. Entraram as tabelas `D_801C0490` (pulsacao) e
  `D_801C0510` (repouso); antes so `D_801C0428` existia, e o conjunto ficava
  estatico depois de aberto.
- Defesa: acao propria com yaw travado, agachamento reiniciado a cada entrada e
  `shieldQuad`/`shieldMf` estampados. O R antes so trocava o skeleton desenhado.
- `body_anim_select` continua dono da locomocao. `attacking`, `punch_step` e
  `shielding` viraram leitura; `instrument_note` agora vem do staff.
- Assets seguem 100% cross-world, lidos de `mm/...` no `mm.o2r`.

- Build Release do target `soh`: passou, sem erro nem aviso no arquivo alterado.
- `shipmod validate` no diretorio e no `.shipmod`: passou.
- Host SHA-256: `82821943446E2014FE142E8FFF68D8700EEF194B47040EE057081FD2B6B94D29`.
- Mod 0.1.34 SHA-256: `1BDEC62716894BA0DF13199731CA4329404927EE6DA24CD81933E9E5E6FA903D`.

## Escada, defesa em loop e combo 0.1.35

Retorno do teste em jogo de 0.1.34: terceiro soco com animacao errada, a defesa
com R parando ao fim da animacao, e escalada de escada sem animacao.

- Defesa: regressao introduzida em 0.1.34. Eu troquei a entrada por
  `Animation_PlayOnce`; `SkelAnime_InitFlex` usa `Animation_PlayLoop`, e ONCE
  fazia a postura parar no ultimo frame. Voltou a LOOP.
- Escada: a causa era cadencia fixa. `Player_Action_8084BF1C` reescreve
  `skelAnime.playSpeed` todo frame a partir da magnitude do analogico
  (`z_player.c:13218`) e desce com SINAL NEGATIVO na mesma animacao. O corpo
  externo tocava os passos em 1.0 fixo, entao a subida nao acompanhava o Player
  e a descida — que so existe pelo sinal — nao acontecia. O host passou a
  espelhar a playSpeed do Player enquanto `PLAYER_STATE1_CLIMBING_LADDER` esta
  ligado, e `reverse_anims` saiu do mod (inverter por cima cancelaria o sinal).
- Combo: um B novo durante a RECUPERACAO agora encadeia na hora, com o passo
  ciclando A->B->C->A (`% 3`), como faz a referencia (mm_player_form.cpp:4740).
  Antes o combo morria no terceiro golpe ate a recuperacao inteira terminar.
- Recuperacao: a variante `R` passou a ser escolhida por
  `PLAYER_STATE1_HOSTILE_LOCK_ON`, que e o seletor real de MM
  (`sMeleeAttackAnimInfo` guarda o par end/endR e quem escolhe e
  `Player_CheckHostileLockOn`). A regra por velocidade era invencao nossa.
- Janela de dano: `earlyStart = 5.0f` para o Goron, como em func_8083FCF0. A
  janela por hitStart era mais estreita que a original.
- O log de estado do draw ganhou `action`, `combo`, `rec`, `gakki`, `climb` e
  `ovr`, para uma corrida em jogo fechar diagnostico sem novo build.

- Build Release do target `soh`: passou, sem erro nem aviso.
- `shipmod validate` no diretorio e no `.shipmod`: passou.
- Host SHA-256: `EE54759E5EC730268DF7B36A8484B11F0778FD181FFC34EA4F8569A505B7397C`.
- Mod 0.1.35 SHA-256: `439AAD8E9010C9F03D176CEAA3DD3B68EAC346D0C25E2CF6044E0ACB333DD864`.

## Teleporte do terceiro soco 0.1.36

Sintoma: a bundada (punch C) teleportava o Goron e o devolvia a posicao.

Causa: `SkelAnime_UpdateTranslation` faz duas coisas — devolve o delta da junta
raiz E devolve a junta a `baseTransl`. A segunda e obrigatoria em todo frame de
qualquer animacao, porque `SkelAnime_DrawFlexOpa` usa `jointTable[0]` como
translacao do modelo. A chamada estava condicionada a estar num punch, entao ao
entrar na recuperacao a junta parava de ser neutralizada, a malha saltava pela
translacao de raiz da animacao e voltava quando uma animacao de raiz zerada
assumia. Punch C e o mais visivel porque tem a maior translacao dos tres.

- A chamada passou a ser incondicional; o deslocamento continua sendo APLICADO
  ao Player somente no perfil fechado dos tres socos.
- `ANIM_FLAG_NOMOVE` passou a ser marcado em TODA troca de animacao, nos dois
  caminhos (one-shot e seletor). Antes so os punches marcavam, entao qualquer
  outra troca herdava o `prevTransl` da anterior e produzia um deslocamento de
  um frame. A referencia trata o mesmo caso com `rootMotion.firstFrame`.
- Isso tambem cobre `mask_off`, `land`, `damage`, os passos de escada e as
  animacoes de porta/bau, que tem translacao de raiz e sofriam do mesmo defeito.

- Build Release do target `soh`: passou, sem erro nem aviso.
- `shipmod validate` no `.shipmod`: passou.
- Host SHA-256: `795E65E668532DAC867AD620392F2F1076C8D087F47C65AC2688C94922F999F8`.
- Mod 0.1.36 SHA-256: `5C25191E618E7D504652C731E2D0EF3D0C0856EC9BCB8E8E691F6C857D8D9088`.

## Freeze ao sacar a ocarina 0.1.37

Sintoma: sacar a ocarina trava a tela; os tambores nao aparecem e o Goron nem
comeca a animacao do instrumento.

Duas causas, ambas em codigo que so passou a EXECUTAR na 0.1.34:

1. `CustomBodyPostLimbDraw` emitia `gSPDisplayList` mesmo com
   `ResourceMgr_OTRSigCheck` reprovado — isto e, com um ponteiro que nao e um
   recurso valido. Ate 0.1.33 o gate era por NOME de animacao e nunca ficava
   verdadeiro, entao os seis DLs de tambor nunca eram desenhados. Ao trocar o
   gate para a FASE do instrumento eles passaram a ser desenhados pela primeira
   vez, e um DL que nao resolve vai como lixo para o renderizador. O mesmo
   padrao existia em `CustomBodyDrawPunchEffect`. Os dois agora PULAM o modelo
   quando a assinatura reprova, e logam uma vez o caminho que falhou.
2. `Audio_StopSfxById(NA_SE_OC_OCARINA)` rodava a cada frame da acao. A
   referencia faz isso porque tem o sintetizador de MM para por o instrumento da
   forma no lugar; nos nao temos (OOT-AUDIO-001). Mexer no estado de audio
   durante a propria execucao da ocarina, sem substituto, so tem custo. Removido.

O log dira, na proxima corrida, se os seis caminhos `object_link_goron_DL_*`
resolvem no mm.o2r. Se nao resolverem, o proximo passo e descobrir os nomes
reais no archive — e provavel que precisem tambem do mesmo tratamento de
segmento que a referencia aplica (`MmForm_PatchSegmentedDL` em 0x08/0x0C), como
ja foi preciso para o dialeto de DLs cross-world.

- Build Release do target `soh`: passou, sem erro nem aviso.
- `shipmod validate` no `.shipmod`: passou.
- Host SHA-256: `20771BC3B1DC711B282FC7447BC1B132FD261011EF1685043FF000841B5BBA5C`.
- Mod 0.1.37 SHA-256: `8A870558F79E4177424F19FB63950BE31CD36B8311250A0CEFBDA38E72A46AD4`.

## Instrumento nunca comecava 0.1.38

O log da corrida de 26/07 12:21 fechou o diagnostico que faltava:

```
ocarina=1  action=0  gakki=0  anim=idle
```

O host DETECTA a ocarina (`PLAYER_STATE2_OCARINA_PLAYING` esta correto), mas a
acao nunca comeca: `CustomBodyBeginGakki` retornava false e saia em silencio.
Com a fase em 0, o callback de limb nem chega a tentar desenhar os tambores —
por isso o aviso de recurso de 0.1.37 tambem nao apareceu. As duas hipoteses de
0.1.37 (DL invalido, audio) estavam olhando para o lugar errado: nada daquilo
chegava a executar.

- `CustomBodyStartAnimation` tinha DUAS saidas silenciosas (nome ausente na spec
  e recurso que nao carrega). Cada uma agora se identifica uma vez por nome.
  Essa era a falha de instrumentacao que custou o ciclo inteiro.
- A abertura (`gakki_start`) virou enfeite: se nao subir, a acao entra direto na
  fase 2. Se a pose de repouso tambem nao subir, a fase fica ativa mesmo assim e
  os modelos sao desenhados sobre a animacao corrente. Os tambores nao dependem
  mais de nenhuma animacao especifica ter carregado.

A proxima corrida diz, em uma linha de log, qual das duas coisas falha — o
header `gPlayerAnim_pg_gakkistart` nao carregar, ou os seis
`object_link_goron_DL_*` nao resolverem.

- Build Release do target `soh`: passou, sem erro nem aviso.
- `shipmod validate` no `.shipmod`: passou.
- Host SHA-256: `A5A1713F7E312DA335203C13BDAD14570C010D1C9F6982BFB18DA0F7A10FC5CE`.
- Mod 0.1.38 SHA-256: `CF80F66E1E3535E82F2ADBCD380F8F32AA03A1F90CF604DFB04B008ACB4F9DC6`.

## O ator hospedeiro parava de atualizar com a ocarina 0.1.39

CAUSA RAIZ de toda a saga do instrumento, e do "o Goron trava na animacao que
estiver" do primeiro relato.

`Actor_UpdateAll` (`z_actor.c:2624`):

```c
if (player->stateFlags2 & PLAYER_STATE2_OCARINA_PLAYING) {
    unkFlag = ACTOR_FLAG_UPDATE_DURING_OCARINA;
}
```

Enquanto a ocarina esta na mao, so atores com essa flag continuam atualizando. O
ator hospedeiro do corpo externo nao a tinha. O DRAW continuava rodando — por
isso o log ainda registrava `ocarina=1` e parecia que o host detectava tudo —
mas o UPDATE nao rodava: `CustomBodyBeginGakki` nunca era chamado, a fase ficava
0, os tambores nunca eram desenhados e o corpo congelava na pose corrente.

`actor->flags |= ACTOR_FLAG_UPDATE_DURING_OCARINA` na ativacao resolve.

Licao de metodo: o log dizia `ocarina=1 action=0 gakki=0` desde 0.1.35 e eu li
isso tres vezes como "a entrada falha", investigando DL invalido, supressao de
audio e carga de animacao — todos plausiveis e todos errados. O campo que teria
apontado a resposta era um que nao existia: se a acao nao comeca E nenhum aviso
sai, a pergunta certa e se o codigo esta rodando, nao por que ele decidiu nao
agir. As correcoes de 0.1.37 e 0.1.38 continuam validas por merito proprio (DL
sem assinatura ia como lixo para o renderizador; saidas silenciosas nao se
diagnosticam), mas nenhuma delas era a causa.

- Build Release do target `soh`: passou, sem erro nem aviso.
- `shipmod validate` no `.shipmod`: passou.
- Host SHA-256: `93551B528C91A51301D5AD2906E50773CB882FF6C8DE442BC0B925A4863B97D7`.
- Mod 0.1.39 SHA-256: `39B8E2E89C30BA917C831DFCCA1D6715D5C19D59A9309892CE33D3E8E79448EC`.

## Pendente

- PRIORIDADE — confirmar em jogo as quatro correções de 0.1.34: B produz soco
  (e B de novo encadeia B/C), a ocarina abre os tambores e o corpo volta ao
  normal ao guardar, R mostra a defesa inteira, e nenhuma pose fica presa.
- Risco conhecido de 0.1.34, a observar no teste: o B chega ao Player ANTES do
  ator hospedeiro, então o primeiro frame de um B pode disparar uma ação vanilla
  antes de a pausa entrar. Se aparecer, o ponto de correção é vetar o B para o
  Player enquanto um corpo com socos declarados estiver ativo.
- Confirmação visual dos tambores: ao tocar ocarina, a entrada deve abrir os
  seis modelos no torso; A/C-esquerda/C-baixo/C-cima/C-direita precisam usar
  as poses correspondentes e, entre notas, voltar à postura `gakkiwait` sem
  remover os tambores.
- Confirmação visual dos socos: A deve mostrar efeito vermelho na mão esquerda,
  B na direita e C na cintura apenas durante as janelas de impacto.
- Confirmação de borda: o Goron deve cair/passar pela borda sem entrar na ação
  de agarrar; Link normal e outros corpos sem `block_ledge_grab` devem manter
  o comportamento vanilla.
- Confirmação visual da água: acima de 20 unidades de profundidade, o Goron
  deve tocar o curl, virar uma bola sem giro, afundar por cerca de 15 frames e
  voltar pelo respawn/void-out nativo. Em água rasa, deve manter o movimento
  normal do OoT.
- Confirmação visual no jogo de que as solas encostam no chão em terreno plano e inclinado, R mostra a postura enrolada sem regressão de render e o dano mostra os olhos surprised antes de voltar ao blink normal.
- Confirmação visual da ocarina: os tambores devem abrir no torso, permanecer durante
  a execução e recolher ao guardar o instrumento.
- Confirmação visual da entrada: G deve congelar Link, tocar a máscara humana,
  ocultar a troca no flash branco e liberar o controle só com o Goron visível.
- Confirmação do combo: B inicia A; outro B antes do fim de A produz B, e mais
  um B durante B produz C, sem tocar a recuperação entre os golpes encadeados.
- Confirmação de bounce: role rápido contra parede sólida; a bola deve refletir
  e continuar, sem refletir quando atingir primeiro um alvo quebrável.
