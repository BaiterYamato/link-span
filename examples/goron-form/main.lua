local ship = require("ship")

-- Forma Goron composta de primitivas genéricas. Nenhuma função do host se
-- chama "goron": o mod é que decide o que a forma significa, inclusive o
-- corpo visual (ship.oot.player.set_body é genérica — skeleton + animações
-- nomeadas livremente por quem chama, o host não sabe o que é um "Goron").
--
--   imunidade a fogo -> ship.oot.player.set_damage_immunity("fire", true)
--   peso pesado      -> ship.oot.player.set_weight("heavy")
--   rolamento        -> ship.oot.player.set_roll_mode("chain")
--   corpo visual     -> ship.oot.player.set_body({ skeleton=..., anims=... })
--
-- Uma forma Zora, Deku, ou de lobo (Wolfos/Wolf Link) usa exatamente as
-- mesmas peças — só troca os caminhos de asset e os limiares de animação
-- abaixo. Nada disto precisa de C++ novo no host.

local SPEED = 1.15 -- Goron anda pesado, mas rola rápido

-- Skeleton + animações reais do Goron do MM, lidos ao vivo do mm.o2r
-- cross-world (25 limbs — não cabe nos buffers fixos do Player, por isso
-- set_body usa um SkelAnime próprio dimensionado pelo host).
local GORON_BODY = {
    skeleton = "mm/objects/object_link_goron/gLinkGoronSkel",
    anims = {
        idle = "mm/objects/gameplay_keep/gPlayerAnim_pg_wait",
        walk = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_walk_free",
        run = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_run_free",
        -- A seleção padrão do host usa esta animação enquanto o Goron está
        -- sem chão sob os pés; é o header de pulo normal do Player do MM.
        jump = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_jump",
        fall = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_fall",
        land = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_landing_free",
        damage = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_damage_run_free",
        -- Tabela Goron de MM: os dois passos alternados sao reutilizados em
        -- sentido inverso para descer escada/vinha; os starts/finais ficam
        -- declarados para a spec poder representar o conjunto completo.
        climb_start_a = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_startA",
        climb_start_b = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_startB",
        climb_up_l = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_upL",
        climb_up_r = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_upR",
        climb_down_l = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_upL",
        climb_down_r = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_upR",
        climb_wait = "mm/objects/gameplay_keep/gPlayerAnim_link_normal_jump_climb_wait",
        climb_end_a_l = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_endAL",
        climb_end_a_r = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_endAR",
        climb_end_b_l = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_endBL",
        climb_end_b_r = "mm/objects/gameplay_keep/gPlayerAnim_pg_climb_endBR",
        door_open_left = "mm/objects/gameplay_keep/gPlayerAnim_pg_doorA_open",
        door_open_right = "mm/objects/gameplay_keep/gPlayerAnim_pg_doorB_open",
        chest_open = "mm/objects/gameplay_keep/gPlayerAnim_pg_Tbox_open",
        curl = "mm/objects/gameplay_keep/gPlayerAnim_pg_maru_change",
        roll_enter = "mm/objects/gameplay_keep/gPlayerAnim_pg_maru_change",
        roll_exit = "mm/objects/gameplay_keep/gPlayerAnim_pg_maru_change",
        mask_off = "mm/objects/gameplay_keep/gPlayerAnim_pg_maskoffstart",
        punch_a = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchA",
        punch_b = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchB",
        punch_c = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchC",
        -- Recuperações reais da tabela sMeleeAttackAnimInfo de MM. O bridge
        -- as escolhe automaticamente ao fim de cada punch (a variante `R`
        -- quando há movimento), sem expor controle de física para Lua.
        punch_a_end = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchAend",
        punch_b_end = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchBend",
        punch_c_end = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchCend",
        punch_a_end_run = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchAendR",
        punch_b_end_run = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchBendR",
        punch_c_end_run = "mm/objects/gameplay_keep/gPlayerAnim_pg_punchCendR",
        -- Tambores Goron: a entrada e a saída usam o mesmo `gakkistart`,
        -- com a saída tocada ao contrário; `gakkiplay` permanece em loop
        -- enquanto o OoT está no modo de ocarina. As cinco variantes são a
        -- tabela D_8085D714 do Player de MM (A, esquerda, baixo, cima, direita).
        gakki_start = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkistart",
        gakki_wait = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiwait",
        gakki_play = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplay",
        gakki_play_a = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplayA",
        gakki_play_l = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplayL",
        gakki_play_d = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplayD",
        gakki_play_u = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplayU",
        gakki_play_r = "mm/objects/gameplay_keep/gPlayerAnim_pg_gakkiplayR",
    },
    -- Descer NÃO usa mais `reverse_anims`. O OoT já desce a escada tocando a
    -- mesma animação com playSpeed negativa (z_player.c:13218), e o host passou
    -- a espelhar essa velocidade no corpo externo — inverter aqui por cima
    -- cancelaria o sinal e a descida voltaria a subir.
    reverse_anims = {},
    -- A cabeça Goron lê o segmento N64 0x08; sem ele o corpo inteiro aparece,
    -- mas os olhos ficam transparentes. O host carrega este resource antes do
    -- skeleton e o preserva para qualquer mod que troque a forma.
    segments = {
        [8] = "mm/objects/object_link_goron/gLinkGoronEyesOpenTex",
    },
    -- Enquanto o Player do OoT está em Player_Action_Roll, o host troca o
    -- skeleton ereto por esta display list enrolada e a gira a cada frame.
    models = {
        roll = "mm/objects/object_link_goron/gLinkGoronCurledDL",
        -- Geometria opaca dos espinhos. O bridge so a desenha apos 60 frames
        -- de A mantido durante o roll; os efeitos de energia continuam em um
        -- passe separado, como no Player original de MM.
        roll_spikes = "mm/objects/object_link_goron/object_link_goron_DL_00C540",
        -- Os dois passes translúcidos do carregamento (grt_01/grt_02). O host
        -- instala o TwoTexScroll no segmento 0x08 antes de os desenhar.
        roll_energy_1 = "mm/objects/object_link_goron/object_link_goron_DL_0127B0",
        roll_energy_2 = "mm/objects/object_link_goron/object_link_goron_DL_0134D0",
        -- Efeito vermelho translúcido de impacto dos socos A/B/C. O bridge o
        -- prende à mão/cintura correta apenas nas janelas de dano de MM.
        punch_effect = "mm/objects/object_link_goron/gLinkGoronGoronPunchEffectDL",
        -- O MM desenha estes seis display lists no torso durante as animações
        -- gakki. Eles não pertencem à malha base e por isso são modelos
        -- separados, anexados pelo callback de limb do bridge.
        gakki_container = "mm/objects/object_link_goron/object_link_goron_DL_00FC18",
        gakki_piece_1 = "mm/objects/object_link_goron/object_link_goron_DL_010590",
        gakki_piece_2 = "mm/objects/object_link_goron/object_link_goron_DL_010368",
        gakki_piece_3 = "mm/objects/object_link_goron/object_link_goron_DL_010140",
        gakki_piece_4 = "mm/objects/object_link_goron/object_link_goron_DL_00FF18",
        gakki_piece_5 = "mm/objects/object_link_goron/object_link_goron_DL_00FCF0",
    },
    -- A defesa do Goron é outro skeleton de quatro membros e usa AnimationHeader
    -- normal (não LinkAnimationHeader). O bridge a mostra enquanto R está
    -- mantido, sem substituir a lógica de bloqueio que o Player do OoT já
    -- tiver disponível.
    shield = {
        skeleton = "mm/objects/object_link_goron/gLinkGoronShieldingSkel",
        animation = "mm/objects/object_link_goron/gLinkGoronShieldingAnim",
    },
    -- Deslocamento vertical do corpo, EM UNIDADES DE MUNDO (o host divide pela
    -- escala antes de gravar em shape.yOffset, e Actor_Draw multiplica de volta
    -- — ver ShipLuaBootstrap.cpp, CustomBodyActorUpdate).
    --
    -- CUIDADO ao calibrar: o skeleton Goron tem cerca de 60 unidades de altura,
    -- então valores muito negativos enterram o corpo inteiro sob o piso e ele
    -- some da tela — indistinguível de "não desenhou". Uma calibração anterior
    -- chegou a -210 às cegas e deixou a forma invisível.
    --
    -- Valor calibrado VISUALMENTE em jogo: -10. O comentário do host afirma que
    -- o corpo ereto flutuaria ~30 unidades sem compensação, mas na prática -30
    -- já deixava o Goron afundado no piso — ou seja, o root do skeleton
    -- convertido já pousa quase alinhado e só precisa de um ajuste fino. Confie
    -- no teste em jogo, não naquela suposição.
    ground_offset = -10.0,
    roll_offset = 12.0,
    -- Goron nao nada: em agua funda, enrola, afunda e volta pelo void-out
    -- nativo. A regra e opt-in da spec para nao afetar outros corpos.
    water_void = true,
    -- MM Goron nunca agarra bordas; o gate é exclusivo deste corpo e não
    -- reutiliza o estado global de Crowd Control do OoT.
    block_ledge_grab = true,
    default_anim = "idle",
}

local transformed = false
local removeAfterMaskOff = false
local transforming = false
local transformGeneration = 0
local maskResetPending = false
local blinkStep = 0
local blinkTimer = 0
local surprisedTimer = 0
local eyesResetPending = false
local doorActive = false
local chestActive = false
local EYES = {
    "mm/objects/object_link_goron/gLinkGoronEyesOpenTex",
    "mm/objects/object_link_goron/gLinkGoronEyesHalfTex",
    "mm/objects/object_link_goron/gLinkGoronEyesClosedTex",
    "mm/objects/object_link_goron/gLinkGoronEyesHalfTex",
}
local SURPRISED_EYES = "mm/objects/object_link_goron/gLinkGoronEyesSurprisedTex"

-- A troca de cena derruba temporariamente o corpo externo, mas preserva a
-- spec para restaurá-lo no Player novo. Se o timer terminar nesse intervalo,
-- set_body_segment ainda não pode aplicar a textura. Mantenha a restauração
-- pendente e tente de novo nos frames seguintes, já com o corpo reativado.
local function reset_eyes()
    blinkStep = 0
    blinkTimer = 0
    surprisedTimer = 0
    if not ship.capabilities.has("oot.player.custom_body") then
        eyesResetPending = false
        return true
    end
    eyesResetPending = not ship.oot.player.set_body_segment(8, EYES[1])
    return not eyesResetPending
end

local function apply(on)
    ship.oot.player.set_damage_immunity("fire", on)
    ship.oot.player.set_weight(on and "heavy" or "normal")
    ship.oot.player.set_roll_mode(on and "chain" or "vanilla")

    if ship.capabilities.has("player.speed") then
        ship.player.set_speed_multiplier(on and SPEED or 1.0)
    end

    -- Corpo visual: tenta o skeleton real primeiro (precisa do mm.o2r
    -- montado); se faltar, cai para a máscara Goron como marcador.
    if ship.capabilities.has("oot.player.custom_body") then
        -- Restaura o segmento antes de liberar o corpo; depois de set_body(nil)
        -- a API não pode mais alterar a spec ativa.
        if not on then
            reset_eyes()
        end
        local okBody = ship.oot.player.set_body(on and GORON_BODY or nil)
        if okBody then
            if on then
                reset_eyes()
            end
            -- The mask is visible only during the human mask-on animation.
            -- Clear it before Link resumes drawing after a body teardown.
            if not on and ship.capabilities.has("oot.player.mask") then
                ship.oot.player.set_mask("none")
            end
        elseif ship.capabilities.has("oot.player.mask") then
            ship.oot.player.set_mask(on and "goron" or "none")
        end
    elseif ship.capabilities.has("oot.player.mask") then
        ship.oot.player.set_mask(on and "goron" or "none")
    end
end

-- Sem callback nenhum, set_body escolhe animação por limiar de velocidade
-- usando as chaves convencionais "idle"/"walk"/"run" — é o que este mod usa.
-- Uma forma Zora, por exemplo, pode preferir nomes próprios ("swim" em vez
-- de "walk") ou lógica de seleção totalmente diferente (debaixo d'água vs.
-- fora): nesse caso, assine hook.oot.player.body_anim_select e devolva o
-- nome da animação via ship.hooks.result() a cada frame — o host não impõe
-- nenhum vocabulário, só espera uma chave que exista na tabela `anims`.
--
-- ship.events.on("hook.oot.player.body_anim_select", function(payload)
--     ship.hooks.result(payload.on_ground and "idle" or "swim")
-- end)

ship.events.on("hook.oot.player.body_anim_select", function(payload)
    if transformed and payload.door_opening then
        if not doorActive then
            doorActive = true
            local animation = payload.door_direction == "left" and "door_open_left" or "door_open_right"
            ship.oot.player.play_body_animation(animation, "once", 1.0)
        end
        return
    end
    doorActive = false

    if transformed and payload.chest_opening then
        if not chestActive then
            chestActive = true
            ship.oot.player.play_body_animation("chest_open", "once", 1.0)
        end
        return
    end
    chestActive = false

    if transformed and payload.climbing and not payload.rolling then
        if payload.climb_starting then
            ship.hooks.result(payload.climb_step == 0 and "climb_start_a" or "climb_start_b")
        elseif payload.climb_direction == "up" then
            ship.hooks.result(payload.climb_step == 0 and "climb_up_l" or "climb_up_r")
        elseif payload.climb_direction == "down" then
            ship.hooks.result(payload.climb_step == 0 and "climb_down_l" or "climb_down_r")
        else
            ship.hooks.result("climb_wait")
        end
        return
    end

    -- Instrumento (tambores) e soco NÃO são mais dirigidos daqui.
    --
    -- Eram, e não funcionavam: uma ação com estado — abrir os tambores, tocar
    -- em loop, recolher; encadear A/B/C — não sobrevive a um round-trip de um
    -- frame no qual o resultado é descartado enquanto um one-shot anterior
    -- ainda corre. O host passou a possuir as duas ações, como faz o Player de
    -- MM (e como a referência skijer/Not-Enough-Items resolve no OoT).
    --
    -- O mod continua declarando os assets (`anims`/`models` acima): o host não
    -- sabe o que é um Goron, só toca o que a spec nomeou. Para observar o
    -- estado, `payload.instrument`, `payload.attacking` e `payload.punch_step`
    -- continuam disponíveis — agora como leitura, não como comando.

    if transformed and payload.falling and not payload.rolling then
        ship.hooks.result("fall")
        return
    end

    if transformed and payload.roll_phase == "enter" then
        ship.hooks.result("roll_enter")
    end
end)

-- A máscara forçada vive no SaveContext do OoT. Se uma execução anterior
-- terminou enquanto a forma estava ativa, o próximo load pode restaurá-la no
-- Link humano antes de qualquer hotkey. O host publica save.loaded somente
-- quando o Player novo já existe, então esta limpeza não corre cedo demais.
if ship.capabilities.has("save.events") then
    ship.events.on("save.loaded", function()
        if not transformed and not transforming and not removeAfterMaskOff
            and ship.capabilities.has("oot.player.mask") then
            maskResetPending = true
        end
    end)
end

ship.events.on("game.frame", function()
    if maskResetPending and ship.oot.player.set_mask("none") then
        maskResetPending = false
        ship.log.info("máscara residual da forma Goron removida após carregar o save")
    end
    if not transformed or not ship.capabilities.has("oot.player.custom_body") then
        return
    end
    if surprisedTimer > 0 then
        surprisedTimer = surprisedTimer - 1
        if surprisedTimer == 0 then
            reset_eyes()
        end
        return
    end
    if eyesResetPending and not reset_eyes() then
        return
    end
    blinkTimer = blinkTimer + 1
    local nextStep = 0
    if blinkTimer >= 150 and blinkTimer < 153 then
        nextStep = blinkTimer - 149
    elseif blinkTimer >= 153 then
        blinkTimer = 0
    end
    if nextStep ~= blinkStep then
        blinkStep = nextStep
        ship.oot.player.set_body_segment(8, EYES[blinkStep + 1])
    end
end)

-- O hook vem de Health_ChangeBy: valor negativo significa dano já aplicado
-- (depois de double defense/modificadores). A textura surprised é o quarto
-- estado específico do Goron no Player do MM; fica 20 frames antes de o
-- ciclo normal de piscar reassumir.
ship.events.on("hook.oot.player.health_change", function(payload)
    if transformed and payload.amount < 0 and ship.capabilities.has("oot.player.custom_body") then
        surprisedTimer = 20
        blinkTimer = 0
        eyesResetPending = not ship.oot.player.set_body_segment(8, SURPRISED_EYES)
    end
end)

ship.events.on("game.ready", function()
    local ok = ship.capabilities.has("oot.player.immunity")
        and ship.capabilities.has("oot.player.weight")
        and ship.capabilities.has("oot.player.roll")
    if not ok then
        ship.log.warn("host sem as primitivas de habilidade da forma Goron")
        return
    end


-- ---------------------------------------------------------------------------
-- Rampa de luz e fog da transformação.
--
-- O que se costuma chamar de "efeito de partículas" na troca de máscara de MM
-- é, no decomp, esta rampa mais uma luz pontual no Link (func_808550D0 de
-- z_player.c). Não há partícula nenhuma.
--
-- Números tirados de D_8085D848 e D_8085D910 (mm/src/audio/../z_player.c):
--   início no frame 16, avanço 0.10/frame até o estágio 1
--   estouro de luz no frame 59 (onde o MM toca NA_SE_EV_LIGHTNING_HARD)
--   três estágios de {fogNear, fogColor, ambientColor}
-- ---------------------------------------------------------------------------

-- Camera, fog, luz pontual e flash agora pertencem a uma unica maquina no
-- host. Isso garante cleanup atomico em cancelamento, morte, agua, troca de
-- cena e unload; o Lua conserva apenas o callback que troca o corpo no pico.


-- ---------------------------------------------------------------------------
-- Trilha sonora da transformação.
--
-- Os tempos vêm do decomp de MM, tabela D_8085D8F0 de z_player.c — não são
-- estimativa. Cada entrada é (frame, id do sfx):
--
--    2  NA_SE_PL_PUT_OUT_ITEM          Link saca a máscara
--    4  NA_SE_IT_SET_TRANSFORM_MASK    a máscara encosta no rosto
--   11  NA_SE_PL_FREEZE_S              o corpo trava
--   20  NA_SE_IT_TRANSFORM_MASK_BROKEN a máscara "quebra" no rosto
--   30  NA_SE_PL_TRANSFORM_VOICE       o grito
--   59  NA_SE_EV_LIGHTNING_HARD        estouro de luz
--
-- O host dispara estes IDs diretamente do Soundfont_0 do mm.o2r. Os bits
-- baixos do próprio NA_SE_* são o índice real; não existe mais uma tabela Lua
-- vazia nem um caminho silencioso:
-- 2/4/11/20/30 = máscara, freeze, quebra e grito; 59 = raio.

    ship.hotkeys.register("goron_form", { default = "G", label = "Forma Goron" }, function()
        if transforming then
            -- G também permite desistir antes de a troca visual acontecer.
            transformGeneration = transformGeneration + 1
            transforming = false
            if ship.capabilities.has("oot.audio") then
                ship.oot.audio.set_voice_map(nil)
            end
            -- Devolve a câmera junto: sem isto o jogador ficaria preso no
            -- enquadramento até o timer acabar.
            if ship.capabilities.has("oot.cutscene") then
                ship.oot.cutscene.stop()
            end
            if ship.capabilities.has("oot.player.mask") then
                ship.oot.player.set_mask("none")
            end
            ship.log.info("transformação Goron cancelada")
            return
        end
        if removeAfterMaskOff then
            return
        end
        if not transformed then
            removeAfterMaskOff = false
            blinkStep = 0
            blinkTimer = 0
            surprisedTimer = 0
            eyesResetPending = false
            doorActive = false
            chestActive = false
            -- O Player humano fica visível durante gPlayerAnim_cl_setmask;
            -- somente no frame final o corpo externo toma seu lugar.
            local transitionFrames = 0
            local canRunCutscene = ship.capabilities.has("core.timers")
                and ship.capabilities.has("oot.player.mask")
                and ship.capabilities.has("oot.player.custom_body")
            if canRunCutscene then
                -- A entrada sempre parte do Link humano sem máscara. Além do
                -- save.loaded, esta defesa cobre reload do mod e sessões que
                -- foram interrompidas antes do cleanup normal.
                ship.oot.player.set_mask("none")
                maskResetPending = false
            end
            if canRunCutscene and ship.oot.player.set_mask("goron") then
                transitionFrames = ship.oot.player.play_mask_on_animation()
            end
            if type(transitionFrames) == "number" and transitionFrames > 0 then
                transformGeneration = transformGeneration + 1
                local generation = transformGeneration
                transforming = true
                ship.timer.after(transitionFrames, function()
                    if generation ~= transformGeneration or not transforming then
                        return
                    end
                    -- Morte, água, troca de cena e unload cancelam a máquina
                    -- nativa e devolvem a câmera. Um timer antigo nunca pode
                    -- aplicar o corpo numa cena nova.
                    if ship.capabilities.has("oot.cutscene") and not ship.oot.cutscene.is_active() then
                        transforming = false
                        if ship.capabilities.has("oot.player.mask") then
                            ship.oot.player.set_mask("none")
                        end
                        ship.log.warn("transformação Goron cancelada pelo estado do jogo")
                        return
                    end
                    transforming = false
                    transformed = true
                    apply(true)
                    -- Voz do Goron: o Link tem um bloco de sfx de voz a
                    -- partir de 0x6800 e cada forma do MM tem o seu no mesmo
                    -- formato, deslocado. Goron = 0xC0.
                    if ship.capabilities.has("oot.audio") then
                        ship.oot.audio.set_voice_map(0x6800, 0xC0)
                    end
                end)
                ship.log.info("cutscene da máscara Goron iniciada")
                return
            end
            if canRunCutscene then
                if ship.capabilities.has("oot.player.mask") then
                    ship.oot.player.set_mask("none")
                end
                ship.log.warn("cutscene da máscara recusada pelo estado atual")
                return
            end
            transformed = true
            apply(true)
            ship.log.info("forma Goron: corra+A para rolar; R enrola para defesa; B na bola sem espinhos faz ground pound")
        else
            if ship.capabilities.has("core.timers")
                and ship.capabilities.has("oot.player.custom_body")
                and ship.oot.player.play_body_animation("mask_off", "once", 1.0) then
                transformed = false
                removeAfterMaskOff = true
                ship.timer.after(15, function()
                    if removeAfterMaskOff then
                        removeAfterMaskOff = false
                        apply(false)
                    end
                end)
            else
                transformed = false
                doorActive = false
                chestActive = false
                apply(false)
            end
            ship.log.info("forma Goron desfeita")
        end
    end)

    ship.log.info("Forma Goron pronta — aperte G")
end)
