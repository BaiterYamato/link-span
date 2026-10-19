# Contratos dos serviços genéricos de corpo e controle do Player

Proposta de ABI C, de 19/10/2026. **Não implementada.** Escopo: as cinco lacunas solicitadas; não é uma declaração de que toda a extração do conteúdo MM esteja concluída.

## 1. Base conferida e decisões

Entradas lidas:

- [inventario-jogador.md](inventario-jogador.md), seções 2, 3 e 4, inclusive ordem de execução.
- Host `D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001`, HEAD `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`, sem alterações rastreadas na consulta. Referências abaixo são relativas a esse host.
- Core `D:/Desenvolvimento/ship-lua-worktrees/OOT-RELEASE-002-core`, HEAD `053265040e9ddf62da66473f1c6713221d44b2bf`: RFCs 0012, 0018 e 0021, `docs/native-providers.md`, ABI e lifecycle de `NativeProvider`.

| Lacuna | Contrato proposto | Motivo |
| --- | --- | --- |
| Corpo do Player | **Novo** `linkspan.oot.player-body` V1, `oot_player_body.h` | Não existe serviço de corpo; anexá-lo a skeletons confundiria a instância visual com a posse do Player. |
| Animação Link | `linkspan.oot.skeletons` **V2** | Mesmo recurso e lifetime, com formato tipado e callback de membro. |
| Ação | `linkspan.oot.movement` **V3** | Amplia primitivas de controle existentes; não exporta endereços de ações privadas. |
| Câmera | `linkspan.oot.camera` **V2** | Acrescenta roll, epoch e limpeza por dono à posse já existente. |
| Quads do Player | `linkspan.oot.colliders` **V2** | Posse de slots existentes, sem substituí-los por cilindros. |

O exe mantém o motor de corpo customizado, interpretação genérica de specs, recursos tipados, animação, render, arbitragem e cleanup. Máquina de estados, janelas de dano, dimensões, escolhas de assets, física da habilidade, áudio e coreografia ficam na DLL. Nenhum nome novo de serviço, campo ou função representa uma forma ou habilidade concreta.

## 2. Regras comuns da ABI

### 2.1 Versão e prefixos

Os blocos C usam os headers públicos atuais, `<stdint.h>` e `ship_native_abi.h`. São declarações de contrato, não implementações. Cada tabela nova é pedida por `runtime->get_service(context, nome, versão, sizeof(Tabela))`; o core compara a versão **exata** em `NativeProvider::GetService`. Publicar versões antigas e novas simultaneamente em `CreateOotNativeProviderPolicy`, não apenas trocar o macro VERSION.

Nos exemplos, `ShipOotSkeletonsV1 v1` e equivalentes estão no offset zero. Isso contém literalmente todos os campos antigos, incluindo `size`, e preserva o prefixo binário. O header definitivo pode repetir esses campos como as V2 atuais fazem. `v1.size` informa o tamanho da tabela completa quando esta é obtida como V2. Não adicionar `layout_id` antes das funções antigas: skeletons/camera/colliders V1 **não têm esse campo**. Nessas V2 ele entra depois do prefixo; movement já o contém na V2.

Validar offsets e assinaturas em C e C++, sem `#pragma pack`. Toda struct de entrada/saída começa com `uint32_t size`; o chamador inicializa `size`, zeros e campos reservados. Aceitar `size >= sizeof(versão conhecida)`, copiar apenas campos conhecidos e não escrever além do tamanho informado. Tabelas são imutáveis, pertencem ao host e podem ser usadas até o unload do consumidor; `layout_id` é string estática do host. Conferir o fingerprint antes de interpretar `void*` como tipos do jogo.

Modificar headers fingerprintados muda o layout do SDK mesmo mantendo prefixos. Recompilar as DLLs contra o SDK novo; compatibilidade de offsets não implica compatibilidade com o fingerprint anterior.

### 2.2 Status, threads e ponteiros

Usar somente os valores existentes de `ShipNativeStatus`:

| Status | Significado nas operações novas |
| --- | --- |
| `SHIP_NATIVE_OK` | Operação aceita; ausência consultável vem em booleano, não em valor mágico. |
| `SHIP_NATIVE_INVALID_ARGUMENT` | Struct curta, enum/flag inválido, NaN/Inf, ponteiro obrigatório nulo, token inventado ou chamada fora da thread do jogo. |
| `SHIP_NATIVE_UNSUPPORTED` | Sem gameplay/Player, token conhecido mas revogado/epoch antigo, fase inadequada ou formato não suportado. |
| `SHIP_NATIVE_LIMIT` | Outro dono, limite de instâncias ou tentativa de operação conflitante durante callback. |
| `SHIP_NATIVE_FAILURE` | Recurso ausente/inválido, erro de alocação, erro interno ou falha de callback. |

Todas as funções e callbacks aqui são da **thread do jogo**. Draw e pós-membro exigem também o escopo render de `oot.render`; não são callbacks da GPU. Não chamar de worker, áudio ou `DllMain`; não enfileirar silenciosamente uma chamada feita na thread errada. Pausa não avança o tick de gameplay nem expira pedidos por tempo de parede.

Strings de entrada, arrays e specs são emprestados só até o retorno; o host copia os caminhos e nomes antes de chamar APIs que os retêm. Em particular `LinkAnimation_Change` guarda o endereço do caminho OTR: uma `std::string` temporária não serve. Copiar para storage estável e manter o recurso vivo até terminar a fila de animação e os consumidores de render. Nenhum `Ship::Resource`, `shared_ptr`, container STL ou `lua_State*` cruza a ABI.

`PlayState*`, `Player*`, `Actor*`, matrizes e payloads emprestados só valem durante a chamada/callback. O host pode manter referências internas, sempre associadas à geração; a DLL não guarda esses ponteiros entre frames. Callbacks e `user` permanecem válidos até unregister/release concluído. A DLL mantém também vivos buffers de display list nativa até o renderer terminar seu consumo; o serviço não transforma um ponteiro de memória da DLL em recurso persistente.

### 2.3 Dono, geração e descarregamento

`owner` é o **id canônico do mod no manifesto**, não um apelido escolhido por recurso. Usar o mesmo id do `onProviderUnload`; copiar e limitar a 128 bytes UTF-8, sem controles e sem truncar. Lua obtém esse id do contexto real de execução do mod. Serviços compartilhados atuais não recebem `runtime->context` em cada chamada: não existe identidade autenticada implícita ao chamar um ponteiro da tabela. Nas APIs novas a DLL deve declarar honestamente seu id, como nos serviços atuais por dono. Isto é arbitragem entre código nativo confiado, não isolamento contra uma DLL que escreve memória ou falsifica ids.

Tokens `uint64_t` são opacos, não zero, nunca reciclados durante a vida do host e pertencem a um domínio de serviço. Registro interno: `{owner, token, scene_epoch, player_generation, resource_generation}`. Uma comparação de endereço de `PlayState` é insuficiente: a arena pode reutilizar endereços. `scene_epoch` monotônico muda em **todo** teardown/reload de gameplay, inclusive mesmo mapa e mesmo endereço; `player_generation` muda na destruição/substituição do Player. Não usar o `camera_epoch` de interpolação como epoch de cena.

Aquisição é atômica, sem preempção ou fila: primeiro dono aceito vence; o segundo recebe LIMIT e token zero. O mesmo dono também precisa usar/reliberar o token que já possui. Ordenação determinística de callbacks segue a carga dos mods; não criar um sistema novo de prioridades. Release válido é idempotente, inclusive para token conhecido revogado. Não restaurar estado em outro Player ou outra cena.

Integração obrigatória no host: ampliar `NativeProviderPolicy::onProviderUnload` (já existe e é chamado antes de shutdown). Marcar owner como indisponível; impedir reaquisição; invalidar callbacks; revogar quads/root motion; soltar ação; soltar corpo e câmera; destruir skeletons próprios. Depois permitir shutdown e unload da biblioteca. Nada depende da DLL executar cleanup corretamente. Rollback de init parcial usa a mesma rotina. Finalizadores Lua não podem rearmar o owner depois dessa marcação.

Callbacks ativos não podem disparar unload imediato do próprio provider. Release dentro do próprio callback cancela novas chamadas imediatamente, mas adia destruição de buffers até sair do callback e drenar filas. Destruição de instância com LOADFRAME/MOVEACTOR pendente é diferida até `AnimationContext_Update`; render já emitido mantém recursos presos até o fim do consumidor gráfico. Não chamar código da DLL no cleanup automático. Em `OnPlayDestroy` tardio, apenas descartar handles/ponteiros da arena já morta; jamais tentar liberar outra vez os buffers daquela arena. Preferir buffers próprios do host para o mecanismo novo.

## 3. Animação Link: skeletons V2

### 3.1 Declarações

```c
#include <stdint.h>
#include "oot_skeletons.h"
#include "oot_engine.h"
#include "oot_camera.h"
#include "oot_colliders.h"

#define LINKSPAN_OOT_SKELETONS_VERSION_2 2u
#define SHIP_OOT_SKELETON_STANDARD 0u
#define SHIP_OOT_SKELETON_LINK 1u
#define SHIP_OOT_ROOT_DISABLED 0u
#define SHIP_OOT_ROOT_XZ 1u
#define SHIP_OOT_ANIM_OVERRIDE 0u
#define SHIP_OOT_ANIM_FALLBACK 1u

typedef struct ShipOotSkeletonSpecV2 {
    uint32_t size;
    uint32_t format;
    const char* skeleton_path;
    const char* initial_animation_path;
    int16_t base_translation[3]; /* unidades da junta raiz; nunca hardcoded por forma */
    uint16_t reserved;
} ShipOotSkeletonSpecV2;

typedef struct ShipOotAnimationPlayV2 {
    uint32_t size;
    float speed; /* magnitude > 0; reverse escolhe o sinal */
    float morph_frames; /* >= 0 */
    uint8_t mode; /* LINKSPAN_OOT_ANIM_LOOP ou LINKSPAN_OOT_ANIM_ONCE */
    uint8_t reverse;
    uint8_t reserved[2];
} ShipOotAnimationPlayV2;

typedef struct ShipOotAnimationInfoV2 {
    uint32_t size;
    uint8_t present; /* declarado/encontrado */
    uint8_t loaded;  /* resolvido e validado como o formato da instância */
    uint8_t reserved[2];
    float asset_last_frame; /* limite do asset, independentemente de reverse */
} ShipOotAnimationInfoV2;

typedef struct ShipOotAnimationStateV2 {
    uint32_t size;
    uint64_t scene_epoch;
    uint64_t tick; /* último update desta instância */
    uint64_t animation_serial; /* muda a cada restart/troca */
    float current_frame;
    float asset_last_frame;
    float target_end_frame; /* 0 em reverse; não confundir com asset_last_frame */
    float speed; /* com sinal */
    uint8_t finished_previous_update;
    uint8_t reserved[3];
} ShipOotAnimationStateV2;

typedef struct ShipOotSkeletonLimbV2 {
    uint32_t size;
    uint64_t scene_epoch;
    uint64_t skeleton;
    void* play_state;
    void* actor; /* dono visual; NULL para draw genérico sem ator */
    int32_t limb; /* índice emitido pelo draw do jogo, não normalizado */
} ShipOotSkeletonLimbV2;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotSkeletonLimbFnV2)(
    void* user, const ShipOotSkeletonLimbV2* limb);

typedef struct ShipOotSkeletonsV2 {
    ShipOotSkeletonsV1 v1; /* offset 0, prefixo intocado */
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *create_ex)(const char* owner,
        const ShipOotSkeletonSpecV2* spec, uint64_t* skeleton);
    ShipNativeStatus (SHIP_NATIVE_CALL *play_ex)(uint64_t skeleton,
        const char* animation_path, const ShipOotAnimationPlayV2* play);
    ShipNativeStatus (SHIP_NATIVE_CALL *animation_info)(uint64_t skeleton,
        const char* animation_path, ShipOotAnimationInfoV2* info);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_state)(uint64_t skeleton,
        ShipOotAnimationStateV2* state);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_post_limb)(uint64_t skeleton,
        ShipOotSkeletonLimbFnV2 callback, void* user);
    ShipNativeStatus (SHIP_NATIVE_CALL *draw_ex)(void* play_state,
        void* actor, uint64_t skeleton);
} ShipOotSkeletonsV2;
```

### 3.2 Semântica e lifetime

- `create_ex` diferencia explicitamente `AnimationHeader` de `LinkAnimationHeader`. STANDARD mantém normal/flex atual; LINK exige skeleton flex compatível e chama `SkelAnime_InitLink`. Curva continua UNSUPPORTED. A existência de bytes em `oot.resources.read_file` não comprova tipo nem devolve um header vivo. Resolver tipadamente no host e prender skeleton/animações; recusar incompatibilidades detectáveis, sem prometer retargeting ou validar compatibilidade artística de juntas.
- O bridge mantém o formato em cada instância. `v1.update`, `v1.get_frame`, `v1.destroy` e `v1.draw` despacham pelo tipo do handle, sem mudar comportamento dos handles criados pela V1. Para LINK usar `LinkAnimation_Update`, `LinkAnimation_Change` e a fila do PlayState; para STANDARD manter `SkelAnime_Update`/`Animation_Change`. `play_ex` resolve início/fim e reverse explicitamente. `v1.play_animation` preserva sua semântica antiga; código novo usa `play_ex`.
- `v1.get_frame.last_frame` continua sendo o `endFrame` atual; em reverse pode ser zero. Só `asset_last_frame` é o último frame do asset. `animation_info` retorna OK+present=0 se o caminho não existe; FAILURE se existe e falha a carga/validação. Para um asset válido, frame zero também pode ser um limite legítimo.
- O host aloca buffers alinhados e dimensionados com a mesma conta de `SkelAnime_InitLink`: flags internas 9 significam juntas mais raiz e **dois bytes extras**; o ponteiro fornecido é alinhado a 16. Não usar apenas `vector<Vec3s>(limbCount)` e presumir que isso reproduz o caso Link. Dar ao engine buffers externos do host evita dependência da arena Zelda. `ResourceMgr_UnregisterSkeleton` e a liberação pertencem ao mesmo dono da instância, depois das filas.
- Cada `create_ex` tem dono único e cleanup por owner. Skeleton é recurso que pode sobreviver à cena como na V1, mas filas, associação a ator e pose em andamento são invalidadas no teardown: na cena nova é necessário rebind interno ao novo PlayState e reinício explícito da animação. Não transportar ponteiro de ator. V1 mantém sua política publicada; handles novos têm owner e nenhuma callback pendurada após unload.
- `set_post_limb(NULL,NULL)` desregistra. Há um callback por skeleton, somente do dono. O callback é **pós-membro**, observacional para a pose e com matriz daquele membro e escopo render ativos; pode desenhar anexos por `oot.render`. O host salva/restaura pilha, interpolação e estado render para que não vaze ao próximo membro. Não fornece `Gfx**` gravável nem `SkelAnime*`. `draw_ex` informa o ator só para essa chamada.
- Falha/exceção de callback não pula o restante do skeleton; registrar diagnóstico e, após três falhas consecutivas, desativar a callback. UNSUPPORTED significa recusa sem falha. O host drena callback antes de retornar unregister para código externo; no unregister reentrante, término real ocorre ao sair do callback. Sem execução de callback no destroy.

### 3.3 Mudanças no host

`oot_skeletons.h`, `OotNativeSkeletons.{h,cpp}` e `OotNativeSkeletonsGame.cpp`: registro por tipo/owner, getters V2, `Create/Play/Update/Frame/Draw/Destroy`, carga tipada, storage de caminhos, buffers Link e trampoline pós-membro. `OotNativeEngine.cpp`: publicar V2 e chamar `ReleaseOotSkeletonOwner` no unload. Reusar `SkelAnime_DrawFlexOpa`/`SkelAnime_DrawOpa`; não copiar callbacks privados de equipamento do Player. O callback vanilla `oot.player.limb_draw` continua separado e não é disparado artificialmente para este skeleton.

## 4. Corpo customizado: player-body V1

### 4.1 Declarações

```c
#define LINKSPAN_OOT_PLAYER_BODY_SERVICE "linkspan.oot.player-body"
#define LINKSPAN_OOT_PLAYER_BODY_VERSION 1u

typedef struct ShipOotBodyAnimationV1 {
    uint32_t size;
    const char* name;
    const char* path;
    uint8_t reverse_default;
    uint8_t reserved[3];
} ShipOotBodyAnimationV1;

typedef struct ShipOotBodySegmentV1 {
    uint32_t size;
    uint32_t segment; /* 1..15, conforme a API Lua atual */
    const char* resource_path;
} ShipOotBodySegmentV1;

typedef struct ShipOotBodySpecV1 {
    uint32_t size;
    ShipOotSkeletonSpecV2 skeleton;
    const char* default_animation; /* nome lógico */
    const ShipOotBodyAnimationV1* animations;
    uint32_t animation_count;
    const ShipOotBodySegmentV1* segments;
    uint32_t segment_count;
    float ground_offset; /* ajuste visual, não teleporte do Player */
} ShipOotBodySpecV1;

typedef struct ShipOotBodyStateV1 {
    uint32_t size;
    uint64_t body_generation;
    uint64_t scene_epoch;
    uint8_t active;
    uint8_t controlled;
    uint8_t reserved[2];
    ShipOotAnimationStateV2 animation;
} ShipOotBodyStateV1;

typedef struct ShipOotBodyTickV1 {
    uint32_t size;
    uint64_t control;
    void* play_state;
    void* player;
    ShipOotBodyStateV1 state; /* snapshot antes do avanço deste tick */
} ShipOotBodyTickV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotBodyTickFnV1)(
    void* user, const ShipOotBodyTickV1* tick);

typedef struct ShipOotBodyDrawV1 {
    uint32_t size;
    uint64_t control;
    void* play_state;
    void* player;
    void* actor;
    ShipOotBodyStateV1 state;
} ShipOotBodyDrawV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotBodyDrawFnV1)(
    void* user, const ShipOotBodyDrawV1* draw, uint8_t* draw_default);

typedef struct ShipOotBodyControllerV1 {
    uint32_t size;
    ShipOotBodyTickFnV1 tick;
    ShipOotSkeletonLimbFnV2 post_limb;
    ShipOotBodyDrawFnV1 draw; /* opcional, matriz do corpo e escopo render ativos */
    void* user;
} ShipOotBodyControllerV1;

typedef struct ShipOotPlayerBodyV1 {
    uint32_t size;
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *set_body)(const char* owner,
        const ShipOotBodySpecV1* spec); /* NULL desativa o corpo do owner */
    ShipNativeStatus (SHIP_NATIVE_CALL *get_state)(ShipOotBodyStateV1* state);
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_control)(const char* owner,
        uint64_t expected_body_generation, const ShipOotBodyControllerV1* controller,
        uint64_t* control);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_control)(uint64_t control);
    ShipNativeStatus (SHIP_NATIVE_CALL *start_animation)(uint64_t control,
        const char* name, const ShipOotAnimationPlayV2* play, uint32_t lane);
    ShipNativeStatus (SHIP_NATIVE_CALL *animation_info)(uint64_t control,
        const char* name, ShipOotAnimationInfoV2* info);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_segment)(uint64_t control,
        uint32_t segment, const char* resource_path); /* NULL remove */
    ShipNativeStatus (SHIP_NATIVE_CALL *set_root_motion)(uint64_t control,
        uint64_t action_token, uint32_t mode);
} ShipOotPlayerBodyV1;
```

### 4.2 Posse e convivência Lua/DLL

Há **um corpo ativo para o Player**, uma spec e um `body_generation`; não existem corpos Lua e nativo paralelos. O mod que fez `set_body` possui a definição. `acquire_control` exige o mesmo owner, a geração esperada e uma instância ativa; se outro mod possui o corpo, LIMIT. Após reload da spec, toggle ou cena, token de controle antigo deixa de valer. Uma DLL pode criar o corpo por `set_body`; o adapter Lua existente também passa pelo mesmo backend.

Lua conserva `ship.oot.player.set_body/get_body/play_body_animation/set_body_segment`, parsing das tabelas, assets, animação de fallback e `hook.oot.player.body_anim_select`. Nenhuma forma é ativada automaticamente por nome de animação. Opções de conteúdo como água, combo, instrumento, modelo de habilidade e postura dedicada deixam de ser interpretadas como comportamento pelo backend genérico. O pacote de compatibilidade/provider traduz essas opções ou as recebe via `ship.native.call`; não ignorá-las silenciosamente como se a habilidade ainda existisse.

Coexistência mínima: Lua e DLL pertencem ao **mesmo mod**. Lua declara a spec; a DLL consulta `get_state`, adquire controle da geração e usa nomes dessa spec. Acrescentar a `get_body` geração/epoch como strings hexadecimais (evitar perda de precisão de uint64 em Lua); consultas nativas não precisam desses campos Lua. Não permitir adotar o corpo de outro mod por mera descoberta de handle. Delegação entre mods diferentes fica para depois.

Enquanto o controlador nativo detém a animação, `play_body_animation` Lua retorna o padrão de erro/false do binding com motivo de ocupação; não reinicia a animação no meio de uma ação. O próprio mod coordena uma transição por `ship.native.call`, soltando controle primeiro. `set_body(nil)` do owner sempre pode cancelar o corpo e revoga controlador, quads e ação dependentes antes de remover a instância. `set_body` de outro owner recebe ocupação. Alterações cosméticas de segmentos feitas pelo mesmo owner continuam possíveis: são copiadas e aplicadas na fronteira de draw, sem invalidar storage retido pelo renderer.

Ordem de escolha: override explícito ativo > resultado válido de `body_anim_select` > fallback nativo declarado neste tick > fallback genérico da spec. Um loop em OVERRIDE só termina por outro pedido/release; ONCE libera o override após o sinal de término. FALLBACK não sobrepõe um override nem reinicia a mesma animação a cada tick. Este segundo canal representa `DesiredAnimation` da máquina sem acoplar o exe à máquina. Nomes não declarados não chegam ao resource manager como ponteiros arbitrários.

`animation_info` por nome separa hasAnimation (present) de carga e último frame. Nome não declarado: OK+present=0. Nome declarado mas não carregável: FAILURE+present=1, loaded=0. `get_state.animation.current_frame` implementa currentFrame, e `asset_last_frame` implementa animationLastFrame sem confundir reverse com frame zero. A DLL adapta seu `Services`; o struct C++ antigo não cruza a ABI.

### 4.3 Ordem de atualização e root motion

O motor do corpo mantém **um único avanço por tick**. No update do ator auxiliar: publica snapshot anterior; chama `controller.tick`; resolve override/hook/fallback; chama `LinkAnimation_Update`; guarda `finished_previous_update` para o próximo tick; enfileira neutralização/root motion; desenha só depois de a fila de animação processar. Sem segundo avanço pelo hook de ação. Callbacks recebem o estado antes do avanço, preservando o uso anterior de `currentFrame` e do sinal de fim anterior.

`set_root_motion` vale somente para o tick do callback do corpo e precisa de token de ação vivo, do mesmo owner e Player, que tenha suprimido a ação vanilla nesse tick. DISABLED é o padrão a cada tick. XZ enfileira `AnimationContext_SetMoveActor` **depois de LOADFRAME**, com Y scale zero e `ANIM_FLAG_UPDATEY` para neutralizar Y visual; mantém escala/yaw do ator conforme a rotina vanilla. Fora de ação própria continua enfileirando neutralização, com NOMOVE, sem mover o Player. Primeira amostra após troca, reverse, rebind ou habilitação de root motion é NOMOVE, com `baseTransl/prevTransl/prevRot` inicializados; nada de salto herdado da animação anterior. `base_translation` vem da spec, não de uma constante de conteúdo no exe.

Não extrair o delta imediatamente após `LinkAnimation_Update`: a junta ainda pode ser a anterior. Nunca acumular root motion duas vezes por update. Se o token é revogado antes do drain, neutralizar o pedido pendente sem mover o ator; se o Player foi destruído, invalidar/cancelar a entrada antes de liberar seus ponteiros. Uma implementação precisa registrar as entradas enfileiradas para conseguir esse cancelamento, não apenas limpar a struct do corpo.

### 4.4 Cena, unload e arquivos

Na troca de cena o motor preserva **só a spec copiada**, enquanto o mod continua carregado, reproduzindo o pending-restore atual. Invalida controle/callbacks ligados à instância, serial, root motion e atores. Recria o corpo apenas quando o novo Player estiver pronto; nova geração, nova aquisição explícita pela DLL. No unload ou `set_body(nil)`, elimina também a spec e o pending-restore. Nunca restaura callbacks de uma DLL descarregada. Falha de tick libera controle e recursos dependentes e devolve fallback genérico; não deixa ação pausada aguardando três frames de erro.

Extrair a infraestrutura de `ShipLuaBootstrap.cpp` (`CustomBodySpec`, `CustomBodyStartAnimation`, `CustomBodyActivateFromSpec`, update/draw/deactivate e reset de cena) para **novos** `OotNativePlayerBody.{h,cpp}` e `OotNativePlayerBodyGame.cpp`; manter no bootstrap os adapters Lua. Internamente reusar skeletons V2 sem expor ao consumidor um handle mutável do skeleton do corpo. Encaminhar `post_limb` do controlador ao trampoline genérico do skeleton. `CustomBodyPostLimbDraw` deixa de conhecer estado, limbs ou tabelas de uma forma. Remover `MmFormInstallServices` e chamadas à máquina do motor genérico.

O corpo não implica posse da ação, nem imunidade, escudo ou dano. O callback opcional `controller.draw` roda antes do desenho padrão, na matriz do corpo e em escopo render próprio, com `draw_default=1` inicial. Pode desenhar listas/um skeleton alternativo por `oot.render`/`oot.skeletons` e devolver zero para omitir o skeleton padrão naquele draw. Sem callback, ou com callback recusado/falho, desenha o padrão; callback deve emitir comandos só após validar seus recursos. Assim poses alternativas ficam na DLL sem REPLACE global de atores. Não avança animação dentro do draw. O pós-membro só roda quando o skeleton correspondente é efetivamente desenhado; skeleton alternativo usa seu próprio callback V2. Não há promessa de rollback de comandos gráficos já emitidos por callback que depois falha. O host restaura o escopo e desativa callback após três falhas; revogar controle imediatamente remove ambas as callbacks de desenho.

## 5. Ação do Player: movement V3

### 5.1 Declarações

```c
#define LINKSPAN_OOT_MOVEMENT_VERSION_3 3u
#define SHIP_OOT_ACTION_PASS 0u
#define SHIP_OOT_ACTION_SUPPRESS 1u
#define SHIP_OOT_ACTION_STATE_IDLE 0x01u
#define SHIP_OOT_ACTION_STATE_ROLL 0x02u
#define SHIP_OOT_ACTION_STATE_DOOR 0x04u
#define SHIP_OOT_ACTION_STATE_BLOCKED 0x08u

typedef struct ShipOotPlayerActionStateV1 {
    uint32_t size;
    uint64_t scene_epoch;
    uint64_t player_generation;
    uint64_t tick;
    uint32_t flags; /* estados semânticos, não endereços privados */
    uint8_t owned;
    uint8_t reserved[3];
} ShipOotPlayerActionStateV1;

typedef struct ShipOotPlayerActionTickV1 {
    uint32_t size;
    uint64_t action_token;
    void* play_state;
    void* player;
    void* input;
    ShipOotPlayerActionStateV1 state;
    uint8_t vanilla_allowed;
    uint8_t reserved[3];
} ShipOotPlayerActionTickV1;

typedef ShipNativeStatus (SHIP_NATIVE_CALL *ShipOotPlayerActionFnV1)(
    void* user, const ShipOotPlayerActionTickV1* tick, uint32_t* disposition);

typedef struct ShipOotPlayerActionSpecV1 {
    uint32_t size;
    ShipOotPlayerActionFnV1 callback;
    void* user;
} ShipOotPlayerActionSpecV1;

typedef struct ShipOotMovementV3 {
    ShipOotMovementV2 v2; /* inclui size e layout_id originais */
    ShipNativeStatus (SHIP_NATIVE_CALL *get_action_state)(ShipOotPlayerActionStateV1* state);
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_action)(const char* owner,
        const ShipOotPlayerActionSpecV1* spec, uint64_t* action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_action)(uint64_t action_token);
    ShipNativeStatus (SHIP_NATIVE_CALL *restore_player_animation)(uint64_t action_token);
} ShipOotMovementV3;
```

### 5.2 Arbitragem e integração

Assumir ação significa controlar o despacho de `actionFunc`, não gravar um ponteiro da DLL nesse campo. Um slot exclusivo por Player/epoch; callback retorna PASS (original, sujeito ao gate existente) ou SUPPRESS (não executar ação vanilla). O host conserva o restante de `Player_UpdateCommon`, colisão, câmera e atualização comum. A função original só pode rodar uma vez; `vanilla_allowed=0` nunca é convertido em true pela DLL. Não se implementa posse reafirmando PAUSE no final do frame e esperando que o valor sobreviva ao reset vanilla.

Inserir o bridge exatamente na decisão de `VB_EXECUTE_PLAYER_ACTION_FUNC` em `z_player.c:12113`. Chamar o controlador antes da execução da ação autorizada. `acquire_action` funciona no update anterior ou em um ponto de entrada antes da ação; no próprio frame de aquisição só vale se a fronteira ainda não passou. A aquisição por callback tardio do corpo não pode apagar uma ação já executada. A DLL consulta o tick/estado e começa a física no tick em que obtém SUPPRESS; não simular retroativamente o frame de entrada.

O host recusa aquisição em pausa, morte, cutscene, item-cs, porta/transição, escalada, nado/estado incompatível e Player ainda não pronto. Com lease vivo, reavalia os gates antes de cada callback; se vanilla tomou uma ação forçada (dano, porta, cutscene etc.), revoga imediatamente a posse e preserva essa ação. Esse gate deve derivar das condições reais do Player, não de uma lista hardcoded de animações da forma.

Guardar actionFunc interno e geração da ação no acquire. Mudança externa de ação que não foi do bridge revoga o lease; um endereço que coincidentemente voltou ao mesmo valor não recupera posse. Para isso, versionar a troca no helper central de ação do Player e auditar atribuições diretas. `get_action_state` produz IDLE/ROLL/DOOR/BLOCKED no host; ROLL é a rolagem vanilla, não uma habilidade de mod. Não entregar `Player_Action_Idle`, `Player_Action_Roll` ou `Player_Action_80845EF8` ao consumidor.

`release_action` é cleanup incondicional do lease: desarma quads/root motion dependentes e retira a supressão. Se o bridge ainda controla a ação e o Player pode voltar ao neutro, executa helper do host para transição válida a Idle, usando a rotina central de mudança de ação e zerando apenas estado transitório que o bridge possui. Seleciona `Player_GetIdleAnim` e chama `LinkAnimation_Change(... LOOP ...)` com o modelo/equipamento atual. Soltar PAUSE sem restaurar animação não basta: um loop externo poderia nunca terminar.

Se morte, água, dano, porta ou cutscene tomou a ação, release continua OK, mas **não força Idle** sobre ela. Preserva a ação e a animação vanilla que a nova condição exige. Durante teardown do Player só invalida; não chama funções no ator morto. Não ressuscita uma função antiga capturada no acquire, nem restaura structs inteiras do Player.

`restore_player_animation` restaura apenas a animação neutra vanilla, conservando o lease de ação e o corpo. Requer token vivo e estado em que Idle é apropriado; caso contrário UNSUPPORTED. `release_action` inclui essa restauração quando aplicável, inclusive no unload. Estado que a DLL alterou diretamente fora do serviço (gravidade, escala, flags arbitrárias etc.) continua responsabilidade dela; para garantir rollback no unload, essas futuras mutações também terão de passar por propriedades genéricas com dono. O lease não oferece rollback mágico de escritas cruas.

### 5.3 Hooks e arquivos

Implementar `OotNativePlayerAction.{h,cpp}` e `OotNativePlayerActionGame.cpp`, novos bridges internos; declarar a tabela V3 em `oot_engine.h` e publicá-la em `OotNativeEngine.cpp`. Em `z_player.c`, adicionar helpers genéricos próximos ao dispatcher/rotina de mudança de ação, usando internamente `Player_GetIdleAnim` e `Player_Action_Idle`; nenhum deles vira símbolo resolvível pelo mod.

Para estas cinco lacunas, o callback exclusivo acima é a fronteira de ação necessária. Não abrir simultaneamente um REPLACE público independente que possa disputar esse slot. O `oot.player.action` proposto pelo inventário pode começar como **OBSERVE BEFORE/AFTER**, payload com tick/Player/epoch/vanilla_allowed e resultado de supressão, em `oot_hooks.h` e `OotNativeHooks{,Game}.cpp`. Um futuro REPLACE deve ser integrado à mesma arbitragem (segundo dono LIMIT), não despachado por fora. Isso é um refinamento deliberado da proposta do inventário, não uma afirmação de que o ponto já existe.

`oot.player.update_end` OBSERVE é útil para acompanhar reaquisição da cena e estados posteriores; inserir junto a `GameInteractor_ExecuteOnPlayerUpdate` (`z_player.c:12353`). `oot.play.destroy` pode notificar, mas cleanup obrigatório dos serviços não depende da existência de subscribers nem do callback do mod.

## 6. Câmera: camera V2

### 6.1 Declarações

```c
#define LINKSPAN_OOT_CAMERA_VERSION_2 2u

typedef struct ShipOotCameraViewV2 {
    ShipOotCameraViewV1 v1; /* size, eye, at, fov, mesmos offsets */
    int16_t roll; /* ângulo binário assinado; 0x4000 = 90 graus */
    uint16_t reserved;
} ShipOotCameraViewV2;

typedef struct ShipOotCameraLeaseV2 {
    uint32_t size;
    uint64_t scene_epoch;
    uint64_t camera_generation;
    uint8_t owned;
    uint8_t reserved[3];
} ShipOotCameraLeaseV2;

typedef struct ShipOotCameraV2 {
    ShipOotCameraV1 v1;
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_ex)(const char* owner,
        uint64_t* token, ShipOotCameraLeaseV2* lease);
    ShipNativeStatus (SHIP_NATIVE_CALL *set_view_ex)(uint64_t token,
        const ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_view_ex)(ShipOotCameraViewV2* view);
    ShipNativeStatus (SHIP_NATIVE_CALL *get_lease)(uint64_t token,
        ShipOotCameraLeaseV2* lease);
} ShipOotCameraV2;
```

### 6.2 Semântica e restauração

Mesmo slot global de câmera para V1, V2 e adapter Lua que assumir gameplay. Não usar `gCutsceneCamId` e o novo serviço como dois árbitros separados. `acquire_ex` copia eye/at/fov/**roll** da principal para a subcâmera e guarda identidade/geração/status prévios da principal. Continua recusando câmera de cutscene ou outro dono com LIMIT; não congela atores ou toma input por consequência.

`set_view_ex` usa validação de eye/at/fov atual e copia roll sem conversão implícita de graus/radianos. O bridge escreve `Camera::roll` da subcâmera e reaplica depois da atualização da câmera como faz com a vista atual. `get_view_ex` lê a câmera ativa, de qualquer dono. `get_lease` informa perda de posse sem expor `Camera*`. `v1.set_view` preserva o roll corrente da subcâmera; não zera um roll que só a V2 conhece. `v1.get_view` não escreve a cauda V2, e `v1.release` solta tokens V2 também.

Release/unload na mesma cena: desativar e limpar só a subcâmera daquela geração. Se ela ainda era ativa e a principal continua sob o status de espera imposto pelo lease, devolver principal ao status anterior, normalmente ACTIVE. O snapshot da vista e roll da principal serve para desfazer **somente valores alterados pelo próprio lease**. O serviço não copia a vista/roll da subcâmera sobre a principal no release. Como o serviço normalmente só alterou o status da principal, sua vista/roll original já está preservada; alterações legítimas de terceiros na principal são mantidas. Não restaurar a struct Camera inteira nem seus ponteiros internos.

Se cutscene/outra câmera tomou prioridade: revogar token, remover somente a subcâmera identificada e **não** reativar principal nem escrever roll na câmera nova. Se scene_epoch/camera_generation não confere, só invalidar o registro; não tocar no id reutilizado. Em teardown anterior à liberação pode remover a subcâmera; em teardown tardio só esquecer. Não restaurar câmera antiga após entrar na próxima cena; exige nova aquisição. Interpolação deve marcar a descontinuidade de câmera nas trocas, sem reutilizar o epoch de render como identidade de lease.

Arquivos: `oot_camera.h`, `OotNativeView.{h,cpp}`, `OotNativeViewGame.cpp`: `ViewState`, `CameraCreate/Valid/Apply/Read/Destroy`, `DropCamera`, validação por epoch/geração, nova tabela e `ReleaseOotCameraOwner`. `OotNativeEngine.cpp`: publicar V2 e cleanup. Encaminhar o caminho Lua genérico de câmera/cutscene de `ShipLuaBootstrap.cpp` ao mesmo árbitro; tempos e roll da transformação saem para o provider. Easing, pilha de câmeras, blend de donos e restauração interpolada ficam fora da V2.

## 7. Quads do Player: colliders V2

### 7.1 Declarações

```c
#define LINKSPAN_OOT_COLLIDERS_VERSION_2 2u
#define SHIP_OOT_QUAD_ATTACK_0 0u
#define SHIP_OOT_QUAD_ATTACK_1 1u
#define SHIP_OOT_QUAD_DEFENSE 2u
#define SHIP_OOT_QUAD_MASK_ATTACK 0x03u
#define SHIP_OOT_QUAD_MASK_DEFENSE 0x04u

typedef struct ShipOotPlayerQuadSpecV2 {
    uint32_t size;
    float vertices[4][3]; /* mundo, mesma ordem de Collider_SetQuadVertices(a,b,c,d) */
    uint8_t col_type;
    uint8_t at_flags;
    uint8_t ac_flags;
    uint8_t elem_type;
    uint32_t touch_dmg_flags;
    uint32_t bump_dmg_flags;
    uint8_t touch_effect;
    uint8_t touch_damage;
    uint8_t touch_flags;
    uint8_t bump_effect;
    uint8_t bump_defense;
    uint8_t bump_flags;
    uint8_t defending; /* só para o slot DEFENSE: estado de defesa do Player */
    uint8_t reserved;
    float defense_matrix[16]; /* só DEFENSE: MtxF, m[coluna][linha], sem perspectiva */
} ShipOotPlayerQuadSpecV2;

typedef struct ShipOotPlayerQuadHitsV2 {
    uint32_t size;
    uint64_t scene_epoch;
    uint64_t resolved_tick;
    ShipOotColliderHitsV1 hits; /* snapshot; OC sempre zero */
} ShipOotPlayerQuadHitsV2;

typedef struct ShipOotCollidersV2 {
    ShipOotCollidersV1 v1;
    const char* layout_id;
    ShipNativeStatus (SHIP_NATIVE_CALL *acquire_player_quads)(const char* owner,
        uint64_t action_token, uint32_t slot_mask, uint64_t* lease);
    ShipNativeStatus (SHIP_NATIVE_CALL *submit_player_quad)(uint64_t lease,
        uint32_t slot, const ShipOotPlayerQuadSpecV2* spec);
    ShipNativeStatus (SHIP_NATIVE_CALL *disable_player_quad)(uint64_t lease,
        uint32_t slot);
    ShipNativeStatus (SHIP_NATIVE_CALL *read_player_quad_hits)(uint64_t lease,
        uint32_t slot, ShipOotPlayerQuadHitsV2* hits);
    ShipNativeStatus (SHIP_NATIVE_CALL *release_player_quads)(uint64_t lease);
} ShipOotCollidersV2;
```

### 7.2 Slots, fases e restituição

Slots mapeiam para `Player::meleeWeaponQuads[0]`, `[1]` e `shieldQuad`. Não criar uma segunda cópia ampla do shield: o caminho original participa de bloqueio/reflexão. Posse por conjunto, atômica, com action_token do mesmo owner/Player/epoch. O conjunto ATTACK é indivisível (máscara 0x03): ao tomar um ataque, reserva os dois quads para impedir dano da espada invisível. DEFENSE pode ser separado; por padrão um mod que toma ambos pede 0x07. Qualquer interseção ocupada retorna LIMIT sem reserva parcial.

Aquisição guarda só a configuração estável dos slots e os campos adicionais que o serviço passar a possuir (`shieldMf`, bit SHIELDING); não copia/restaura `ColliderQuad` inteiro, ponteiros de acerto ou vértices antigos. Todos começam inativos. A DLL fornece geometria/dano/material; nada disso é uma tabela fixa no exe. Para DEFENSE informar matriz coerente com os vértices para preservar direção de bloqueio/reflexão; validar valores finitos e matriz afim. Não presumir que mover os quatro vértices atualiza `shieldMf` automaticamente. Seleção de material conforme equipamento pode ser feita na DLL lendo o Player compatível ou em uma futura opção genérica; não fixar um material por forma.

Submit copia os dados para staging do tick e valida máscara, slot, flags AT/AC e tipos do SDK; somente flags de configuração, nunca HIT/BOUNCED/ponteiros. Ataque pode inscrever AT; defesa AT/AC conforme a spec; OC não é suportado nesses quads. `defending` controla o bit de defesa apenas enquanto o lease existe. É preciso reapresentar a spec a cada tick; slot não apresentado fica desarmado, inclusive se o callback não rodar. `disable` cancela staging e retira qualquer submissão pendente do slot, sem destruir memória do Player.

**Ordem real do host:** `z_play.c:1220–1234` resolve AT/OC/dano da lista anterior, limpa contexto e só depois chama `Actor_UpdateAll`. A fila de animação é drenada mais adiante (`:1306`), e draw pode atualizar vértices. Portanto não prometer colisão imediata no mesmo tick do submit, nem colocar um flush fictício “antes da colisão” supondo que esta venha após update. Staging do tick N prepara a lista para a próxima passagem real de CollisionCheck. Capturar resultados depois da resolução e antes dos resets do Player; `resolved_tick` identifica esse ciclo.

O host fornece uma fronteira única ao fim das atualizações para materializar staging, e filtra a entrada de **todos** os `CollisionCheck_SetAT/SetAC` dos slots arrendados. Chamadas vanilla de update ou draw para esses mesmos endereços não podem reinscrever a espada/escudo nem sobrescrever a configuração alugada. Ao adquirir/revogar no meio de um frame, remover referências desses slots já presentes nas listas AT/AC pendentes (ou fazê-las ser ignoradas no consumo). Não basta baixar AT_ON ou zerar ponteiros: listas podem continuar guardando o collider.

`read_player_quad_hits` lê e consome um snapshot do serviço, não limpa os bits vivos que os consumidores vanilla precisam para bloquear, refletir ou tocar reação. Guardar o resultado na fronteira de resolução evita perdê-lo nos `Collider_ResetQuad*` de `Player_UpdateCommon` (`z_player.c:12211–12215`). `*_actor` no snapshot só é devolvido se o ator continuar vivo no frame da leitura, senão NULL; não carregar ponteiro da cena anterior. Leitura repetida sem nova resolução dá flags zero.

Release, perda da ação, troca de corpo, teardown ou unload: parar inscrições, retirar entradas pendentes, limpar flags transitórias/HIT e snapshots, restaurar configuração estável e o bit/matriz de defesa **se ainda pertencem ao lease**, e entregar a geometria nova ao próximo update/draw vanilla. Não restaurar `at/ac/oc` de um frame antigo nem rearmar ataques que estavam ativos na aquisição. Os quads pertencem ao Player; jamais chamar destroy/free neles. No teardown tardio só descartar referências, pois o Player já pode ter sido liberado.

### 7.3 Arquivos e callbacks necessários

`oot_colliders.h`, `OotNativeWorld.{h,cpp}` e `OotNativeWorldGame.cpp`: lease dos slots, staging, cópia de resultado, configuração e getters V2; manter cilindros V1 intactos. `OotNativeEngine.cpp`: publicar V2 e `ReleaseOotPlayerQuadOwner`.

`z_player.c`: preservar resultado antes dos resets e impedir que caminhos de armas reconfigurem slots arrendados. `z_player_lib.c`: proteger atualização vanilla dos quads de arma/escudo durante draw, inclusive `Player_UpdateShieldCollider` e o caminho de `func_80090480`. `z_collision_check.c`: bridge genérico de reserva de collider do Player para filtrar/retirar entradas AT/AC, sem citar conteúdo de mod. `z_play.c`: fronteiras de snapshot após resolver colisão, flush após atualizações e revogação antes do teardown. Auditar demais escritas aos três quads; só bloquear SetAT/AC não protege os vértices.

Não é necessário um hook público novo por colisão: resultados são consultados pelo lease. Cilindro do Player usado por outra habilidade e mutações de raio/massa/gravidade não estão cobertos por este contrato de quads; podem continuar sendo código da DLL com restauração própria, ou virar uma extensão separada com dono se a garantia de rollback automático também for exigida para eles.

## 8. Menor entrega e YAGNI

**Necessário agora:** publicação simultânea de versões, um árbitro de corpo para Lua/nativo, quatro consultas/operações de animação pedidas pela máquina, LinkAnimation tipada, pós-membro com render scope, lease de ação com Idle/animação vanilla, roll da câmera, quads reais com defesa/reflexão, owner cleanup, epochs e cancelamento seguro de filas. Root motion XZ é opt-in mas já é necessário para preservar as ações que o usam; não relegá-lo a futuro se a paridade delas for requisito.

**Pode esperar se o port não usar:** root motion Y/XYZ, retargeting, mistura de poses, controle arbitrário de juntas e callback override pré-membro, retorno de header cru, quads genéricos de qualquer ator, OC em quads, prioridades/preempção de donos, delegação de corpo entre mods, pilha/blend/easing de câmeras, restauração de actionFunc capturada, async loading e API Lua nova para controlar diretamente os quads. O pós-membro observacional basta para anexos; só ampliar se houver consumidor comprovado. O draw opcional por corpo já está previsto para a pose alternativa necessária ao port; deixá-lo NULL custa apenas o caminho padrão.

**Recorte que exige decisão na implementação:** assumir rollback automático de gravidade/cilindro/input/ambiente exige primitivas com dono adicionais. Estes cinco contratos não conseguem restaurar escritas arbitrárias da DLL. O provider deve migrar seu `ReleaseBody` e executá-lo nas saídas normais, mas isso sozinho não garante recuperação caso ele falhe ou seja descarregado antes de restaurar campos que escreveu por fora dos serviços.

Hooks de voz, gates de movimento, máscara/item, provider de áudio, catálogo de atores e aliases de archives do inventário continuam trabalhos separados. Não criar serviços específicos de conteúdo para preencher essas lacunas. Não ampliar o catálogo de resolução por nome além de `functions.h`/`variables.h`.

## 9. Validação exigida antes de implementar o port

1. **ABI sem ROM:** compilar headers em C/C++, comparar offset de cada campo V1/V2 antigo com a tabela nova, assinaturas/calling convention e tamanhos; pedir versões exatas/minimumSize; confirmar recusa em host antigo, tabela curta, enum inválido e thread errada.
2. **Posse:** dois mods e Lua/DLL no mesmo mod; conflitos sem roubo, geração antiga, mesmo endereço após reload, unload parcial e finalizador tentando rearmar. Nenhum callback após unload; invariantes de listas/filas verificadas antes da biblioteca sair.
3. **Animação:** STANDARD e LINK, reverse, ONCE/LOOP, asset de um frame, last-frame do asset versus endFrame, strings temporárias liberadas, reset/rebind e destroy com LOADFRAME pendente. Instrumentar exatamente um avanço e um MOVEACTOR por tick.
4. **Ação e corpo:** corpo Lua sem DLL, corpo nativo, controle nativo da spec Lua do mesmo mod; override/hook/fallback; liberar em loop externo, rolagem, dano, água, porta, pausa e cutscene; preservar ações forçadas e não deixar loop externo no Link vanilla.
5. **Quads:** espada invisível desarmada, janelas sem submit expirando, slots não duplicados por draw, hits antes do reset, reação/ricochete de escudo, matriz orientada, release com entradas já na lista e cena encerrada com ataque ativo. Validar o ciclo de colisão seguinte, não só flags no tick de submit.
6. **Câmera:** V1 e V2 no mesmo árbitro, roll positivo/negativo, release normal, cutscene que rouba câmera, id de subcâmera reutilizado, scene reload, unload no meio do efeito e Lua concorrente.

Validação deste documento: conferência estática dos headers/bridges e das fronteiras citadas; status confrontados com a ABI real; os cinco blocos compilados juntos em modo sintático **C11 e C++17**, com GCC/MinGW, `-Wall -Wextra -Werror`, usando os headers reais do host/core; dez assertions de prefixo/posição da cauda aprovadas em cada linguagem. UTF-8, fechamento dos blocos, nomes genéricos nas declarações e existência dos arquivos de integração atuais também conferidos. Isso não substitui validação de ABI com o compilador MSVC do host nem implementações das funções. Não foi executado build do host, teste de DLL ou playtest; ordem gráfica, compatibilidade de cada asset e todos os caminhos de alteração de ação/quad ainda precisam das verificações acima na implementação. Não existe grafo `graphify-out/graph.json` no host nem nesta pasta; foram usadas buscas delimitadas e leitura direta, sem gerar grafo ou instalar ferramenta fora do escopo. Único arquivo produzido: `CONTRATOS.md`; nenhuma alteração nos repositórios e nenhum commit.
