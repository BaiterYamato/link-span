# Handoff — OOT-GORON-003: portar a máquina de formas do skijer

## Estado

**Referência rápida de caminhos, assets e funções:**
[`OOT-GORON-003-quick-reference.md`](OOT-GORON-003-quick-reference.md) — assets
do `mm.o2r` com contagem de frames, as funções do decomp com linha exata, e os
fatos do host que já custaram ciclo. Consulte antes de procurar qualquer coisa
no código.

**Leia a seção `AUDITORIA DE 01/08/2026` logo abaixo antes de qualquer coisa.**
Ela corrige treze afirmações deste handoff que se provaram falsas quando o port
foi conferido contra o decomp do MM, e estabelece qual fonte usar daqui pra
frente.

Incrementos 1 a 5 concluídos. A bola está portada **e ligada**; o combo de
socos migrou do host para a máquina **e está ligado**. Defesa e instrumento
também migraram para a máquina e passaram em runtime.

Em 27 e 28/07/2026 a Etapa 1 foi executada no `soh.exe` correto e concluída.
Ground pound, espinhos, água/void e ocarina passaram em runtime com entrada
sustentada. O vazamento dos olhos assustados após água/respawn foi corrigido e
retestado. A Etapa 2 também está implementada e comprovada em runtime. Na Etapa
3, defesa e instrumento foram migrados, compilados e comprovados em runtime.
Em 29/07/2026, `GoronAction::Land` e `GoronAction::Damage` também entraram na
máquina e passaram em runtime. A Etapa 4 está concluída. Em 30/07/2026, o
primeiro incremento visual da Etapa 5 e a cutscene da máscara passaram em
runtime: squash/tilt/kick e coloração da bola são consumidos no draw; a
transformação mostra a máscara, executa a composição azul-branca/flash, troca o
corpo no pico e restaura câmera, input e ambiente sem loop. A dívida restante
da Etapa 5 é o áudio próprio dos tambores e os itens explicitamente mantidos
fora de escopo abaixo.

## AUDITORIA DE 01/08/2026 — a referência do skijer é fonte RUIM

**O achado mais importante desta tarefa, e o que muda o método daqui pra frente:**
o decomp do Majora's Mask está no próprio workspace, em
`D:\Desenvolvimento\ship-lua-worktrees\upstream\2ship2harkinian`. É código-fonte
comentado, com nomes de símbolo e as tabelas originais.

Todo o port foi feito contra `research/skijer-transformation-masks/`, que é o
fork de um terceiro. Uma auditoria da bola contra a fonte primária achou **dez**
divergências. A referência erra de três maneiras distintas:

- **inventa** o que não existe (o "kick visual" de parede; o amortecimento de
  curva em baixa velocidade; o multiplicador de giro reduzido; a exceção de dyna
  no quique);
- **omite** o que existe (`al_hensin`, a máscara de boca aberta, a anti-reversão,
  a saída da bola por impacto frontal, os 20 frames sem controle);
- **escreve pela metade** (arma `unk_B8C` e nunca o lê; lê `unk_B8E` e nunca o
  arma — a mecânica inteira do quique ficou solta).

Regra que vale a partir de agora: **antes de portar ou corrigir qualquer coisa,
ler a função no `upstream/2ship2harkinian`.** A `research/` serve para achar o
NOME da função e a estrutura geral; os números e o comportamento vêm do decomp.

### Corrigido nesta auditoria (compila, zero aviso, NÃO testado em jogo)

| # | o que estava errado | fonte primária |
|---|---|---|
| 1 | `rollWallBounceTimer` armado e nunca lido; `rollNoInputTimer` lido e nunca armado | `z_player.c:19949-19959` |
| 2 | quique bloqueava velocidade **e** direção; o MM trava só a direção | `:19956` |
| 3 | "kick visual" de parede — invenção, o MM não tem | `:19932-19944` |
| 4 | taxa de curva com ramo inventado abaixo de 2.0 | `:20106` |
| 5 | faltava o piso do giro (`CLAMP_MIN(spDC, spBC*500)`) | `:20041-20047` |
| 6 | tombamento com passo fixo `0x190` e sem os dois clamps | `:20112-20124` |
| 7 | cancelamento de espinhos não armava os 20 frames sem controle | `:20137` |
| 8 | anti-reversão inexistente | `:8578`, chamada em `:20061` |
| 9 | normal da ladeira aproximada por sin/cos em vez da normal real | `:20130` |
| 10 | aceleração decidida pelo giro em vez de `spC0` | `:20044`, `:20072` |
| 11 | impacto FRONTAL não existia — só havia reflexão de raspão | `:19921-19931` |
| 12 | saída da bola tinha uma condição só; o MM tem duas | `:19845-19847` |
| 13 | trajetória inicial vinha de `shape.rot.y` em vez de `yaw` | `:19870` |

O item 12 é o provável knockback que o usuário relatou três vezes: bater de
frente **com espinhos** marca carga 3, a bola é jogada para cima, e ao começar a
cair o Goron **sai da bola no ar**. Nada disso existia.

### Cutscene da máscara — o que a auditoria desmentiu

O handoff afirmava "pose de sofrimento" na timeline. Era `cl_setmaskend`, de
**três frames**, em loop por dezoito. A animação real da transformação é
`gPlayerAnim_al_hensin` (51 frames) mais `al_hensin_loop` (48) — existem só no
`mm.o2r`, são `PLAYER_CSACTION_48` no decomp (`z_player.c:20612` e `:20755`), e a
referência do skijer nunca as menciona.

A máscara também **troca de malha**: a partir do frame 51 o MM soma 4 ao índice
da tabela `D_801C0B20` (`z_player_lib.c:4049-4056`), indo de `gGoronMaskDL` para
`object_mask_goron_DL_0014A0` — a de boca escancarada. Desenhávamos a fechada do
começo ao fim.

Também corrigido: `al_hensin_loop` estava em `ANIMMODE_LOOP` e travava o rosto
contorcido **depois de tirar a máscara** (um loop nunca termina, e
`Player_Action_Idle` só troca quando termina, `z_player.c:8358`); e a cena passou
a pausar a action func **e** avançar a animação sozinha, porque
`Player_Action_Idle` é quem chama `LinkAnimation_Update` (`:8349`) e quem rouba a
animação com um fidget (`:8370`).

### Áudio dos tambores — destravado, e o diagnóstico anterior estava errado

Estava registrado como "bloqueado por falta dos IDs das cinco notas". **Não
existem cinco IDs.** `NA_SE_OC_OCARINA` é `0x5800` — um só — e o MM não toca nota
por SFX: troca o INSTRUMENTO do motor de ocarina. O enum está em
`mm/include/z64ocarina.h` com `OCARINA_INSTRUMENT_GORON_DRUMS = 7`.

No OoT, `AudioOcarina_SetInstrument` (`soh/src/code/code_800EC960.c:1746`) emite
`Audio_SeqCmd8(SEQ_PLAYER_SFX, 1, SFX_PLAYER_CHANNEL_OCARINA, id)` — o
instrumento é índice dentro da sequência 0, e a do OoT só vai até 6. Mas nós já
interpretamos a sequência 0 **do MM**: `MmSeq_PlaySfx`
(`soh/soh/mmaudio/mmseq/MmAudioEngine.cpp:408`) escreve direto no canal do banco,
e o banco da ocarina é o 5 — o mesmo canal que o comando de instrumento
endereça. Falta a chamada equivalente ao `SeqCmd8`, não portar samples.

## PLANO — a ordem, e por que ela é essa

Fazer fora de ordem custa ciclo. Cada etapa depende da anterior ter sido
**verificada em jogo**, não só compilada.

### Etapa 1 — testar o que já está no ar (BLOQUEIA todo o resto)

Duas trocas de dono entraram sem nunca rodar um frame. Migrar mais nada por cima
disso é empilhar risco sobre risco não medido.

Executável (é o único lugar onde este trabalho existe):
`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\soh.exe`

**Não** usar `RELEASE-UNIFICADA\OOT\soh.exe` — instalação separada, exe de
21/07, com um `goron-form.shipmod` diferente (1.4K contra 8.1K).

O que olhar, em ordem de risco:

1. **A no chão** → curl → bola; soltar A desenrola sem pop de posição. Bola
   presa no curl ⇒ suspeitar de `animationFinished` (cairia em
   `kRollInitTimeoutFrames`).
2. **B no chão** → combo; B durante o golpe e durante a recuperação encadeia; o
   terceiro volta para A. Link deslizando durante o soco ⇒ suspeitar da pausa da
   action func.
3. **B rolando** → ground pound, quake, quebra o que responde a martelo.
4. **Segurar A rolando** → espinhos, custo 2 de magia, dreno 1 a cada 10 frames.
5. **Rolar para dentro d'água** → void sem a bola disputar.
6. **R** ainda defende, **ocarina** ainda toca — os dois caminhos que continuam
   no host, protegidos pelos gates de entrada.

Antes de rodar, arquivar `logs/Ship of Harkinian.log`: o arquivo é sobrescrito
por sessão e sobra cauda de sessões antigas, o que torna a leitura ambígua. A
impressão digital confiável para saber QUAL build gerou uma linha é o número de
linha em `[ShipLuaBootstrap.cpp:NNNN]` — comparar com o arquivo em disco.

### Etapa 2 — corrigir o gatilho do rolamento (CONCLUÍDA E TESTADA)

Já sabido, pendente só de confirmação pelo teste. `TryStartRoll` lê o botão A
cru, mas **A no chão em movimento já é o roll de esquiva do OoT**: os dois entram
no mesmo frame e a pausa da action func só vale no frame seguinte, depois de a
esquiva ter dado o impulso dela. Sintoma no log: `actRoll=1` junto de
`anim=roll_enter`.

A referência não disputa o botão — ela **converte** o `Player_SetupRoll` do OoT
em curl (`MmForm_StartGoronCurlFromOot`, `:14660`). O host já detecta essa
condição (`nativeRolling`), então a peça existe. Como a máquina não pode
referenciar `Player_Action_Roll` (símbolo do z_player), o host informa por um
gate novo, no mesmo desenho de `SetBallEntryBlocked`.

Vem junto: ao sair da bola o Player ainda tem `actionFunc == Player_Action_Roll`
pausado, e retomaria a esquiva ao despausar. Precisa voltar para
`Player_Action_Idle`.

Resultado de 27/07/2026:

- o host informa `player->actionFunc == Player_Action_Roll` à máquina por
  `SetOotRollActive`;
- a máquina converte esse estado antes de `PlayerOwnsBody` ceder a pose;
- na entrada e na saída da bola o host devolve `actionFunc` a
  `Player_Action_Idle`;
- sequência observada no log:
  `roll_init → roll → roll_uncurl → idle`, sempre com `actRoll=0` depois da
  conversão. A esquiva nativa não retoma ao desenrolar.

### Resultado parcial da Etapa 1 — runtime de 27/07/2026

Build realmente executada:

- `x64/Release/soh.exe`, 41.782.784 bytes, timestamp `27/07/2026 02:06:49`;
- SHA-256
  `8B15618E245B34D03F8B18E16B48C30131FBACE39D9E9703D71E08FEA1952B2B`;
- impressão digital da sessão:
  `[ShipLuaBootstrap.cpp:5930]` inicializando e
  `[ShipLuaBootstrap.cpp:5938]` inicializado.

O que passou:

1. **A no chão**: `roll_init → roll → roll_uncurl → idle`; soltar A desenrola
   e não retoma a esquiva do OoT.
2. **B no chão**: `punch_a → punch_b → punch_c → punch_end`; entradas B
   repetidas durante golpe/recuperação encadeiam, e um novo ciclo volta a A.
3. **Root motion do soco**: o primeiro teste revelou o Goron suspenso acima da
   sombra depois da recuperação. A causa era ordem da fila, não ausência de
   `ANIM_FLAG_UPDATEY`: `LinkAnimation_Update` só enfileira `LOADFRAME`, então
   a chamada direta a `SkelAnime_UpdateTranslation` lia a pose anterior e era
   sobrescrita. O host agora enfileira `MOVEACTOR` depois de `LOADFRAME`, define
   `baseTransl=(-57,3377,0)` e refaz a matriz do hospedeiro no draw. Em runtime,
   `jointTable[0]` volta à base durante soco, recuperação e idle, enquanto
   `playerY=0`; a malha voltou ao piso e X/Z do Player recebem os deltas reais.
4. **R**: o pulso entrou no caminho de defesa (`action=2`) e voltou a
   `action=0`, sem input preso. Em validação manual posterior, o modelo exibido
   pela defesa continuou parcialmente quebrado. Continua sendo o caminho antigo
   do host, como previsto para antes da Etapa 3.
5. **Ground pound/stomp**: validação manual posterior indica que o salto, a
   queda e o stomp aparentam funcionar corretamente.
6. **Ocarina/tambores**: todas as animações foram observadas funcionando na
   validação manual posterior. O defeito restante aparenta ser somente a
   ausência do áudio próprio dos tambores.
7. **Espinhos**: validação manual posterior indica que a transformação da bola
   com espinhos e seu comportamento geral estão funcionando.
8. **Água/void**: a mecânica de entrada na água e respawn funciona, mas o estado
   visual não é liberado por completo: os olhos assustados continuam ativos
   depois de sair da água e reaparecer na última porta.

Os itens 4 a 8 acima incluem relato manual do usuário posterior à sessão
automatizada; esta atualização não trouxe um novo log/fingerprint para anexar.

Defeito novo a corrigir antes de fechar a Etapa 1:

- **olhos assustados presos após água/respawn**: `surprisedTimer` é armado no
  hook de dano e só é decrementado pelo callback Lua `game.frame`
  (`examples/goron-form/main.lua`). Verificar primeiro se esse evento é
  realmente despachado durante e depois do void; em teste anterior o host
  atualizava timers, mas não entregava `game.frame`. A correção precisa restaurar
  `EYES[1]` tanto no fim normal do timer quanto na troca de cena/respawn e na
  liberação do corpo.

Logs arquivados mais úteis:

- `logs/Ship of Harkinian.OOT-GORON-003-etapa1-queued-rootmotion-20260727-0211.log`
  — soco isolado e combo completo depois da correção;
- `logs/Ship of Harkinian.OOT-GORON-003-etapa1-roll-shield-20260727-0214.log`
  — conversão do rolamento, tentativas documentadas de ground pound e pulso R.

### Resultado final da Etapa 1 — runtime de 28/07/2026

**Etapa 1 concluída.** Os quatro casos manuais passaram:

1. **Ground pound**: A foi mantido até permanecer na bola; B iniciou a subida,
   seguida de queda, quake/impacto e retorno ao roll.
2. **Espinhos**: A foi mantido de forma contínua durante o roll; houve custo
   inicial de 2 de magia, aumento de velocidade, dreno de 1 a cada 10 frames e
   desligamento limpo.
3. **Água/void**: a máquina percorreu `water=1 → 2 → 3 → 0`, sem disputa entre
   duas bolas. Após o respawn, input, gravidade, colliders e sombra voltaram; uma
   entrada de movimento posterior confirmou controle. O corpo e os olhos
   retornaram ao estado normal.
4. **Ocarina e R**: a ocarina entrou, recebeu as cinco notas, saiu e devolveu
   controle. A regressão curta de R entrou e saiu sem prender input. O áudio
   próprio dos tambores continua dívida separada da Etapa 5.

O defeito dos olhos era causado pela ausência do despacho documentado de
`game.frame`: o host avançava `FrameTimerScheduler`, mas o callback Lua que
decrementa `surprisedTimer` nunca rodava. O host agora despacha `game.frame`
depois do `Tick()` bem-sucedido, e o mod restaura `EYES[1]` no fim do timer, na
troca/liberação do corpo e depois da reativação.

Build do reteste final:

- `x64/Release/soh.exe`, 41.782.784 bytes, timestamp `28/07/2026 15:29:49`;
- SHA-256
  `240413AF3521554E13906CF7441AEC20009A9EF864044C75C30BA34B60D67EB1`;
- impressão digital da sessão:
  `[ShipLuaBootstrap.cpp:5937]` inicializando,
  `[ShipLuaBootstrap.cpp:5945]` inicializado e
  `[ShipLuaBootstrap.cpp:6002]` finalizado;
- pacote ativo `goron-form.shipmod`:
  `6D6BF09F738CB76F0E56CDC4072AE59A14B0D4D85DEF23EC58CC651D9DA44622`;
- fonte `main.lua` dentro do pacote:
  `C41D5B2F429AA709A9AED9544A8B61CAB304C9CC676F42AE161FCD781CECFE5B`.

Sessões e logs arquivados:

- `runtime-session-OOT-GORON-003-etapa1-gate-20260728-140658.result.json` e
  `Ship of Harkinian.OOT-GORON-003-etapa1-gate-20260728-140658.post.log`;
- `runtime-session-OOT-GORON-003-etapa1-gate2-20260728-145850.result.json` e
  `Ship of Harkinian.OOT-GORON-003-etapa1-gate2-20260728-145850.post.log`;
- `runtime-session-OOT-GORON-003-etapa1-waterfix-20260728-1532.result.json` e
  `Ship of Harkinian.OOT-GORON-003-etapa1-waterfix-20260728-1532.post.log`.

O gate de proveniência confirmou fonte = pacote, executável correto, fingerprint
compatível com o fonte atual e nenhum uso do executável proibido em
`RELEASE-UNIFICADA\OOT`.

### Etapa 3 — migrar defesa e instrumento

Receita já estabelecida nos incrementos 4 e 5: chave de posse, lógica para dentro
da máquina, caminho antigo desligado **no mesmo commit**, `ReleaseBody` cobrindo
a saída.

- **Defesa**: o esqueleto de 4 limbs (`gCustomBodyShieldSkelAnime`) fica no host
  — é alocado da spec e a máquina não conhece asset. Ela só diz "estou em
  Shield"; mesmo arranjo do root motion do soco.
- **Instrumento**: `Audio_OcaGetPlayingStaff()` é engine do OoT, então a máquina
  lê a nota direto. **Armadilha 4**: nunca pausar a action func durante a
  ocarina.

Resultado da etapa: `SetBallEntryBlocked` e `SetPunchEntryBlocked` foram
substituídos por um único `SetEntryBlocked`; a máquina enxerga defesa e
instrumento por conta própria, e o host informa apenas blockers externos.

#### Resultado parcial da Etapa 3 — defesa em runtime de 28/07/2026

- A máquina passou a ser a única dona da entrada, sustentação e saída de R.
  O host conserva somente o `gCustomBodyShieldSkelAnime` de 4 limbs, seu update
  visual e o collider.
- A entrada foi comprovada por `mmAction=shield` com `action=0`: o caminho
  antigo `CustomBodyAction::Shield` não existe mais e não disputou o Player.
- R sustentado manteve a defesa estável e R solto devolveu `idle`, movimento e
  controle. A silhueta encolhida, com cabeça e pés recolhidos sob a carapaça
  espinhosa, corresponde ao skeleton dedicado de shielding; não houve perda ou
  oscilação de limbs.
- Regressões curtas passaram no mesmo ciclo:
  `roll_init → roll → roll_uncurl → idle` e
  `punch_a → punch_end → idle`.
- Build usada: `soh.exe` com 41.783.808 bytes, timestamp
  `28/07/2026 16:14:44` e SHA-256
  `D77874980F264B5F5CDEE1AA5F2CE1B6C4330B857A006B6BBF6C3091C94BFFB3`.
- Fingerprint no log arquivado:
  `[ShipLuaBootstrap.cpp:5931] ShipLua inicializando para`,
  `[ShipLuaBootstrap.cpp:5939] ShipLua inicializado` e
  `[ShipLuaBootstrap.cpp:5996] ShipLua finalizado`, compatíveis com as 6.040
  linhas do fonte testado.
- Sessão: `OOT-GORON-003-etapa3-shield-20260728-1618`.
  Evidência principal:
  `logs/Ship of Harkinian.OOT-GORON-003-etapa3-shield-20260728-1618.post.log`,
  oito capturas `logs/OOT-GORON-003-etapa3-shield-01-before.png` até
  `-08-settled.png` e o resultado estrito
  `logs/runtime-session-OOT-GORON-003-etapa3-shield-20260728-1618.result.json`.
- O gate de proveniência passou: fonte = pacote, executável correto, hash
  esperado e nenhum uso de `RELEASE-UNIFICADA\OOT`.

#### Resultado final da Etapa 3 — instrumento em runtime de 28/07/2026

**Etapa 3 concluída.** A defesa continuou verde e o instrumento passou a ser
dirigido pela máquina:

- `CustomBodyAction::Gakki` e o estado duplicado do host foram removidos. A
  máquina inicia `GoronAction::Gakki` antes da cessão genérica a
  `OotAction`, porque a ocarina vive em `PLAYER_STATE1_IN_ITEM_CS`.
- As três fases da referência estão ativas: abertura (`gakki=1`), espera/notas
  (`gakki=2`) e fechamento reverso (`gakki=3`). A action func do Player
  **nunca é pausada** durante o instrumento.
- `Audio_OcaGetPlayingStaff()` é lido diretamente pela máquina. O runtime
  comprovou as cinco animações:
  `gakki_play_a`, `gakki_play_d`, `gakki_play_r`, `gakki_play_l` e
  `gakki_play_u`, sempre retornando a `gakki_wait`.
- A saída percorreu `gakki=3 → idle`, com `ocarina=0`. Movimento posterior
  produziu `walk → run → walk → idle`, comprovando a liberação do controle.
- A regressão de R passou no mesmo ciclo:
  `idle → shield → idle`, seguida de novo movimento.
- Os gates separados `SetBallEntryBlocked` e `SetPunchEntryBlocked` foram
  consolidados em `SetEntryBlocked`. Água/void, transição de máscara e ação
  legada do host continuam bloqueando **somente novas entradas**.

Build usada:

- `x64/Release/soh.exe`, 41.781.248 bytes, timestamp
  `28/07/2026 16:34:18`;
- SHA-256
  `551CFBA2A00836D818B5E37EBE7846D7A0541C657A3CD3C5C88554A09FAD41CF`;
- fonte `ShipLuaBootstrap.cpp` com 5.894 linhas;
- fingerprint no log:
  `[ShipLuaBootstrap.cpp:5785] ShipLua inicializando para`,
  `[ShipLuaBootstrap.cpp:5793] ShipLua inicializado`,
  `[ShipLuaBootstrap.cpp:3245] ShipLua corpo:` e
  `[ShipLuaBootstrap.cpp:5850] ShipLua finalizado`.

Evidência:

- sessão `OOT-GORON-003-etapa3-gakki-20260728-1636`;
- log arquivado
  `logs/Ship of Harkinian.OOT-GORON-003-etapa3-gakki-20260728-1636.post.log`;
- resultado estrito
  `logs/runtime-session-OOT-GORON-003-etapa3-gakki-20260728-1636.result.json`;
- sequência e resultado
  `logs/runtime-input-OOT-GORON-003-etapa3-gakki.json` e
  `logs/runtime-input-OOT-GORON-003-etapa3-gakki.result.json`;
- doze capturas, de `logs/OOT-GORON-003-etapa3-gakki-01-before.png` a
  `-12-settled.png`.

O encerramento estrito confirmou executável, fontes, pacote e save inalterados.
O `shipofharkinian.json`, alterado apenas pelo estado da janela/menu, foi
restaurado pela fixture verde antes do fechamento. Save ativo:
`31CCB6D12374780EF8F02A66B989661B48D6D9F253961C820C4905FD2F81FFEB`.
O gate de proveniência passou com fonte = pacote, executável correto, hash
esperado e nenhum uso de `RELEASE-UNIFICADA\OOT`.

### Etapa 4 — as ações que nunca existiram

`GoronAction::Damage` (knockback, `MmForm_GoronAction_Damage`) e
`GoronAction::Land`. Ao abrir a Etapa 4, ambos estavam no enum e nunca eram
alcançados, nem no host nem na máquina. Os dois foram concluídos nos resultados
registrados abaixo.

Plano adotado para a Etapa 4:

1. reconstruir primeiro o estado vivo e separar dois problemas:
   `Land` é recuperação/pose após a borda ar→chão; `Damage` é knockback e
   recuperação depois de o OoT já aplicar o dano;
2. começar por `Land`, de menor risco: detectar a borda de aterrissagem fora de
   bola, água/void e ações bloqueadoras; tocar `land` uma vez, desacelerar e
   voltar a `idle`. No mesmo incremento, retirar do Lua o bloco
   `payload.landing` que tocava `land` por fora da máquina;
3. testar `Land` em queda curta e longa, comprovando `fall → land → idle`,
   posição no piso, input e ausência de repetição da animação;
4. só então portar `Damage`: estudar juntos `MmForm_CheckDamage`
   (`mm_player_form.cpp:4882`) e `MmForm_GoronAction_Damage`
   (`:5357`). Não reaplicar vida/dano já consumido pelo OoT. Criar ponte do
   host apenas se a máquina não conseguir observar com segurança o estado
   danificado e o yaw/velocidade de knockback;
5. quando a máquina possuir a pose de dano, remover no mesmo incremento o
   `play_body_animation("damage")` do hook Lua de `health_change`, preservando
   apenas olhos assustados e o timer visual;
6. testar dano no chão, dano com lançamento/queda, transição para `Land`,
   recuperação do controle e regressões curtas de R, roll e ocarina.

#### Resultado de Land — runtime de 29/07/2026

`Land` passou. A borda ar→chão agora é da máquina:

- `Jump`/`Fall` no frame anterior e chão no frame atual entram uma única vez em
  `GoronAction::Land`;
- a queda acumulada mantém o corte da referência: até 80 unidades acelera
  `land` para `1.5`; acima disso toca a animação completa em `1.0` e aplica o
  primeiro passo de desaceleração de `3.0`;
- durante `Land`, a máquina desacelera `linearVelocity` por `8.0` e volta a
  `Idle` quando o one-shot termina;
- a action func do OoT não é pausada: a máquina possui pose/recuperação, não o
  movimento nativo;
- o driver Lua `payload.landing` e seu estado `landingActive` foram removidos.
  O hook de `health_change` permaneceu intacto para o incremento de `Damage`.

Build Release sem erro nem aviso:

`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\soh.exe`

SHA-256:
`402059A71E5170A696006414DB9000838091BBA2F21386BF2F8A18F05FD68CEC`

Pacote ativo validado:
`mods\goron-form.shipmod`, SHA-256
`50D1BD1ED282E5C33FBCF455DD980F952146B48B1D3DD75B9F0B47021411156C`.

Sessão estrita:
`runtime-session-OOT-GORON-003-etapa4-land-20260729-1951.json`, encerrada com
`valid=true` e todos os caminhos rastreados inalterados. Log arquivado:

`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\Ship of Harkinian.OOT-GORON-003-etapa4-land-20260729-1951.post.log`

SHA-256 do log:
`C9173E4582C16AE553BD24372615AD6776DDC12C8B3B8EF15DC96CBB887049A9`.

Evidência observada:

- queda curta: pico registrado em `playerY=hostY=28.293753`, seguido de
  `jump → fall → land → idle`;
- queda longa: pico registrado em `playerY=hostY=163.49063`, seguido da mesma
  sequência;
- nas duas aterrissagens, `Land` entrou em `playerY=hostY=0`;
- depois de cada recuperação, o input produziu
  `walk → run → walk → idle`;
- cada queda produziu exatamente uma entrada `Land`; a espera posterior ficou
  em `Idle`, sem loop.

As fixtures temporárias foram restauradas: nenhum `soh.exe` ficou aberto, o
save não mudou e `shipofharkinian.json` voltou ao SHA-256
`4018E36895850A52E7B97287690ECDB34F9348BE1A72C8BE5B78983A147F0401`.

#### Resultado de Damage — runtime de 29/07/2026

`Damage` passou. A implementação segue a divisão de posse observada em
`MmForm_CheckDamage` e `MmForm_GoronAction_Damage`:

- a máquina observa somente a nova borda de `PLAYER_STATE1_DAMAGED`;
- o OoT continua dono de vida, dano, invencibilidade, hit, yaw e velocidades do
  knockback. A máquina não reaplica nenhum desses valores;
- `GoronAction::Damage` possui apenas a pose e a recuperação. No ar, acompanha a
  física nativa e segue por `Fall`/`Land`; no chão, volta a `Idle` quando o
  one-shot termina;
- a action func do Player não é pausada;
- o driver Lua `play_body_animation("damage")` foi removido do hook
  `health_change`. Olhos assustados e timer visual foram preservados.

Build Release final sem erro nem aviso:

`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\soh.exe`

SHA-256:
`80E70F9498B6C9A3D52067966BD97B5E983036A062D148E3B8AFE7ECFF723C56`

Pacote ativo validado:
`mods\goron-form.shipmod`, SHA-256
`7EB30D436661FCE55883F0253414406EB61860A9F1A9D4FDD437363278147B89`.

Sessão estrita de `Damage`:
`runtime-session-OOT-GORON-003-etapa4-damage-20260729-2019.json`, encerrada com
`valid=true` e todos os caminhos rastreados inalterados. Log arquivado:

`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\Ship of Harkinian.OOT-GORON-003-etapa4-damage-20260729-2019.post.log`

SHA-256 do log:
`3DAD8F3F5805558BFB651904FC9F9404483BD500FBB045654213A44DA56FD459`.

Evidência observada:

- uma bomba nativa, criada por `spawn 16 0`, causou dano real no chão:
  `hp=48 → 40` uma vez, `damaged=1`, pose `damage` e olhos assustados;
- o knockback desse golpe permaneceu nativo: `linear=15 → 7 → 0`, sem
  desaceleração reaplicada pela máquina, seguido de `damage → idle` e limpeza do
  flag;
- o caso aéreo válido, por `knockback 1`, manteve `hp=24` durante todo o trecho,
  percorreu `vy=5 → 4 → 3 → 2 → 1 → 0 → -1`,
  `damage → land → idle`, tocou `Land` uma vez em `playerY=hostY=14` e devolveu
  o input (`walk`);
- uma tentativa preparatória com `knockback 3` saiu da colisão e caiu no void;
  ela foi descartada como gate e explica o `hp=24` inicial do trecho aéreo
  válido;
- não houve loop de `Damage` nem de `Land`.

Sessão estrita de regressão:
`runtime-session-OOT-GORON-003-etapa4-regression-20260729-2038.json`, também
encerrada com `valid=true`. Log arquivado:

`D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\Ship of Harkinian.OOT-GORON-003-etapa4-regression-20260729-2038.post.log`

SHA-256 do log:
`B00E193D2F392AF84F986B6C831898229DA01CA941D8F087B0A3A93D49BEE1DC`.

As regressões curtas passaram:

- R no chão: `idle → shield → idle → walk/run → idle`;
- rolamento: `roll_init → roll → roll_uncurl → idle`;
- instrumento: `gakki=1 → 2 → 3 → idle`, com
  `gakki_play_a/d/r/l/u`, seguido de movimento. A action func permaneceu ativa
  durante a ocarina.

Ao fechar as duas sessões, nenhum `soh.exe` ficou aberto. O save permaneceu em
`31CCB6D12374780EF8F02A66B989661B48D6D9F253961C820C4905FD2F81FFEB` e
`shipofharkinian.json` foi restaurado ao SHA-256
`4018E36895850A52E7B97287690ECDB34F9348BE1A72C8BE5B78983A147F0401`.

Etapa 4 concluída. A Etapa 5 está liberada.

### Etapa 5 — dívida visual e de áudio

- `rollSquash`, `rollColorLerp`, `rollDriftYaw` e o timer/energia do bounce
  agora têm acessores read-only e são consumidos por `CustomBodyDrawRoll`.
  Squash, tilt, coloração azul e kick visual curto foram ligados sem duplicar a
  reflexão ou alterar a física.
- Recuo de parede do soco (`MmForm_CheckWallHit`): não portado.
- Crack decal do ground pound: fora de escopo (desenho por frame).
- Áudio do MM real (loops contínuos, ids `MM_NA_SE_*`): enquanto for disparo
  único, o zumbido da carga não sai e `StopRollSfx()` fica vazia.
- Validação manual confirmou as animações de ocarina/tambores, mas o áudio
  próprio dos tambores aparenta continuar ausente.

#### Prioridade visual pedida pelo usuário — squash e kick de parede

O quique físico da bola **já existe** na máquina: parede sólida em velocidade
alta reflete `yaw`, acumula `rollBounce`, arma `rollWallBounceTimer=4` e toca o
baque; a exceção de dyna/objeto atingido pelo cylinder evita quicar antes de
quebrá-lo. Não criar uma segunda detecção ou uma segunda reflexão no host.

O incremento visual deve:

1. publicar acessores read-only para `rollSquash`, `rollDriftYaw`,
   `rollColorLerp` e o timer/energia necessários ao feedback do impacto;
2. fazer `CustomBodyDrawRoll` consumir o squash da referência:
   `Y = 1 + squash`, `X = 1 - squash*0.5` e
   `Z = max(X,Y)`, preservando a escala-base atual da bola;
3. aplicar o tilt direcional já calculado por `rollDriftYaw`, sem alterar
   `actor.shape.rot` nem a trajetória física;
4. usar o `rollWallBounceTimer` existente para um kick visual curto e leve
   contra a parede. O pulso deve apenas deslocar/inclinar/deformar a malha por
   até quatro frames; nunca alterar novamente `yaw`, velocidade, collider ou
   `Player_Action`;
5. validar em runtime: rolagem normal sem jitter, squash visível na cadência,
   uma única reflexão/kick por impacto rápido, nenhuma resposta em parede
   abaixo do limiar, e dyna quebrável mantendo prioridade do `AT_HIT`.

Esse é o primeiro incremento da Etapa 5 depois de `Land` e `Damage`.

#### Prioridade visual pedida pelo usuário — cutscene ao vestir a máscara

O usuário também quer a apresentação completa de transformação mostrada na
referência visual: close-up frontal da máscara, grito/dor, fundo azul-branco em
movimento, estouro de luz e retorno limpo à câmera normal. Não considerar
`gPlayerAnim_cl_setmask` isoladamente como entrega desse requisito.

O caminho atual já tem peças parciais:

- `play_mask_on_animation()` toca `gPlayerAnim_cl_setmask` no Link humano;
- `oot.cutscene` usa `style="jump"` e gira o Player para a câmera;
- o exemplo aplica rampa de luz/fog e o host cobre a troca com flash branco;
- o corpo Goron entra no pico do flash.

Ainda falta compor essas peças como uma cutscene única e reproduzir o plano
fechado da referência, incluindo a animação/pose de sofrimento, o grito e os
SFX nos tempos corretos e o efeito azul-branco ao redor da máscara. Antes de
implementar, identificar quais assets do `mm.o2r` produzem a máscara expressiva
da cena; não substituir silenciosamente o efeito por uma máscara estática.

Fonte externa indicada pelo usuário para essa pesquisa:

- [Tabela de assets do MM no 2S2H](https://docs.google.com/spreadsheets/u/0/d/1Ke2OSwFBqpn7HG9bWQHS0eeyEOFukItEraZSbJdCFsE/htmlview#gid=0)
  — consultar para localizar e cruzar nomes de animações, modelos, texturas e
  efeitos do `mm.o2r`. O conteúdo da planilha não foi validado nesta sessão
  porque o `htmlview` público não respondeu pelo leitor web; preservar a URL
  exatamente como fornecida e confirmar os assets no runtime antes de usá-los.

Critérios de aceite:

1. recusar a entrada durante outra cutscene, diálogo, morte, ocarina,
   água/void ou ação que já possua o corpo;
2. fixar posição e movimento durante a apresentação sem pausar uma action func
   de ocarina;
3. executar close-up, animação/pose, máscara expressiva, rampa azul-branca,
   SFX/grito e flash em uma linha de tempo determinística;
4. trocar para o corpo Goron somente sob o pico do flash;
5. restaurar câmera, input, ambiente, luz pontual e qualquer efeito temporário
   tanto no fim normal quanto em cancelamento, troca de cena, morte ou unload;
6. testar entrada, saída, repetição, cancelamento e regressões curtas de roll,
   R e ocarina, sem câmera ou input presos.

Tratar esse trabalho como incremento visual separado da máquina de ações. A
referência skijer em `MmForm_UpdateTransforming` fornece fases, flash e tempos
de SFX, mas não comprova por si só todos os assets e o efeito de tela da imagem.
Se a composição azul-branca exigir uma superfície de render ainda não exposta,
registrar essa lacuna antes de ampliar a ponte.

#### Resultado final da Etapa 5 visual — runtime de 30/07/2026

O incremento visual e a cutscene da máscara passaram no executável autorizado.
O problema que deixava o save já com a máscara Goron aplicada era estado
residual: o host oferecia a intenção, mas não publicava `save.loaded` quando o
`Player` já estava vivo. A correção ficou dividida assim:

- o host registra `save.events`, guarda o slot em `OnLoadGame` e publica
  `save.loaded` no primeiro `OnGameFrameUpdate` com `PlayState` e `Player`
  válidos;
- o mod 0.1.41 remove a máscara residual no frame seguinte ao `save.loaded` e
  também chama `set_mask("none")` imediatamente antes de vestir a Goron;
- shutdown/unload libera `gMaskForced` mesmo se a fase da transição já tiver
  terminado;
- a máscara real `gGoronMaskDL` é desenhada primeiro na mão esquerda e depois
  no membro `HEAD`;
- a linha de tempo dedicada `CutsceneStyle::Mask` executa aproximação,
  sway/roll, close-up, pose de sofrimento, SFX, rampa azul-branca, squash,
  flash, troca do corpo e reveal; cleanup normal e cancelamento devolvem
  câmera, input, ambiente e luz pontual.

Build Release concluída sem erro ou aviso:

- executável:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\soh.exe`;
- SHA-256:
  `9015F191092549CAFF542DD39611E2D821A532699B59756383398AAAEBC1AC4B`;
- pacote validado:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\packages\goron-form-0.1.41.shipmod`;
- SHA-256 do pacote instalado e arquivado:
  `F6826EE9F8A4B38B8532111EAED7A55183EF8911DA918945A12D43A6045376BE`;
- regressão:
  `py -3 -m pytest tests\regression\test_goron_form_cutscene.py -q`
  → `5 passed`.

O log anterior foi arquivado antes de abrir o runtime e a sessão recebeu
manifest próprio:

- pre-log:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\Ship of Harkinian.OOT-GORON-003-etapa5-mask-demo4-final-20260730-0145.pre.log`;
- manifest:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\runtime-session-OOT-GORON-003-etapa5-mask-demo4-final-20260730-0145.json`;
- log vivo:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\Ship of Harkinian.log`.

Evidência da primeira passagem:

- `01:43:43.753`: máscara residual removida após carregar o save; o baseline
  visual mostra Link humano sem máscara;
- `01:44:25.627`: cutscene iniciada;
- `01:44:26.025`: `gGoronMaskDL` na mão esquerda, frame 8;
- `01:44:26.225`: `gGoronMaskDL` no `HEAD`, frame 12;
- `01:44:30.130`: corpo Goron trocado sob o pico branco;
- `01:44:30.775`: câmera, input, luz e ambiente restaurados.

A entrada foi repetida depois de uma retirada completa. Na segunda passagem:

- `01:50:23.579`: cutscene iniciada;
- `01:50:23.976`: máscara na mão;
- `01:50:24.176`: máscara na cabeça;
- `01:50:28.077`: troca do corpo;
- `01:50:28.726`: cleanup concluído.

Não houve reinício espontâneo nem loop: o log contém exatamente duas entradas,
duas passagens mão→cabeça e duas transformações solicitadas. O processo final
permaneceu responsivo. O retorno do input também foi exercitado depois do
cleanup com um hold real de `W`: às `01:56:15.976` a máquina entrou em
`mmAction=walk` com `linear=1,86`, continuou recebendo movimento durante o hold
e voltou a `mmAction=idle` às `01:56:16.626` após soltar. O jogo foi deixado
aberto, já na forma Goron, para o teste manual do usuário (PID `38232`).

Capturas:

- baseline, rampa azul, inclinação, flash, troca e retorno:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\captures\OOT-GORON-003-etapa5-mask-demo4-final-20260730-0145`;
- entrada densa com máscara visível e câmera em movimento:
  `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright\x64\Release\logs\captures\OOT-GORON-003-etapa5-mask-entry-final-20260730-0149`.

## O padrão de posse, estabelecido no incremento 4

As ações que ainda possuem fallback antigo têm uma **chave de posse** que
decide qual implementação vive. As duas implementações nunca podem valer ao
mesmo tempo — seriam duas físicas escrevendo no mesmo Player, ou dois combos
armando o mesmo quad, no mesmo frame.

| ação | chave | condição |
|---|---|---|
| bola | `MmFormOwnsRoll()` | corpo externo ativo **e** `set_roll_mode("chain")` |
| soco | `MmFormOwnsPunch()` | corpo externo ativo **e** a spec declara `punch_a` |

Além da posse durável, existe um único **gate de entrada por frame**:
`SetEntryBlocked`. A posse não pode oscilar — isso reativaria um caminho antigo
no meio de uma ação —, enquanto o gate só impede novas entradas quando uma ação
legada do host, água/void ou transição de máscara já possui o corpo. Ações em
curso fazem sua própria liberação; defesa e instrumento já vivem na máquina.

**Regra que vale para toda migração seguinte:** `ReleaseBody(player)` antes de
`Reset()`. `Reset()` descarta o estado sem devolver nada ao Player — raio do
cylinder, quad armado, input travado, gravidade do baque e sombra encolhida
ficariam pendurados no Link para sempre, e nada mais roda para corrigir.

## Incremento 5 — socos na máquina (feito, ligado)

`MmForm_StartPunch` (`:4312`), `MmForm_Action_Punch` (`:4426`),
`MmForm_GoronAction_PunchEnd` (`:4670`) e `sGoronPunchFrames` (`:4209`). A
lógica já existia no host; o que mudou é o **dono** — ela passa a viver junto das
outras ações em vez de espalhada pelo update do corpo.

- Tabela de janelas `{6,8} {12,18} {8,14}`, `earlyStart = 5.0f`, dano 2 com
  `DMG_HAMMER_SWING`, geometria do quad por passo, escolha `end`/`endR` por
  lock-on, encadeamento por borda de B (durante o golpe **e** durante a
  recuperação, com o passo voltando ciclicamente para A depois de C).
- **Serviço novo**: `animationLastFrame(nome)`. O combo compara
  `curFrame >= endFrame` como a referência; `animationFinished` sozinho não serve
  porque chega com um frame de atraso e o encadeamento é sensível a esse frame.
- **Root motion**: quem aplica continua sendo o host — `SkelAnime_UpdateTranslation`
  precisa do SkelAnime do corpo, que é dele. O que mudou é quem **decide**:
  `UsesRootMotion()` responde pelo perfil fechado (os três golpes, nunca a
  recuperação) no lugar da detecção por nome de animação.
- **Não portado**: o recuo de parede (`MmForm_CheckWallHit`), que interrompe o
  golpe ao acertar geometria. Depende de raycast próprio e de uma exceção de
  dyna com o mesmo cuidado do quique da bola.
- `WantsPlayerActionPaused()` passou a cobrir os socos também.

## Incremento 4 — bola ligada (feito)

Os cinco itens fecharam juntos:

1. `PLAYER_STATE3_PAUSE_ACTION_FUNC` reafirmado por
   `WantsPlayerActionPaused()`.
2. `gCustomBodyLastAnimFinished` guarda o retorno de `LinkAnimation_Update` para
   o frame seguinte. A atribuição fica **logo após a chamada**, não no fim da
   função: os blocos de soco e instrumento saem por `return`, e no fim o sinal
   se perderia justamente nos frames em que uma ação termina.
3. Desenho, payload do `body_anim_select` e getter Lua leem por `IsBallBody()`,
   `RollSpikesActive()`, `RollChargeLevel()` e `RollPhaseName()` — mesmo
   vocabulário de antes, para o mod não notar a troca.
4. Caminho antigo desligado: `nativeRolling`, `CustomBodyUpdateGroundPound`,
   `CustomBodyUpdateRollSpikes`, `CustomBodyUpdateRollHitbox`,
   `CustomBodyHandleRollWallBounce` e os três hooks (`gRollStartHook`,
   `gRollChainHook`, `gRollSteerHook`). O `set_roll_mode("chain")` continua
   funcionando **sem corpo externo** — a capability pública não pode deixar de
   existir só porque a forma existe.
5. Void da água: se ele entra com a bola rolando, a máquina larga tudo via
   `ReleaseBody` e o void fica dono único. Sem isso os dois escreveriam
   `velocity.y` no mesmo frame.

Dois campos do payload passaram a devolver zero/false com a máquina no comando,
em vez de mentir um valor do caminho antigo: `roll_bounce_active`/`_frames` (a
máquina quica dentro da própria física, sem janela observável) e
`punch_combo_queued` (a borda de B é consumida no mesmo frame).

## Incremento 3 — bola do MM (feito, desligada)

`MmForm_Action_GoronRoll` (`mm_player_form.cpp:6839`, ~750 linhas com os
helpers) está em `soh/soh/mmform/MmGoronForm.cpp`. `soh.exe` Release compila e
linka.

### Por que ela nasce desligada

`MmForm::Update()` já roda **todo frame** desde o incremento 2. Portar a bola
sem chave a colocaria em jogo no mesmo instante: o A entraria no curl, a máquina
escreveria velocidade e yaw, e ninguém pausaria a action func do Player nem
trocaria o desenho — com o rolamento antigo ainda no ar disputando o mesmo
corpo. Daí `SetBallEnabled(bool)`, que começa `false`. Enquanto estiver false,
`TryStartRoll` ignora o A e nada mais muda.

### O que entrou

- **`SetRollAttack` / `ClearRollAttack`** (`:6784`, `:6810`, de
  `Player_SetCylinderForAttack`, MM `z_player.c:2901-2927`). A regra é **por
  faixa de raio, não por ação**: acima de 30 o golpe é grande (pound usa 60) →
  sobreposição desligada e o Goron imune durante o baque; abaixo é rolamento
  normal (25). `ClearRollAttack` zera `dmgFlags` além de `atFlags`, senão o
  `DMG_HAMMER_SWING` fica cacheado no cylinder e qualquer rearme posterior bate
  com dano de martelo.
- **`ArmRollAttack`** — acréscimo nosso, não da referência: chama
  `Collider_UpdateCylinder` antes do `CollisionCheck_SetAT`. O ator hospedeiro
  atualiza *depois* do Player, então o `Collider_UpdateCylinder` do frame
  (`z_player.c:12163`) já passou com raio e posição antigos. Sem refrescar, um
  raio 60 de ground pound entraria na checagem centrado onde o Goron estava
  antes de cair. `CustomBodyUpdateRollHitbox` no host já faz isso pelo mesmo
  motivo.
- **Perfis de gravidade** `Normal` (-1.2) / `RollApex` (-0.2) / `RollSlam`
  (-10.0).
- **Alvo de yaw e velocidade**: `rel.stick` + `Camera_GetInputDirYaw`,
  `speedTarget = (stickMag / 60) * 8.0 * 2.6`.
- **Entrada** (`:12255-12294`) e **saída** (`:6847-6879`) da esfera, incluindo a
  cópia de `prevPos` sobre `world.pos` que evita o pop visual ao desenrolar.
- **Ground pound** inteiro: subida com gravidade segurada no ápice,
  `rollGroundPoundTimer = 10`, apex/slam, quake, shockwave branco, anel de
  poeira, `actorCtx.unk_02 = 4` e a janela de dano de um frame.
- **Espinhos e carga**: as duas fases, custo inicial de 2 de magia, dreno de 1 a
  cada 10 frames, cancelamento por ladeira íngreme (`0x3A98`), por A solto e por
  magia zerada.
- **Física do rolamento**: trajetória (`rollHomeYaw`) separada da direção
  apontada (`player->yaw`), derrapagem lateral com decaimento, gravidade de
  ladeira, aceleração por giro (0.08 em gelo/areia/terra), curva amortecida em
  baixa velocidade, quique em parede com a exceção de dyna, tombamento por
  sondagem de terreno, poeira por superfície e giro/deformação da esfera.

### Diferenças deliberadas em relação à referência, e o porquê

| referência | aqui |
|---|---|
| `sFormProps[form].cylinderRadius` | raio salvo/restaurado — não temos a tabela |
| `MmSfx_*` com loops contínuos e `Stop` | só disparo único; `StopRollSfx()` fica **vazia e documentada**. Os sons de rolar já são gatilho por cruzamento de zero nos dois modos, então não perdem nada; o zumbido contínuo da carga é o único som que não sai |
| ids `MM_NA_SE_*` | o 3º argumento de `MmForm_PlaySfx(player, mmId, ootId)` da própria referência **já declara** o equivalente OoT de cada som — são esses os ids usados, não uma escolha nova |
| `bgCheckFlags \|= 0x800` | **não portado** — é bit do MM; o OoT não define nada acima de `(1 << 9)` em `bgCheckFlags` |
| reafirma `shadowScale` todo frame | uma vez na entrada: `z_player.c` não escreve `shape.shadowScale` em ponto nenhum (zero ocorrências no arquivo) |
| crack no chão (`ACTOR_EN_TEST`) | fora de escopo: é desenho por frame, e a máquina não tem superfície de desenho |

Um achado que vale registrar: o bloco `if (actionTimer == 0)` na fase POUND da
referência (`:7005`) é **inalcançável** — o dispatcher incrementa `actionTimer`
antes de despachar (`:11589`, e o comentário em `:3450` diz isso). A janela de
dano do baque é o frame único armado na aterrissagem, no ramo de subida. Nosso
dispatcher tem a mesma ordem, então o comportamento é o mesmo; o código registra
o achado em vez de copiar código morto.

### Superfície nova do módulo

`WantsPlayerActionPaused()`, `IsRolling()`, `IsBallBody()`,
`RollSpikesActive()`, `RollPhaseName()`, `SetBallEnabled()`/`BallEnabled()`.
`ReleaseRollCollider()` deixou de devolver só o collider: agora devolve também
input, gravidade, sombra e travas de rotação, para um desligamento fora do fluxo
normal (troca de forma, troca de cena, mod desativado no meio do rolamento) não
deixar o Link sem controle ou com a gravidade de baque.

## O que falta migrar

Defesa e instrumento foram migrados e testados na Etapa 3. O gate de entrada
foi consolidado em `SetEntryBlocked`.

`Land` e `Damage` foram migrados e testados na Etapa 4. A máquina agora possui a
pose e a recuperação das duas ações; o OoT permanece dono do dano e da física de
knockback. O Lua não consome mais `payload.landing` nem dirige a animação
`damage`; o hook de `health_change` ficou somente com olhos assustados e timer.

`rollSquash`, `rollColorLerp`, `rollDriftYaw` e o pulso do bounce já são
consumidos no draw desde o primeiro incremento visual da Etapa 5. Eles continuam
read-only fora da máquina: o desenho altera somente a malha, nunca a física,
`actor.shape.rot`, collider ou `Player_Action`.

## Incremento 2 — máquina ligada ao host (feito)

Host SHA-256 `5F8FC4211BB401A15BB71FC79E4A8D23F0631E849AA09D05F8DE85B5B9826464`.
Mod inalterado (0.1.39) e **ainda desativado** — sem ele não há corpo externo e a
máquina não tem o que dirigir. Para testar:
`Rename-Item goron-form.shipmod.disabled goron-form.shipmod`.

- Ponte `MmForm*` em `ShipLuaBootstrap.cpp`: os quatro serviços resolvem nome de
  animação contra a spec do mod, e `playMmSample` vai para
  `ShipLua::MmAudio_PlaySampleOneShot` — o motor de amostras do MM que **já
  existe e funciona** (`soh/soh/mmaudio/MmSfxPlayer.h`).
- `MmForm::Update()` roda todo frame, depois do dispatcher das ações atuais.
- `MmForm::DesiredAnimation()` substituiu o limiar de velocidade solto que havia
  no fallback de seleção. Mesma regra de idle/walk/run, agora dentro de uma ação
  com dono, com cessão explícita ao OoT e transições de ar antes do dispatch.
- `gCustomBodyPlay` publica o `PlayState` do frame para a ponte.

### Correção de semântica feita durante o incremento

O retorno de `Update()` chegou a ser documentado como "reafirme
`PLAYER_STATE3_PAUSE_ACTION_FUNC`". **Errado**: na locomoção é a action func do
Player que move o Link — pausá-la o deixa parado. A referência só pausa nas
ações que tomam o movimento (soco, defesa, bola); idle/walk/run deixam o OoT
rodando 1:1. O retorno agora significa apenas "a forma está dirigindo a pose", e
quem pausa é cada ação, no incremento em que ela entrar.

### Risco a observar no teste

A troca do fallback é o único ponto que pode regredir: se um mod não declarar
`idle`/`walk`/`run`, agora cai em `default_anim` em vez de tentar a chave por
limiar. Para o `goron-form` não muda nada — ele declara as três.

## Incremento 1 — estado e dispatcher (feito)

`soh/soh/mmform/MmGoronForm.{h,cpp}`. Compila e linka; o glob recursivo de
`soh/CMakeLists.txt:139` pega o diretório novo, mas **exige reconfigurar o
CMake** (`cmake -S . -B build/x64`) antes do primeiro build.

- Enum `GoronAction` com o subconjunto Goron das ações da referência.
- `Services`: ponteiros de função que o host empresta. A máquina pede animação
  **pelo nome** e não conhece Lua, spec, nem caminho de asset — mesma fronteira
  do resto do projeto, e o que deixa Zora/Deku reusarem a máquina depois.
- Cessão ao OoT (`GoronAction::OotAction`) para cutscene, item, diálogo,
  escada, borda, morte e input travado. A ocarina fica **de fora** dessa lista
  de propósito: é ação do Player que precisa continuar rodando.
- Detecção ar/chão antes do dispatch, como `MmForm_UpdateActive`.
- Locomoção Idle/Walk/Run por limiar (0.5 / 4.0).
- `Update()` devolve se a forma é dona do corpo no frame — o chamador usa isso
  para decidir reafirmar `PLAYER_STATE3_PAUSE_ACTION_FUNC`.

**Ainda não está ligado** ao update do corpo. O caminho antigo continua no ar e
o comportamento em jogo é idêntico ao de 0.1.39. Ligar é o fim do incremento 2.

## A decisão que originou esta tarefa

O `goron-form` (mod Lua sobre `set_body`) foi **desativado, não removido** —
`goron-form.shipmod.disabled` nas duas pastas de mods; fonte intacto em
`examples/goron-form/`. Ele serve de comparação até a nova forma passar dele.

O usuário pediu a abordagem do OoTMM/ComboShip. A pesquisa mostrou que nenhum
dos dois transforma o Link dentro do OoT (ver `CREDITS.md`), e que a única
implementação de referência é o **skijer / Not-Enough-Items**. As fontes estão
copiadas em `research/skijer-transformation-masks/` com o porquê de cada uma.

## O achado que torna o port tratável

`mm_player_form.cpp` usa `Player*` do **OoT** em 165 lugares e `MmPlayer` em 20.
Não é código do MM colado verbatim: é a lógica do MM reescrita contra as structs
do OoT. **A forma Goron não precisa de `mm_compat.h` nem de
`mm_player_struct.h`** — esses servem a outros arquivos do fork dele.

Consequência prática: o port é adaptar funções que já falam a linguagem do nosso
host, trocando a infraestrutura dele pela nossa.

## Substituições de infraestrutura (dele → nossa)

| skijer | Link-Span |
|---|---|
| `MmAnim_Load(MM_ANIM_PG_*)` | `ResourceMgr_LoadAnimByName("__OTR__mm/objects/gameplay_keep/gPlayerAnim_pg_*")` |
| `mm_asset_loader.h` | caminhos declarados pela spec do mod (`anims`/`models`) |
| `MmSfx_*` / sintetizador próprio | `ShipLua::MmAudio_PlaySampleOneShot("mm/audio/samples/…")` — **já existe e funciona** em `soh/soh/mmaudio/MmSfxPlayer.h`; visto no log tocando amostra de soundfont |
| `gFormState.formSkelAnime` | `gCustomBodySkelAnime` (já dimensionado para os 25 limbs do Goron) |
| `extended_inventory` / `custom_items` | fora de escopo; não portar |
| `PikachuForm`, `GaroForm`, `GerudoForm` | fora de escopo; não portar |

## Ordem de port, com origem

Linhas referem-se a `research/skijer-transformation-masks/mm_player_form.cpp`.

1. **Estado + dispatcher** — enum de ações em `:660-730`, `gFormState` em
   `:690-1110`, `MmForm_UpdateActive` em `:10999`. Portar só o subconjunto
   Goron.
2. **Locomoção** — `GORON_ACT_IDLE/WALK/RUN`. Substitui nosso fallback por
   limiar de velocidade.
3. **Rolamento — a maior divergência. PORTADO, desligado por chave.**
   `GORON_ACT_ROLL_INIT`, `GORON_ACT_GORON_ROLL` (o `Player_Action_96` do MM,
   com física própria), `GORON_ACT_GORON_ROLL_JUMP`,
   `GORON_ACT_GORON_ROLL_POUND`, `GORON_ACT_ROLL_UNCURL`. Em jogo ainda vale o
   caminho antigo — sequestro do `Player_Action_Roll` do OoT, que é um
   rolamento de esquiva, encadeado para fingir a bola. Trocar um pelo outro é o
   incremento 4.
4. **Socos — MIGRADOS para a máquina e ligados.** `MmForm_StartPunch` `:4312`,
   `MmForm_Action_Punch` `:4426`, `PunchEnd` `:4670`. Tabelas
   `sGoronPunchFrames` `:4209`. Fora: o recuo de parede
   (`MmForm_CheckWallHit`).
5. **Instrumento** — `MmForm_EnterGakki` `:10894`, `MmForm_UpdateGakki`
   `:10912`, `MmForm_ExitGakki` `:10974`, keyframes `:1211-1226`. **Já
   portamos** a máquina de 3 fases e as três tabelas de escala.
6. **Defesa** — `MmForm_EnterShield` `:3585`,
   `MmForm_ActivateFormShieldQuad` `:3481`, desenho do esqueleto de 4 limbs
   `:15698`. **Já portamos**.

## Armadilhas já pagas nesta árvore — não repetir

Do `claims/OOT-GORON-001.md`, todas custaram um ciclo:

1. `ACTOR_FLAG_UPDATE_DURING_OCARINA` no ator hospedeiro. Sem ela
   `Actor_UpdateAll` **para de atualizar** o ator assim que a ocarina sai
   (`z_actor.c:2624`) — o draw continua e mascara o sintoma.
2. A raiz tem de ser neutralizada em **todo frame de toda animação**, não só nas
   de soco, mas a ordem é obrigatória: `LinkAnimation_Update` apenas enfileira
   `LOADFRAME`. Chamar `SkelAnime_UpdateTranslation` diretamente logo depois lê
   a pose anterior e o `LOADFRAME` subsequente desfaz a neutralização. Enfileire
   `AnimationContext_SetMoveActor` depois dele; use `ANIM_FLAG_NOMOVE` fora do
   perfil autorizado e `ANIM_FLAG_UPDATEY` para devolver Y a `baseTransl`.
   `SkelAnime_InitLink` não define essa base: para o skeleton Player ela precisa
   ser `(-57,3377,0)`.
3. `ANIM_FLAG_NOMOVE` em **toda** troca de animação, senão a nova herda o
   `prevTransl` da anterior.
4. `PLAYER_STATE3_PAUSE_ACTION_FUNC` é limpo todo frame
   (`z_player.c:12193`): reafirmar sempre, e **nunca** durante a ocarina — ela
   é uma ação do Player que precisa rodar.
5. `gSPDisplayList` só depois de `ResourceMgr_OTRSigCheck` aprovar. Emitir com
   assinatura reprovada entrega lixo ao renderizador e trava a tela.
6. Nenhuma saída de erro silenciosa em caminho de estado. Foi a falta de log em
   `CustomBodyStartAnimation` que escondeu a causa real por três ciclos.
7. Capability nova: schema → regen → copiar para `MODSDK-005` → commit →
   apontar `extern/ship-lua` → só então compilar. Pular a etapa 1 faz o runtime
   **rejeitar todos os mods**.

## Validação

`MSBuild build\x64\soh\soh.vcxproj /p:Configuration=Release` — compila e linka
`x64/Release/soh.exe` sem erro nem aviso (o projeto trata aviso como erro; o
único que apareceu em todo o trabalho, `C4244` na conversão de `rel.stick_*`
para `f32` no `Math_Atan2S`, foi corrigido com cast explícito).

Runtime da Etapa 1 concluído em 28/07/2026. Conversão do roll,
curl/bola/desenrolar, combo A/B/C, root motion, ground pound, espinhos,
água/void, ocarina, defesa migrada e instrumento migrado estão comprovados.
O reteste de água/respawn confirmou a correção dos olhos. A Etapa 3 está
concluída. `Land` e `Damage` passaram em 29/07/2026, incluindo dano no chão,
knockback aéreo, aterrissagem, recuperação do controle e regressões de R, roll e
ocarina. A Etapa 4 está concluída; permanecem a dívida visual e o áudio próprio
dos tambores na Etapa 5. Em 30/07/2026, squash/tilt/kick/azul da bola e a
cutscene completa da máscara passaram em duas execuções consecutivas, com
cleanup e retorno ao idle.

## Próxima ação

**Testar em jogo o build de 01/08/2026 17:06**
(SHA-256 `7BCEBA8258B864426769A1262510BD9EF0F3F4D68F3D112D383D504ACFD88D7C`).

Ele acumula as treze correções da auditoria contra o decomp mais a cutscene, e
**nenhuma delas rodou um frame**. O `goron-form.shipmod` está ativo em
`x64/Release/mods` e o `mm.o2r` está presente.

O build tem diagnóstico de impacto: toda vez que a bola encosta em parede sai uma
linha `ShipLua bola/parede:` com velocidade, ângulo em graus, flag de interação e
qual ramo executou (FRONTAL / RASPÃO / NENHUM). Se o knockback ainda parecer
errado, é essa linha que diz onde olhar — foram três ciclos perdidos deduzindo o
sintoma em vez de medir.

Ordem de risco: knockback de parede, transformação (contorção e boca da máscara),
retirada da máscara, e por fim curvar/girar, que mudaram de fórmula.

Depois do teste, as dívidas restantes:

1. áudio dos tambores — ver o diagnóstico corrigido na seção da auditoria; NÃO é
   port de samples, é seleção de instrumento no canal 5 do `MmSeq`;
2. recuo de parede do soco (`MmForm_CheckWallHit`) — conferir no decomp, não na
   `research/`;
3. crack decal do ground pound, só se o escopo de desenho por frame for reaberto.

**A observação anterior de que squash/tilt/kick "já têm evidência de runtime" era
falsa** — o kick de parede era invenção e foi removido nesta auditoria. Não trate
nenhuma parte deste port como validada sem log ou teste que a comprove.

### Não commitado — risco real

Nada desta tarefa foi commitado. `soh/soh/mmform/` sequer é rastreado pelo git, e
há mudanças preexistentes de outras tarefas nas duas worktrees que precisam ser
separadas antes de qualquer commit.

Ground pound/stomp, espinhos, água/void, ocarina e defesa não precisam ser
repetidos como casos desconhecidos: já têm validação positiva. Faça regressões
curtas apenas depois de mudanças nos respectivos caminhos. O save ativo está no
SHA-256
`31CCB6D12374780EF8F02A66B989661B48D6D9F253961C820C4905FD2F81FFEB`.

Nota: `mods/survival-demo.shipmodss` tem extensão inválida — o loader exige
`.shipmod`, então não carrega. Se um mod de survival aparecer em jogo mesmo
assim, é bug de filtro do loader e vira investigação própria.
