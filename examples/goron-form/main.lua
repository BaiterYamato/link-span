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
    reverse_anims = {
        climb_down_l = true,
        climb_down_r = true,
    },
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
local blinkStep = 0
local blinkTimer = 0
local surprisedTimer = 0
local attackActive = false
local landingActive = false
local instrumentActive = false
local doorActive = false
local chestActive = false
local EYES = {
    "mm/objects/object_link_goron/gLinkGoronEyesOpenTex",
    "mm/objects/object_link_goron/gLinkGoronEyesHalfTex",
    "mm/objects/object_link_goron/gLinkGoronEyesClosedTex",
    "mm/objects/object_link_goron/gLinkGoronEyesHalfTex",
}
local SURPRISED_EYES = "mm/objects/object_link_goron/gLinkGoronEyesSurprisedTex"
local GAKKI_NOTE_ANIMS = {
    a = "gakki_play_a",
    l = "gakki_play_l",
    d = "gakki_play_d",
    u = "gakki_play_u",
    r = "gakki_play_r",
}

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
        local okBody = ship.oot.player.set_body(on and GORON_BODY or nil)
        if okBody then
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

    if transformed and payload.instrument and not payload.rolling then
        if not instrumentActive then
            instrumentActive = true
            ship.oot.player.play_body_animation("gakki_start", "once", 1.0)
        end
        -- A nota chega apenas no frame de pressão. Tocá-la como one-shot
        -- preserva toda a pose antes de voltar a `gakkiwait`; escolhê-la só
        -- pelo seletor a trocaria no frame seguinte e cortaria a animação.
        local noteAnimation = GAKKI_NOTE_ANIMS[payload.instrument_note]
        if noteAnimation then
            ship.oot.player.play_body_animation(noteAnimation, "once", 1.0)
        end
        ship.hooks.result("gakki_wait")
        return
    end
    if transformed and instrumentActive then
        instrumentActive = false
        -- `gakkistart` é a mesma sequência usada pelo Player de MM na saída,
        -- só que tocada de trás para frente.
        ship.oot.player.play_body_animation("gakki_start", "reverse_once", 1.0)
        return
    end

    -- Todo ataque começa em A. O bridge replica o buffer nativo de MM: um
    -- novo B durante A/B agenda B/C sem reiniciar a animação em curso. Nos
    -- frames de impacto, a espada invisível vira o quad pesado do punho.
    if transformed and payload.attacking then
        if not attackActive then
            ship.oot.player.play_body_animation("punch_a", "once", 1.0)
        end
        attackActive = true
        return
    end
    attackActive = false

    if transformed and payload.landing and not payload.rolling then
        if not landingActive then
            ship.oot.player.play_body_animation("land", "once", 1.0)
        end
        landingActive = true
        return
    end
    landingActive = false

    if transformed and payload.falling and not payload.rolling then
        ship.hooks.result("fall")
        return
    end

    if transformed and payload.roll_phase == "enter" then
        ship.hooks.result("roll_enter")
    end
end)

ship.events.on("game.frame", function()
    if not transformed or not ship.capabilities.has("oot.player.custom_body") then
        return
    end
    if surprisedTimer > 0 then
        surprisedTimer = surprisedTimer - 1
        if surprisedTimer == 0 then
            blinkStep = 0
            ship.oot.player.set_body_segment(8, EYES[1])
        end
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
        ship.oot.player.set_body_segment(8, SURPRISED_EYES)
        ship.oot.player.play_body_animation("damage", "once", 1.0)
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

local LIGHT_STAGES = {
    { fog_near = 650, fog = {   0,   0,   0 }, ambient = { 10,  0, 30 } },
    { fog_near = 300, fog = { 200, 200, 255 }, ambient = {  0,  0,  0 } },
    { fog_near = 600, fog = {   0,   0,   0 }, ambient = {  0,  0, 200 } },
}

-- Para não-humano o MM usa a mesma luz nos três estágios: ciano forte, raio 100.
local LIGHT_COLOR = { r = 155, g = 255, b = 255 }
local LIGHT_RADIUS_MAX = 100

local RAMP_START = 16   -- frame em que a rampa começa
local RAMP_BURST = 59   -- estouro de luz

local function lerp(a, b, t)
    return math.floor(a + (b - a) * t + 0.5)
end

-- Interpola entre os três estágios conforme a fração 0..1 do trecho.
local function stage_at(t)
    local scaled = t * 2                      -- 0..2 sobre três pontos
    local i = math.min(math.floor(scaled), 1) -- 0 ou 1
    local k = scaled - i
    local a, b = LIGHT_STAGES[i + 1], LIGHT_STAGES[i + 2]
    return {
        fog_near = lerp(a.fog_near, b.fog_near, k),
        fog_color = { lerp(a.fog[1], b.fog[1], k), lerp(a.fog[2], b.fog[2], k), lerp(a.fog[3], b.fog[3], k) },
        ambient_color = { lerp(a.ambient[1], b.ambient[1], k), lerp(a.ambient[2], b.ambient[2], k),
                          lerp(a.ambient[3], b.ambient[3], k) },
    }
end

local function start_transform_lighting(totalFrames)
    if not ship.capabilities.has("oot.env") or not ship.capabilities.has("core.timers") then
        return
    end

    local frame = 0
    local function step()
        frame = frame + 1
        if frame > totalFrames then
            ship.oot.env.clear_light_override()
            if ship.capabilities.has("player.fields") then
                ship.oot.player.set_point_light({ radius = 0 })
            end
            return
        end

        if frame >= RAMP_START then
            local span = math.max(totalFrames - RAMP_START, 1)
            local t = math.min((frame - RAMP_START) / span, 1)
            ship.oot.env.set_light_override(stage_at(t))

            -- A luz pontual cresce junto e dá um pico no estouro, para o
            -- instante da troca ter peso.
            local radius = math.floor(LIGHT_RADIUS_MAX * t)
            if frame >= RAMP_BURST then
                radius = LIGHT_RADIUS_MAX
            end
            ship.oot.player.set_point_light({
                r = LIGHT_COLOR.r, g = LIGHT_COLOR.g, b = LIGHT_COLOR.b,
                radius = radius, offset_y = 20,
            })
        end

        ship.timer.after(1, step)
    end
    ship.timer.after(1, step)
end

    ship.hotkeys.register("goron_form", { default = "G", label = "Forma Goron" }, function()
        if transforming then
            -- G também permite desistir antes de a troca visual acontecer.
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
            attackActive = false
            landingActive = false
            instrumentActive = false
            doorActive = false
            chestActive = false
            -- O Player humano fica visível durante gPlayerAnim_cl_setmask;
            -- somente no frame final o corpo externo toma seu lugar.
            local transitionFrames = 0
            if ship.capabilities.has("core.timers")
                and ship.capabilities.has("oot.player.mask")
                and ship.capabilities.has("oot.player.custom_body")
                and ship.oot.player.set_mask("goron") then
                transitionFrames = ship.oot.player.play_mask_on_animation()
            end
            if type(transitionFrames) == "number" and transitionFrames > 0 then
                -- Câmera dramática, como a troca de máscara de MM: aproxima do
                -- Link enquanto a animação corre e devolve o controle ao fim.
                -- Alguns frames a mais que a animação para o corpo novo
                -- aparecer ainda sob o enquadramento fechado.
                if ship.capabilities.has("oot.cutscene") then
                    -- style="jump" replica Player_Action_86 do MM: em vez de
                    -- orbitar o Link com uma subcâmera, troca o MODO da câmera
                    -- ativa e gira o Link para encará-la. O enquadramento passa
                    -- a ser o nativo do jogo — a órbita destoava do resto.
                    ship.oot.cutscene.start(transitionFrames + 20, { style = "jump" })
                end
                start_transform_lighting(transitionFrames + 20)
                transforming = true
                ship.timer.after(transitionFrames, function()
                    if transforming then
                        transforming = false
                        transformed = true
                        apply(true)
                        -- Voz do Goron: o Link tem um bloco de sfx de voz a
                        -- partir de 0x6800 e cada forma do MM tem o seu no mesmo
                        -- formato, deslocado. Goron = 0xC0. Trocar a voz inteira
                        -- e somar um offset — nao e preciso mapear som por som.
                        -- Zora seria 0xA0, Deku 0x80, Fierce Deity 0x00.
                        if ship.capabilities.has("oot.audio") then
                            ship.oot.audio.set_voice_map(0x6800, 0xC0)
                        end
                    end
                end)
                ship.log.info("colocando máscara Goron")
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
                instrumentActive = false
                removeAfterMaskOff = true
                ship.timer.after(15, function()
                    if removeAfterMaskOff then
                        removeAfterMaskOff = false
                        apply(false)
                    end
                end)
            else
                transformed = false
                attackActive = false
                landingActive = false
                instrumentActive = false
                doorActive = false
                chestActive = false
                apply(false)
            end
            ship.log.info("forma Goron desfeita")
        end
    end)

    ship.log.info("Forma Goron pronta — aperte G")
end)
