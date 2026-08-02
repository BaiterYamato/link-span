# Handoff — OOT-GORON-001

## Estado

review

## Resultado

- A versão 0.1.16 expõe `falling`/`landing` no hook de animação e usa os
  headers MM compartilhados para queda, pouso e dano; pouso/dano são one-shots.
- A versão 0.1.15 remove a máscara temporária antes de voltar ao desenho de
  Link, evitando que ela permaneça equipada após a destransformação.
- O pacote 0.1.14 atualiza `player->shieldQuad` e `player->shieldMf` enquanto
  R mantem a postura Goron, habilitando bloqueio e reflexao frontais pelo
  sistema de colisao nativo do OoT. Nao inclui invencibilidade nem defesa 360.
- O corpo ereto do Goron passou de `ground_offset = -90.0` para `-210.0`.
- A alteração baixa exclusivamente o skeleton ereto 60 unidades de mundo;
  a bola mantém `roll_offset = 12.0`.
- A spec de corpo aceita opcionalmente `shield = { skeleton, animation }`.
  Os dois campos são obrigatórios juntos e são validados antes da ativação.
- A postura de defesa recebe um `SkelAnime` normal independente, usa os assets
  `gLinkGoronShieldingSkel`/`gLinkGoronShieldingAnim` do MM e é desenhada com
  R pressionado. `get_body()` retorna a tabela `shield` e `shield_active`.
- O hook de vida existente mostra `gLinkGoronEyesSurprisedTex` por 20 frames
  depois de dano e então devolve o segmento 8 ao blink normal.
- O mod empacotado é `goron-form.shipmod`, versão `0.1.14`.

## Tambores Goron 0.1.18

- O hook de seleção do corpo agora informa `instrument` quando o Player está
  usando a ocarina; o mod toca entrada, loop e saída reversa dos tambores.
- O bridge anexa os seis modelos de tambor ao torso do skeleton Goron durante
  as animações gakki.
- A escala da abertura usa a tabela nativa `D_801C0428` por eixo/frame e tambem
  rege a saida reversa.
- Host/pacote: `DD57D7B081D8543F2D2D8E7B3F50C3A4AC03B9534F2F1D9AA7DD7277A1DC0E01`.
- Mod: `769D815CFD441030D022267CBC3C74436F74002EAD83CBB6A168150DA68F1ED0`.

## Transição de máscara 0.1.21

- A entrada e a saída passam a ser transacionais: congelam input/movimento,
  tocam `gPlayerAnim_cl_setmask`/`pg_maskoffstart`, sobem flash branco nos
  seis frames finais, trocam o desenho em alpha máximo e só então
  desvanecem/liberam o Player.
- O estado é limpo ao cancelar a máscara, trocar cena ou expirar o timeout de
  segurança. Não toma câmera nem cria cutscene, que continuam sendo domínio do
  transformador nativo do OoTMM.
- `reverse_once` passou a usar o mesmo encerramento de override de `once`.
- Host/pacote: `39BCA5E3DA6C9DB1BA34640C3B04EA88F9B69B36180A8A150C3B381746FF59A3`.
- Mod 0.1.21: `76790578310345B53EC932903EC2CEC27541CA64FB8D770B679F1AF25246673E`.

## Combo Goron 0.1.22

- A sequência deixou de ser um contador Lua: B inicia `punch_a`; B durante A
  agenda B; B durante B agenda C. O buffer é guardado no bridge e não aceita o
  toque inicial no frame 0 como segundo golpe.
- `get_body()` expõe `punch_combo_queued`, e as recuperações só iniciam quando
  não há próximo golpe válido.
- Host/pacote: `D5AA658C98E4543047C2BF1AE681288129AF02448CEF0F5D4E442040E9DD758F`.
- Mod 0.1.22: `841F988CC3E279ABC1A8722D5632237B369859D62901275E41AB23914C617E85`.

## Bounce da bola 0.1.23

- No bonk de velocidade alta, o bridge reflete a orientação por `wallYaw`,
  conserva 85% da velocidade e reentra em `Player_SetupRoll`; o visual da bola
  não cai na animação de bonk humano. O cooldown é de quatro frames.
- `AT_HIT` do cylinder armado tem prioridade para objetos/dynapoly atingidos,
  e o getter mostra `roll_bounce_active`/`roll_bounce_frames`.
- Host/pacote: `624B1F75023DB2939D85D53F55F1CE0F89DDBA39CFDD779CB72C8A9061DDF422`.
- Mod 0.1.23: `E4AB5A76245C25ADB2D544A10EA9D4F94C0649FA2E90025909043D25D03F1496`.

## Recuperação dos socos 0.1.17

- O exemplo passou a declarar `pg_punchA/B/Cend` e as três variantes `endR`.
- Ao finalizar cada golpe A/B/C, o bridge toca a recuperação adequada à
  velocidade, com fallback parado. O mod empacotado é `0.1.17`.
- Host/pacote: `48E0496BEA945612912241360F91406940C7FC00A67666F06ABF357181711D5E`.
- Mod: `0F7955690D219B20BDA47D013E66258585B2BBFFABF47318AAEAE90130BAFE83`.

## Root motion dos socos

- `punch_a`, `punch_b` e `punch_c` agora passam a translacao da junta raiz por
  `SkelAnime_UpdateTranslation`; o primeiro frame e protegido por
  `ANIM_FLAG_NOMOVE`, e o delta sincroniza Player e ator hospedeiro.
- O Release recompilado e o host dentro do pacote coincidem no SHA-256
  `CB5DFC03323A33F3704C715883A951202F18FBC5F8B6785CE2D748D04CEA787E`.
- Ainda falta validar em jogo a distancia real e colisao dos socos. Combo e
  recoil de parede continuam pendentes.

## Arquivos alterados

- `../shipwright-limpo/Shipwright/soh/soh/ShipLuaBootstrap.cpp`
- `examples/goron-form/main.lua`
- `examples/goron-form/manifest.toml`
- `docs/wiki/Research-External-Item-Systems.md`
- `coordination/claims/OOT-GORON-001.md`

## Água funda 0.1.24

- A spec do Goron declara `water_void = true`; a água acima de 20 unidades
  inicia curl, bola sem giro, 15 frames de afundamento e o `Play_TriggerVoidOut`
  nativo. Item pendente adia o início; corpos sem a opção continuam nadando.
- O estado é limpo ao desativar corpo ou trocar de cena, e o getter retorna
  `water_void_active`, fase e contador para diagnóstico sem expor structs.
- Host/pacote: `62094DD512088D2457D964947218A886503B47923D4C721C715526FF445004B8`.
- Mod 0.1.24: `23DEFD7E752EDFFF20F2E2A30300C2FE3D2DF4E42BD95301875509ED186837C1`.

## Notas dos tambores 0.1.25

- Os assets `pg_gakkiwait` e `pg_gakkiplayA/L/D/U/R` foram declarados no mod.
  Durante a ocarina, o bridge encaminha A/C-esquerda/C-baixo/C-cima/C-direita
  como nota e o Lua escolhe a pose correspondente; entre notas usa `gakkiwait`.
- Os modelos de tambor continuam anexados para todas essas poses, não apenas
  durante a entrada e o loop genérico.
- Host/pacote: `36F098C6D4E87244E8AC416BF29F01CF3BEA3463CCC481E26316D572784C161F`.
- Mod 0.1.25: `5B47DE0E5B345C9C2692015409F055CACEF53DC4ED70D3FFACEDA266EA9568DE`.

## Notas dos tambores 0.1.26

- Corrigida a duração das notas: cada botão da ocarina inicia a animação
  direcional como one-shot; o seletor só pede `gakkiwait` depois disso.
- Host/pacote: `36F098C6D4E87244E8AC416BF29F01CF3BEA3463CCC481E26316D572784C161F`.
- Mod 0.1.26: `2ECDFDCE138FCD79B853267831C0B82B2A33A51EECF15D2495CE4585AD3E7AD1`.

## Efeito visual dos socos 0.1.27

- O display list nativo de efeito foi ligado às janelas de hit do combo: mão
  esquerda em A, mão direita em B e cintura em C, no passe Xlu vermelho.
- Host/pacote: `CB63CF0C6C735A904D05A197F91A95B9506DF1A713D73708D54F162523AB9C68`.
- Mod 0.1.27: `73D8A886A3AAE6E944CFD1D4F373310766EC72F4D14B68C6F6F18168396BBE34`.

## Bloqueio de borda 0.1.28

- O gate de ledge grab no `z_player.c` agora consulta o predicado opt-in do
  corpo externo, sem substituir o estado global de Crowd Control.
- Host/pacote: `4F722AE6127ED0285F261B637D73A4C9A35AEC5B9FB67EEBC91ECB7DE718C63D`.
- Mod 0.1.28: `B4E9C69BC80102D19F114AE7245E70C8703D767913A92A4DEBA2C635E03F0C58`.

## Bola na agua 0.1.29

- O renderer passa a escolher `gLinkGoronCurledDL` explicitamente na fase
  `water_void=Ball`; antes a fase alterava transform, mas podia cair no draw do
  skeleton ereto por nao estar em `Player_Action_Roll`.
- Durante esse void nao ha espinhos, energia, carga nem giro.
- Host/pacote: `1E07E5205523F90A13BE2FFECC4EC0DE64112F3C644B2D2A7B7F38CEB5257F0F`.
- Mod 0.1.29: `698CD7AF638B1CC2A5E3B69B649FB728F4807CA155A4375C9A73BCDC72B7FB00`.

## Escalada Goron 0.1.30

- `hook.oot.player.body_anim_select` recebe o estado de escada/vinha, direcao,
  passo alternado e indicador de entrada. A forma Goron seleciona os assets
  reais `pg_climb_*` do MM.
- `reverse_anims` e uma opcao validada de `set_body`; permite reutilizar um
  header em loop reverso. A descida Goron usa esse caminho para `upL/upR`.
- Host/pacote: `7FD2B267A2BF2C99F53F73C45BA4AD12A96C2EA92DBBAC4623B73646A9453DB5`.
- Mod 0.1.30: `0A07863CD06CA5F4B3DFAFE3152A9281BF36F4FD819C15E0DA04E67BD600C91D`.

## Porta e bau Goron 0.1.31

- O hook distingue a abertura confirmada de porta com macaneta e o inicio de
  bau. O mod usa os one-shots MM `pg_doorA/B_open` e `pg_Tbox_open` apenas
  nesses dois casos, mantendo logica de mundo e item no host OoT.
- Host/pacote: `80B256AB2F7876516352B22DC83AF0752CBCAC27A412A26F933F0900092F55D8`.
- Mod 0.1.31: `462DD1BC896722FF9D9D3E781763E092FB8450EA629D20B7F808DED4C582F470`.

## Getter completo da spec 0.1.32

- `get_body()` passou a retornar `segments` e `models` ativos, alem de
  animacoes, shield e configuracao reversa. Os caminhos sao os declarados pelo
  mod, nunca ponteiros/enderecos internos do renderer.
- Host/pacote: `0429EBBC16ED667D66711002851A9235773B2B45E25DD0B2F7EE5046E820CDE0`.
- Mod 0.1.32: `FE586C62B8196CB3F7A6C966DA02540C12252B9F33F202419DF616EDA6F43527`.

## Preflight completo de animacoes 0.1.33

- `set_body` valida e carrega todos os headers declarados antes de ativar o
  skeleton. O catalogo de 31 animacoes exclusivas `pg_*` do MM esta todo na
  spec Goron; a transformacao nao fica parcialmente utilizavel.
- Host/pacote: `19D853EFC66DD46CAC768ED6AAC6EDF724F2EA28EAE00DFEE8A5449224161C94`.
- Mod 0.1.33: `9FA54D5AACADC4038FFEDDD9C11B378BCC86CD8EAE04F18BBD52601BEEB642E4`.

## Validação executada

```powershell
cmake --build build/x64 --config Release --target soh -- /m:1
py -3 tools/shipmod.py validate examples/goron-form
py -3 tests/conformance/PackageHelloWorld.py examples/goron-form x64/packages/oot/Release/mods/goron-form.shipmod
py -3 tools/shipmod.py validate x64/packages/oot/Release/mods/goron-form.shipmod
```

O Release foi compilado. O host copiado para o pacote tem SHA-256
`1DE32D1D82A5A107D9A6F94AFD1A3D855B217B9DDE756A82B5E0AE498812F142` e coincide
com o host compilado. Todos os comandos validaram o pacote
`baiteryamato.goron_form` na versão `0.1.12`.

## Pendência

- Testar visualmente em jogo, em terreno plano e inclinado, segurar R na forma
  Goron e receber dano. Se os pés ainda não coincidirem com o chão, usar a
  diferença observada em unidades de mundo para ajustar somente
  `ground_offset`; não alterar `roll_offset` junto. A defesa implementada é
  visual e preserva o bloqueio nativo do OoT; as regras completas de
  dano/deflexão do Goron MM continuam pendentes de uma capability física
  dedicada.
- Testar a ocarina: os seis modelos de tambor devem surgir no torso durante
  `gakki_start`/`gakki_play` e sumir na saida `reverse_once`.
- Testar a entrada: G deve bloquear Link no chão, tocar a máscara humana,
  cobrir a troca com flash branco e liberar input apenas depois do fade.
- Testar o combo: B, B durante A, B durante B deve mostrar A→B→C e manter as
  recuperações somente após o último golpe.
- Testar bounce: em velocidade alta, uma parede sólida deve refletir a bola;
  objetos/dynapoly acertados pelo cylinder não devem ser antecipados pelo bounce.
