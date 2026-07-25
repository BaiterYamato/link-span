# Handoff — OOT-GORON-002: portar a transformação de MM para o OoT, direito

## Por que este documento existe

A forma Goron no OoT chegou a um ponto em que o **corpo, as animações e a
câmera funcionam**, mas a transformação ainda não é a de Majora's Mask: falta
o efeito visual, o áudio e a voz. As tentativas anteriores foram incrementais
e produziram meias-portas (som substituto do OoT no lugar do som real, voz
silenciada em vez de trocada). O usuário pediu, com razão, que se pare de
remendar e se faça o port direito de uma vez.

Este documento é o estado real + tudo que já foi descoberto, para o próximo
agente não repetir a investigação nem cair nas mesmas armadilhas.

**Regra desta tarefa:** não entregar substituto e chamar de port. Se um asset
ou sfx do MM não puder ser alcançado, isso é uma descoberta a registrar, não
um convite para trocar por algo parecido do OoT sem avisar.

---

## Estado atual (verificado em jogo pelo usuário)

| Peça | Estado | Onde |
|---|---|---|
| Corpo Goron (skeleton 25 limbs, cross-world do mm.o2r) | ✅ funciona | `set_body` genérico |
| 31 animações `pg_*` do MM | ✅ carregam | `goron-form/main.lua` |
| Animação de colocar máscara (`cl_setmask`) | ✅ toca | `play_mask_on_animation` |
| Flash branco na troca | ✅ funciona | `MaskTransitionUpdate` |
| Câmera de cutscene | ✅ ativa, mas **"esquisita"** | `ship.oot.cutscene.*` |
| Efeito visual/partículas da transformação | ❌ **nunca implementado** | — |
| Áudio da transformação | ❌ **nunca implementado** | — |
| Voz (Goron rolando soa como Link criança) | ❌ **nunca implementado** | — |

Tudo isso é OoT-only. O host MM não tem nada disso.

---

## Descobertas que custaram caro (não redescobrir)

### 1. `Play_CameraSetAtEye` não serve para câmera de cutscene
Ela termina gravando `camera->atLERPStepScale = 0.01f` — interpolação
lentíssima. A câmera *é* comandada, mas se arrasta tão devagar que parece que
nada acontece. **Escreva `cam->at`, `cam->eye`, `cam->eyeNext` direto na
struct todo frame**, como `z_boss_dodongo.c` faz. Já corrigido.

### 2. Congelar o jogador atropela a animação
`Player_SetCsActionWithHaltedActors(play, actor, 1)` substitui a ação do Link
e **apaga a animação de máscara** que o mod acabou de tocar. Por isso a
primitiva de cutscene é só-câmera por padrão (`freeze_player` é opt-in).

### 3. Hook de cutscene não pode viver no update do Player
Com atores congelados o `OnPlayerUpdate` não dispara. O avanço da câmera
está em `OnGameFrameUpdate`.

### 4. `OnPlayerSfx` é um hook MORTO
Está declarado em `GameInteractor_HookTable.h:30` mas **não tem nenhum call
site** em `z_player.c`. Não adianta fazer bridge dele.

### 5. Áudio NÃO é cross-world
`mm.o2r` dá modelos, texturas e animações. **Sfx não**: vivem nos bancos de
áudio/soundfont, que são outro sistema e não estão montados. Antes de
prometer "som do MM", *verifique* se há caminho — se não houver, a saída
honesta é dizer que não há, não trocar por um som do OoT em silêncio.

### 6. `player.get`/HUD têm teto de display list
`draw_rect` e afins têm orçamento de 400 retângulos/frame. Estourar o buffer
dá `Fault` em `graph.c:364`. Efeitos de partícula devem usar o caminho de
desenho do engine, não centenas de retângulos.

---

## Pontos de entrada já mapeados

| Necessidade | Símbolo | Arquivo |
|---|---|---|
| Voz do jogador (funil único) | `Player_PlayVoiceSfx(Player*, u16)` | `z_player.c` — soma `ageProperties->unk_92` |
| Tocar sfx posicional | `Player_PlaySfx(Actor*, u16)` | `functions.h:476` |
| Tocar sfx central | `Sfx_PlaySfxCentered(u16)` | `functions.h:982` |
| Sfx geral | `Audio_PlaySoundGeneral(...)` | `functions.h:2138` |
| Subcâmera | `Play_CreateSubCamera` / `Play_ChangeCameraStatus` / `Play_GetCamera` | `z_play.c` |
| Cutscene mode | `func_80064520` / `func_80064534` | `functions.h:864-865` |
| Referência de câmera que funciona | `z_boss_dodongo.c:453-470` e `:594-607` | — |
| Referência de cutscene curta | `z_onepointdemo.c:1216-1251` | — |

### Referências externas já catalogadas
`docs/wiki/Research-External-Item-Systems.md` §9 tem a sequência do OoTMM
(`Player_ToggleForm` → sequestra `actor.update` → `Player_UpdateForm`).
**Limite conhecido:** o corpo de `Player_UpdateForm` está num overlay e não
foi acessível via raw do GitHub — número de frames, id do SFX e a natureza do
efeito visual **não foram verificados**. Quem for portar precisa obter isso de
outra forma (clonar o repo localmente, ou ler o decomp de MM).

Planilhas úteis (da doc oficial do 2S2H, catalogadas em §8):
- **Link Voice Samples** — catálogo de amostras de voz por forma do Link.
  É a fonte mais provável dos ids de voz Goron.
- **2S2H Assets Guide** — mapeamento decomp ↔ 2S2H.

---

## Plano de execução

### Fase 1 — Descobrir de verdade o que MM faz (não pular)
1. Clonar OoTMM localmente (`git clone`) e ler `Player_UpdateForm` de fato —
   é o único jeito de saber frames, SFX e efeito. Sem isso, tudo abaixo vira
   chute.
2. No decomp de MM (ou no 2S2H em `MM-MODSDK-001`), localizar:
   - a função que desenha o efeito de transformação (partículas/flash);
   - o(s) sfx id(s) da transformação;
   - os ids de voz da forma Goron.
3. **Decidir e registrar** se os sfx do MM são alcançáveis a partir do OoT.
   Se não forem, escrever isso na wiki e propor a alternativa explicitamente
   ao usuário — não escolher sozinho um som do OoT.

### Fase 2 — Primitivas de host que faltam
Todas genéricas, no padrão do resto do projeto (nada com "goron" no nome):
1. `ship.oot.audio.play_sfx(id, centered?)` — tocar sfx.
2. `ship.oot.audio.set_voice_override(id|nil)` — silenciar/trocar a voz.
   Exige instrumentar `Player_PlayVoiceSfx` com um `ShipLua_TransformVoiceSfx(u16*)`
   opt-in (mesmo padrão de `ShipLua_ShouldBlockRoll`).
   *(Esta fase chegou a ser escrita e foi revertida por não estar testada —
   ver commit `77ff9bae1` como ponto de partida limpo.)*
3. Primitiva de **efeito de partícula**, se a Fase 1 mostrar que é preciso.
   Provavelmente `EffectSs_Spawn*` — investigar `z_effect_soft_sprite*`.

### Fase 3 — Ajustar a câmera
A cutscene ativa mas o usuário achou "esquisita". Precisa de iteração visual:
`start_distance`, `end_distance`, `height`, `spin` são todos ajustáveis em
`goron-form/main.lua` **sem recompilar**. Comparar com a filmagem de MM.

### Fase 4 — Montar a transformação completa no mod
Só depois de 1-3: sequência mask-on → efeito → som → troca de corpo → câmera,
com os valores reais descobertos na Fase 1.

### Fase 5 — Paridade no host MM
Nada disto existe no `2ship.exe`. Replicar as primitivas lá.

---

## Como validar

O padrão desta sessão (build limpo + mod carrega + N segundos estável)
**não pega** os bugs que importam aqui — invisibilidade, câmera lenta,
travamento e crash só apareceram quando o usuário jogou. Toda entrega desta
tarefa precisa de confirmação visual em jogo antes de ser dada por pronta.

Comandos:
```
cmake --build build/x64 --config Release --target soh -- /m:1
py -3 tools/shipmod.py validate examples/goron-form
```
Pacotes de teste: `link-span/x64/packages/{oot,dual}/Release/`.

**Armadilha de deploy:** o `.shipmod` do pacote `dual` já ficou travado por
handle do Windows por horas, fazendo o usuário testar um mod velho e reportar
bugs inexistentes. **Sempre confirme o conteúdo implantado**, não só a cópia:
```
unzip -p <pkg>/mods/goron-form.shipmod main.lua | grep -c cutscene
```
