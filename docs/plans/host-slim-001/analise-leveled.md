# Port do mod leveled: o que a F2i cobre e o que fica para a F3

Análise estática de 20/10/2026 por subagente Codex (gpt-6.1-sol, esforço alto), somente leitura, contra o host
`e9da99b515` (RC10), o core `50e9e5b` (F1), os exemplos `1a2aa9f` e o PR #11 do leveled (Arrenton/Shipwright, branch
`leveled-blair`, arquivos lidos pela API do GitHub). Nenhum build ou execução; as linhas valem para esses commits.

**A F2i basta como ponto central de transformação; não basta para reproduzir o leveled inteiro nem todos os seus
danos.** O caminho de menor ampliação do host é um pacote Lua + DLL: regras e estado RPG no mod, serviços genéricos no
host. Identidade de instância em Lua, faixa numérica, vida efetivamente consumida e caminhos sem colisão são problemas
distintos. Evidência: [plano-f2-host.md §5.1.1](plano-f2-host.md), `H/soh/include/z64actor.h:79–80` e os patches
detalhados nas seções 1 e 3.

Convenções de evidência:

- `C/` = raiz do core (link-span), HEAD observado `50e9e5bcb6ae1c484aa13560de3f98a5546f640c`.
- `H/` = raiz do host (BaiterYamato/Shipwright), HEAD observado `e9da99b515edbe8e70f3f1fe8f05e342eb1f572b`.
- `E/` = raiz do link-span-examples, HEAD observado `1a2aa9f86f805c62f262523cb173bccd8cfaa576`.
- `P1`, `P2`, `P3` = campo `patch` do registro identificado por `filename` nas três páginas da resposta da API do GitHub
  para os arquivos do PR #11 do leveled (100, 100 e 97 arquivos, nessa ordem). Caminhos de patch são os do fork, **não
  linhas do host atual**. Funções e intervalos `@@` identificam a mudança; para arquivos novos, as linhas adicionadas
  são o próprio fonte.
- `B` = `H/soh/soh/ShipLuaBootstrap.cpp`; `N/` = `H/soh/soh/native/`. As abreviações sempre remetem às raízes acima.

As quatro respostas paginadas da API do GitHub foram lidas com Python: 100 + 100 + 97 + 0 registros, 297 caminhos
distintos. Há 296 campos `patch`; falta o de
`soh/soh/Enhancements/randomizer/3drando/hint_list/hint_list_exclude_overworld.cpp`. O inventário abaixo cobre os
recursos RPG demonstráveis nos patches disponíveis; não atribui todo o diff ao leveled. A wiki de julho fala em “29
arquivos” e registra um estágio anterior das primitivas; não substitui este snapshot
([Research-External-Item-Systems.md:230–245](../../wiki/Research-External-Item-Systems.md)). HEAD identifica a revisão
de referência, sem afirmar árvore limpa ou implementação publicada. A F2i é tratada conforme o **contrato da F2i no
plano** ([plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md)): no fonte lido, a soma continua sem o novo hook
(`H/soh/src/code/z_collision_check.c:3028–3029`), e o evento ainda não aparece no `C/schema/events.yml` lido.

## 1. Inventário do leveled

### Núcleo das regras e integração transversal

| Recurso | Onde o patch altera o jogo | Natureza |
|---|---|---|
| XP acumulado, nível 1–99, curva e teto de XP | `P3:soh/src/code/leveled_stat_math.c`, `sExpTable`, `GetActorStat_NextLevelExp`; `P3:soh/src/code/z_player_lib.c`, `Player_GainExperience`: aplica taxa, limita XP a 999999, sobe vários níveis e recalcula stats. `Player_InitCommon` em `P3:.../ovl_player_actor/z_player.c` reconstrói com ganho zero. | Central, política do mod. |
| Recompensa por espécie, variante, cena e nível | `P3:soh/src/code/leveled_actor_level_table.c`, `Actor_GetLevelAndExperience`, `Leveled_*`; usa `id`, `category`, `params`, estado de atores e exceções como Dark Link acompanhar o jogador. `GetEnemyExperienceReward` em `leveled_stat_math.c`. | Tabela central com regras específicas por ator. Não se reduz a categoria ENEMY. |
| Dificuldade por cena/idade | `P3:soh/src/code/leveled_map_levels.c`, `Leveled_GetSceneLevel`: tabelas de dungeons, bosses, overworld criança/adulto; opção de Castelo de Ganon com 100 tokens. `Actor_GetLevelAndExperience` aplica deslocamento e `Enemy.LevelScale`, limitado a 1–99. | Central, tabela do mod; o retorno 120 da cena termina limitado a 99 nessa aplicação. |
| Power/Courage e equipamento | `P3:soh/src/code/leveled_stat_math.c`, `GetActorStat_{Power,Courage,PlayerPower,PlayerCourage}`, `Leveled_SetPlayerModifiedStats`: espada/túnica/escudo/botas/força. `P3:soh/src/code/z_actor.c`, `Actor_RefreshLeveledStats`; `Player_SetEquipmentData` e `P3:.../z_kaleido_equipment.c` atualizam os modificadores. | Central com integração no equipamento. |
| HP dos inimigos por nível | `GetActorStat_EnemyMaxHealth`, multiplicador de ataque/HP padrão 9 e `Enemy.HPPercent` em `P3:.../leveled_stat_math.c`; `Actor_Init` e ramo de init tardio de `Actor_UpdateAll` em `P3:soh/src/code/z_actor.c` escalam HP depois de `actor->init`. | Central, complementado pelos remendos da próxima tabela. |
| Ataque e defesa RPG | `P3:.../leveled_stat_math.c`, `Leveled_DamageFormula`, `Leveled_DamageFormulaOnPlayer`, `Leveled_DamageModify`: fórmulas diferentes contra Link, modificadores, dispersão aleatória, arredondamento, teto 9999, mínimo 1 se ataque positivo. `P3:soh/src/code/z_collision_check.c`, `CollisionCheck_ApplyDamage`, aplica multiplicador e escolhe Player para AT_TYPE_PLAYER antes da fórmula. | Central; os outros 25 locais de chamada estão abaixo. |
| Dano de knockback com origem | `P3:soh/src/code/z_actor.c`, `func_8002F698`: transforma `arg6` se há ator e dano, antes de preencher `player->knockbackDamage`. | Segundo funil transversal; não é `CollisionCheck_ApplyDamage`. |
| Corações por nível e unidades por coração | `GetPlayerStat_BonusHearts` dá até 10 extras; `GetPlayerStat_GetModifiedHealthCapacity` deriva capacidade efetiva da base e `HeartUnits` (`P3:.../leveled_stat_math.c`). `Actor_RefreshLeveledStats` escreve `healthCapacity2`. `Item_Give`, `Health_ChangeBy`, `Interface_Update` em `P3:soh/src/code/z_parameter.c`, e `Message_Update` em `P3:.../z_message_PAL.c` integram containers, fragmentos, cura e clamp. | Central com repercussões de recuperação/HUD/save. |
| Magia por nível | `GetPlayerStat_MagicUnits` calcula 12 até 72 por barra; `Player_GainExperience` atualiza `magicCapacity = magicLevel * magicUnits`. `Magic_Fill`, `Interface_UpdateMagicBar` em `P3:.../z_parameter.c` deixam de usar só a constante vanilla. | Central; dupla magia pode chegar a 144. |
| Multiplicador de dano recebido e números de Link | `P3:soh/src/code/z_parameter.c`, `Health_ChangeBy`, aplica `Player.DamageMultiplier` depois da defesa, garante dano negativo mínimo e cria número. `P3:.../ovl_player_actor/z_player.c`, `func_80837C0C` usa `func_80837B18_modified(..., false)`; knockback e danos fixos passam a aplicar `DamageMult` explicitamente. | Central; exige preservar ordem e evitar multiplicação dupla. |
| Números flutuantes de dano/XP | `P3:soh/src/code/leveled_overlays.c`, `ActorDamageNumber_New/Draw`, `ActorExperienceNumber_New/Draw`: vida, velocidade, fade, projeção de `focus.pos`. `Actor_ApplyDamage` em `P3:.../z_actor.c` registra dano; `Interface_Draw` em `P3:.../z_parameter.c` percorre inimigos, bosses, NPCs, Leever e Link. Casos de dano manual registram separadamente. | Central de apresentação com emissores por ator. XP flutua sobre Link. |
| Level up, diferenças de stats e fanfarra | `P3:.../leveled_overlays.c`, `ActorLevelUp_New`, `Actor_LevelUpDraw`; `P3:.../z_player_lib.c`, `Player_GainExperience` aciona overlay e `Audio_PlayFanfare(NA_BGM_ITEM_GET)`. | Central de apresentação; não é nova animação esquelética de Link. |
| XP restante, números de HP/magia, stats na pausa | `Leveled_Interface_DrawNextLevel`, `Leveled_KaleidoEquip_Stats`, funções `*ValueNumberDraw` em `P3:.../leveled_overlays.c`; `HealthMeter_Draw/HandleCriticalAlarm` em `P3:.../z_lifemeter.c`; `Interface_DrawMagicBar` em `P3:.../z_parameter.c`; `KaleidoScope_DrawEquipment` em `P3:.../z_kaleido_equipment.c`. | HUD central e integração específica na pausa. |
| Informações da Navi | `P3:soh/src/code/z_message_PAL.c`, `Message_OpenText`; `P2:soh/soh/OTRGlobals.cpp`, `GetLeveledNaviEnemyInfo`, acrescenta nível e HP máximo do alvo. | Integração em mensagens, não apenas texto de overlay. |
| Persistência e nível no seletor de saves | `P1:soh/include/z64save.h` acrescenta `experience`, `heartContainers`, `healthCapacity2`, `magicUnits`, timer; `P2:soh/soh/SaveManager.cpp`, `InitFileNormal/Debug`, loaders base e `SaveBase`, serializa `experience`; `InitMeta` calcula nível. `P2:.../SaveManager.h` acrescenta nível aos metadados; `P3:.../ovl_file_choose/z_file_choose.c`, `FileChoose_DrawFileInfo`, desenha-o. | Central, persistência por arquivo. Não equivale a guardar uma chave global. |
| Opções de UI, gameplay e dificuldade | `P3:soh/soh/SohGui/SohMenuLeveled.cpp`, `SohMenu::AddMenuLeveled`: XP/sons/números/Navi; ataque/HP, bônus de vida/magia, equipamento; unidades de coração, dano recebido, taxas de XP/token, HP e nível dos inimigos. | UI de política do mod. |
| Ampliação de tipos e estado por instância | `P1:soh/include/z64actor.h` muda HP/dano/maximumHealth para `u16` e acrescenta XP, nível, stats, modificadores, sete slots de números e `ignoreExpReward`; `P1:.../z64player.h` amplia knockback; `P1:.../z64save.h` troca magia de `s8` para `u8`. `P1:.../functions.h` altera retorno de `Actor_ApplyDamage`. | Infraestrutura invasiva do fork; não copiar structs RPG para o host. |

### Remendos por ator: dano, fases, sobrevivência e XP

Todos os caminhos desta tabela pertencem a `P3:soh/src/overlays/actors/`. Cada célula identifica o arquivo e a função do
patch; os prefixos `ovl_*` são os diretórios desses arquivos.

| Ator/arquivo | Funções e alteração | Classificação |
|---|---|---|
| `ovl_Boss_Dodongo/z_boss_dodongo.c` | `BossDodongo_Init`, `BossDodongo_Explode`, `BossDodongo_UpdateDamage`: HP próprio, explosão e espada usam fórmula/números. | Dano e HP próprios. |
| `ovl_Boss_Fd/z_boss_fd.c` | `BossFd_CollisionCheck`: decremento manual e piso de HP; `BossFd_UpdateEffects`: projétil/efeito passa dano escalado ao knockback. | Dano manual, fases e dano ao Link. |
| `ovl_Boss_Fd2/z_boss_fd2.c` | `BossFd2_{SetupEmerge,Emerge,Burrow,CollisionCheck}`: limiares 24/18/12/6 escalados, martelo/espada e números. | Dano e fases de Volvagia. |
| `ovl_Boss_Ganon/z_boss_ganon.c` | `BossGanon_UpdateDamage`, `BossGanon_Wait`: dano manual, limiar escalado, restrição de finalização por espada; cutscenes curam até capacidade efetiva. | Fases, finalização e cura. |
| `ovl_Boss_Ganon2/z_boss_ganon2.c` | `func_80900890`, `func_80902524`: HP de fase proporcional, dano manual, preserva requisito do golpe final e limiar 21/30. | Dano e fases. |
| `ovl_Boss_Ganondrof/z_boss_ganondrof.c` | `BossGanondrof_{Neutral,Throw,CollisionCheck}`: limiar, projétil herda nível, dano de espada/retorno de energia, `canKill`. | Dano/fases e origem de projétil. |
| `ovl_Boss_Goma/z_boss_goma.c` | `BossGoma_UpdateHit`: dano calculado por arma, subtração saturada e número. | Dano manual. |
| `ovl_Boss_Mo/z_boss_mo.c` | `BossMo_CoreCollisionCheck`: obtém dano da espada e transforma antes da subtração. | Dano manual. |
| `ovl_Boss_Sst/z_boss_sst.c` | `BossSst_HandCrush`: escala os 8 pontos de `play->damagePlayer`. | Dano direto ao Link. |
| `ovl_Boss_Tw/z_boss_tw.c` | `BossTw_TwinrovaUpdate/ChargeBlast/Damage`: dano manual, limiares escalados, argumento `u16`, subtração saturada. | Dano e fases. |
| `ovl_Boss_Va/z_boss_va.c` | `BossVa_SetupBodyPhase4/BodyPhase4`: `sPhase4HP` passa a `u16`, escala seus limiares, consome `colChkInfo.damage` e emite número. **Não acrescenta chamada a `Leveled_DamageModify`.** | Vida própria que pode consumir o resultado central. |
| `ovl_En_Ba/z_en_ba.c` | `EnBa_Update`, `func_809B75A0`: substitui `health--` por fórmula/número e paga XP ao morrer. | Dano manual e XP. |
| `ovl_En_Bubble/z_en_bubble.c` | `EnBubble_DamagePlayer`: escala dano do toucher e chama `play->damagePlayer`. | Dano direto ao Link. |
| `ovl_En_Clear_Tag/z_en_clear_tag.c` | `EnClearTag_Update`: substitui dano fixo, mostra número, paga XP no fim. | Dano manual e XP do Arwing. |
| `ovl_En_Floormas/z_en_floormas.c` | `EnFloormas_GrabLink`: transforma dano direto; `EnFloormas_SmDecideAction`: paga XP só quando o conjunto terminou. | Agarrão e regra de abate. |
| `ovl_En_Goma/z_en_goma.c` | `EnGoma_UpdateHit/SetupHurt/Hurt`: dano manual da espada e testes de morte compatíveis com HP ampliado. | Dano e sobrevivência da larva. |
| `ovl_En_Ik/z_en_ik.c` | `func_80A75C38`, `func_80A75FA0`: limiares de armadura/HP escalados e dano de 64 contra Link. | Fases e dano direto. |
| `ovl_En_Rd/z_en_rd.c` | `func_80AE3454`: duas chamadas de dano no agarrão; `func_80AE3ECC/80AE4114`: XP ao morrer. | Agarrão e XP. |
| `ovl_En_St/z_en_st.c` | `EnSt_CheckHitLink`: escala o dano direto de 8. | Dano direto ao Link. |
| `ovl_En_Vm/z_en_vm.c` | `EnVm_CheckHealth`: troca `health--` de explosivo por fórmula e `Actor_ApplyDamage`; `EnVm_Die`: XP. | Dano manual e XP. |
| `ovl_En_Fd/z_en_fd.c`, `ovl_En_Fw/z_en_fw.c` | `EnFd_SpawnCore/Draw`: HP compartilhado e cor proporcionais; `EnFw_CheckCollider`: dano extra de `maximumHealth/9`, devolução ao pai, morte por fases. | Saúde composta e apresentação. |
| `ovl_En_Firefly/z_en_firefly.c`, `ovl_En_Bili/z_en_bili.c` | `EnFirefly_UpdateDamage`, `EnBili_UpdateDamage`: condicionam morte/queima a HP restante, tratam efeitos e reação não letal. | Tornar inimigos vanilla de poucos golpes compatíveis com HP escalado. |
| `ovl_En_Crow/z_en_crow.c`, `ovl_En_Dekunuts/z_en_dekunuts.c` | `EnCrow_Respawn/UpdateDamage`, `EnDekunuts_BeDamaged`: restabelecem HP escalado e sobrevivência após golpe. | Respawn e morte não automática. |
| `ovl_En_Dh/z_en_dh.c`, `ovl_En_fHG/z_en_fhg.c` | `EnDh_CollisionCheck`, `EnfHG_Damage`: limiares de HP/fase escalados. | Fases. |
| `ovl_En_Test/z_en_test.c`, `ovl_En_Sw/z_en_sw.c` | `func_80862E6C` restaura HP escalado; `EnTest_Destroy` condiciona XP a `ignoreExpReward`; `func_80B0C9F0` reduz pela metade HP de Skulltula especial sobrevivente. | Ressurreição/abate final e regra especial de HP. |
| `ovl_En_Bb/z_en_bb.c` | `EnBb_Damage/CollisionCheck`: corrige estados de variantes que sobrevivem e paga XP. | Sobrevivência e XP. |
| `ovl_En_Bdfire/z_en_bdfire.c`, `ovl_En_Bigokuta/z_en_bigokuta.c` | `func_809BC598` aumenta dano base 8→16; `EnBigokuta_Init` força identificação na tabela. | Ajuste de balanceamento e identidade. |
| `ovl_En_Dekubaba/z_en_dekubaba.c` | `EnDekubaba_UpdateDamage`: emissor adicional de número. | Apresentação de dano fora do emissor central. |
| `ovl_En_{Bw,Dodongo,Fz,Mb,Peehat,Si,Skb,Tite,Tp,Zf}` / arquivos `z_en_*.c` correspondentes | `func_809D0424/809D0584`, `EnDodongo_SetupDeath`, `EnFz_ApplyDamage`, `EnMb_Stunned/CheckColliding`, `EnPeehat_StateExplode`, `func_80AFB950`, `func_80AFD7B4`, `EnTite_Stunned/CheckDamage`, `EnTp_UpdateDamage`, `EnZf_Die`: emissores de XP adicionais; `EnSi` usa `Leveled_GoldSkulltulaExperience`. | Cobertura de recompensa; token é coleta, não abate comum. |
| `ovl_Bg_Dy_Yoseizo/z_bg_dy_yoseizo.c`, `ovl_En_Elf/z_en_elf.c`, `ovl_En_Bom_Bowl_Pit/z_en_bom_bowl_pit.c`, `ovl_En_GirlA/z_en_girla.c`, `ovl_player_actor/z_player.c` | `BgDyYoseizo_HealPlayer_NoReward/Give_Reward`, `func_80A0329C`, `EnBomBowlPit_Reset`, `EnGirlA_CanBuy_ZoraTunic`, ações de garrafa e recuperação de Link: ajustam cura, enchimento de magia e comparação de vida cheia. | Repercussões do máximo efetivo/unidades de coração. |

O funil central de recompensa do fork é `Enemy_StartFinishingBlow` (`P3:soh/src/code/z_actor.c`), condicionado a
`ignoreExpReward`. A tabela `leveled_actor_level_table.c` marca exceções para grupos, ressurreições e variantes; somar
XP tanto ali quanto em todos os emissores manuais sem essas regras duplica recompensa. O port deve mapear derrotas reais
do host atual, não copiar os emissores do fork literalmente.

Há alterações paralelas ao RPG no PR: versão CMake 9.0.0→9.0.5, controles/menu, randomizer, correções de assets/XML,
copyright da tela inicial, cutscenes e render de fragmentos. Exemplos verificáveis: `P1:CMakeLists.txt`, `P1:README.md`,
`P1:soh/assets/xml/GC_MQ_NTSC_J/objects/object_bv.xml`,
`P2:.../cosmetics/authenticGfxPatches.cpp::PatchBoulderFragment`, `P3:.../ovl_En_Mag/z_en_mag.c`,
`P3:.../ovl_En_Heishi2/z_en_heishi2.c`. Não são dependências demonstradas do sistema RPG.

## 2. Mapa recurso → primitiva do Link-Span

“Lua hoje” abaixo significa schema mais implementação inspecionada, considerando o acréscimo da F2i **previsto no
contrato do plano** quando explicitado. Uma função privada exposta pela DLL via `ship.native.call` continua sendo
solução **nativa**, mesmo que o chamador seja Lua (`C/schema/api.yml:109–110`).

| Recurso | Lua, incluindo F2i | DLL / limite / o que falta |
|---|---|---|
| Curva, nível, fórmulas, taxas, opções | Tabelas e cálculo no mod; menu declarativo `ship.menu.register`, timers e HUD. `C/schema/api.yml:93–108`; `C/schema/capabilities.yml:7–17`. | Não requer RPG no host. Equipamento real e variantes não são automaticamente dados disponíveis a essas fórmulas. |
| XP de abate comum | `hook.oot.enemy.defeat` entrega `actor_id`, categoria e posição. `C/schema/events.yml:59–65`; ponte em `B:7050–7066`. | Não entrega handle, `params`, XP nem nível. Catálogo por espécie/cena é viável; recompensa exata por instância/variante e mortes em grupo pede estado nativo ou identidade Lua ampliada. |
| XP de boss/token e deduplicação | Evento de derrota é de observação, não todo `Actor_Kill`; demo só aceita categoria 5. `C/schema/events.yml:59`; `E/lua/leveling-demo/main.lua:65–66`. | Auditar emissores reais do host para bosses/grupos; coleta de token precisa outro sinal. DLL pode observar estado de save/atores. Não reutilizar um evento de destruição como prova de morte. |
| Persistência ligada ao save | `ship.storage` **não é por save**. `save.loaded.slot` e `ship.game.state().save_slot` permitem separar chaves por slot, apenas como aproximação. `C/schema/api.yml:44–46,55,96–103`; `C/schema/events.yml:27–29`. | **Já existe** `linkspan.oot.save` v1: JSON versionado por namespace, dentro da seção `linkspan` do save; `N/oot_save.h:11–44`. Para Lua puro falta binding desse serviço, incluindo ciclo de vida. DLL pode usá-lo agora. |
| Dificuldade por cena e idade | `scene.enter.scene_id` permite tabela por cena (`C/schema/events.yml:15–16`). | Idade, tokens, `params` e estado específico usados no original não estão no conjunto curado de `ship.player.get`; DLL lê `PlayState`/`SaveContext` e payload de init. `B:5784–5813`; `N/oot_engine.h:19–31`; `N/oot_hooks.h:76–82`. |
| HP inicial de inimigo | Não há `ship.actor.set_health` nem leitura/escrita genérica de ator existente; snapshot contém só handle/id/categoria. `C/schema/api.yml:35–39,64–66`. | **Dá em DLL no `oot.actor.init`**: ator já inicializado, inclusive init tardio, ponteiros válidos durante callback. Escrever `Actor::colChkInfo.health` respeitando `u8`. `N/oot_hooks.h:40–44,76–82`; `H/soh/src/code/z_actor.c:1318–1323,2714–2719`; `N/oot_engine.h:16–18`. |
| HP ampliado/fases/respawn | Nenhuma primitiva Lua correspondente. HP inicial sozinho não corrige morte forçada, restauração literal ou limiar de fase. Evidência: remendos `EnFirefly`, `EnCrow`, `EnFd`, `BossFd2` da seção 1. | HP e dano reais são `u8` (`H/soh/include/z64actor.h:79–80`). DLL precisa HP virtual e adaptação dos consumidores, ou outra primitiva genérica de saúde ampla. Apenas fazer cast/escrever 535 não amplia um campo de 8 bits. |
| Power/Courage no dano central | F2i permite `ship.hooks.result(n)`, com categorias, flags, efeito e dano base; fórmulas por tipo/cena cabem em Lua. [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md). | Fórmula fiel por instância exige identidade/variantes/estado do atacante e alvo. DLL recebe ponteiros e mantém mapa lateral de stats; sem campos RPG novos no Actor. [plano-f2-host.md §5.1.1](plano-f2-host.md). |
| Bônus de corações de Link | `ship.player.set('health_capacity', n)` escreve `gSaveContext.healthCapacity`, faixa 16–320; `health` 0–320. `B:5785–5787,5823–5826,5875–5880`. | Viável dentro de 20 corações, controlando base e bônus sem acumulação. Original admite base 20 + bônus 10 e tem capacidade efetiva separada. Para equivalência falta capacidade efetiva genérica/coerência de cura, HUD e save; DLL pode adaptar esses caminhos, sem inventar `healthCapacity2` no host. |
| Bônus de magia | Lua altera **magia atual**, 0–96, não capacidade/unidades. `B:5787,5827–5828,5881–5883`. | `magicUnits` não existe no host. DLL pode acessar `magicCapacity`, mas o fluxo vanilla recalcula capacidade a partir de 48 por nível e a magia atual é `s8`. `H/soh/src/code/z_parameter.c:3079,3235–3245`; `H/soh/include/z64save.h:299–300,370`. Para 72/144 unidades e recargas corretas é preciso adaptar fluxo e faixa, ou primitiva genérica de capacidade de recurso. |
| Modificadores de equipamento | Não basta `ship.player.set('power', n)`: chave desconhecida é scratch, não estrutura nativa. Equipamento OoT aparece como **planned**. `B:5933–5954,5982–5985`; `C/schema/capabilities.yml:47`. | DLL lê equipamento/força e publica snapshot próprio ao Lua; ou acrescentar leitura genérica de equipamento com capability. |
| HUD, XP, números de Link e level up | Texto/retângulo/anel/ícone em `hook.oot.hud.draw`; Link tem `screen_x/y` calculados por `Actor_GetScreenPos`. `C/schema/api.yml:104–107`; `C/schema/events.yml:70`; `B:5809–5812,5857–5867`. | Overlay e animação 2D de level up são viáveis. Fanfarra não tem API Lua direta nesses schemas; core extension pode chamar `Audio_PlayFanfare`, declarado em `H/soh/include/functions.h:2118`. |
| Números sobre inimigos | Não há projeção de posição/ator arbitrário em Lua, nem posição de instância no payload F2i. `C/schema/api.yml:35–39,64–66,104–107`; [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md). | **Projeção existe no motor**: `Actor_ProjectPos`, `Actor_GetScreenPos`, declaradas em `H/soh/include/functions.h:376,458`; implementação em `H/soh/src/code/z_actor.c:2065–2071`. Core extension ABI 1.4 pode resolvê-las com layout id, desenhar via `N/oot_render.h:94–148` no `oot.play.draw_end`, ou enviar coordenadas privadas ao Lua. Falta API Lua genérica de projeção/identidade, não matemática inexistente no host. |
| Dano real e números corretos | F2i observa/transforma dano **proposto por elemento**, não comprova HP perdido. [plano-f2-host.md §5.1.1](plano-f2-host.md). Mudança de vida de Link é observável, não transformável (`C/schema/events.yml:55–57`). | Mostrar imediatamente o valor da F2i pode anunciar golpe depois rejeitado pelo ator ou contado duas vezes. Para precisão: DLL acompanha consumo/HP ou outro evento genérico pós-aplicação. Chefes com vida própria também precisam emissor específico/adaptação. |
| Pausa, file-select e Navi | Menu/overlay alternativos podem mostrar stats durante gameplay; não há nos três schemas hook de file-select nem transformação de texto da Navi. | Reprodução no local original exige DLL e, onde faltam pontos, core extension. `N/oot_text.h:25–44` troca tabelas fora da exibição, mas não oferece resolução dinâmica por alvo em `Message_OpenText`. `P2:.../OTRGlobals.cpp::GetLeveledNaviEnemyInfo` demonstra o requisito original. |

### Persistência: distinção obrigatória

O bootstrap abre um `KeyValueStorage` em `shiplua-storage.bin` no diretório do app e o conecta ao contexto; não
seleciona arquivo por `gSaveContext.fileNum` (`B:947–965`). O backend compartilhado usa outro arquivo,
`shared-storage.bin` (`B:981–996`), também não o save. O flush local é adiado até aproximadamente cinco segundos e no
shutdown (`B:6840–6858,7221–7229`). Portanto o comentário “write-through: sobrevive a fechar o jogo no mesmo instante”
do demo (`E/lua/leveling-demo/main.lua:43`) não é garantia de persistência imediata.

Chaves `slot.0.xp` isolam slots, mas não acompanham copiar/apagar/recriar saves, rollback ou restauração de backup de
forma transacional; a API Lua expõe carregamento, não eventos equivalentes de saving/copied/deleted
(`C/schema/events.yml:27–29`). O serviço nativo resolve o armazenamento correto: escrita pertence ao save carregado e só
chega ao disco no próximo save; hooks `oot.save.loaded/saving/deleted/copied` estão disponíveis (`N/oot_save.h:11–32`;
`N/oot_hooks.h:19–23,47–50`). **Usar namespace próprio e manter opções globais em `ship.storage`; XP do personagem no
save.** Esta é recomendação de port, não mudança aplicada.

### Vida máxima e amplitude numérica

O HP máximo do HUD atual está em uma extensão lateral de `u8`, inicializada no `GameInteractor::OnActorInit` apenas para
ENEMY (`H/soh/soh/ObjectExtension/ActorMaximumHealth.cpp:6–26`). O `oot.actor.init` vem depois desse callback
(`H/soh/src/code/z_actor.c:1322–1323`): mudar só `colChkInfo.health` em DLL não atualiza automaticamente a barra.
Recalcular o máximo da apresentação ou desenhar barra do mod é parte do port.

Exemplo reproduzível a partir de `P3:soh/src/code/leveled_stat_math.c`,
`GetActorStat_EnemyMaxHealth/GetActorStat_PlayerPower/GetActorStat_Attack`: no nível 99, com padrões, HP base 4 vira
`floor(4 * 9 * (1 + 75 * (0.14 + 75 * 0.0006))) = 535`. Isso excede 255 antes de considerar bosses. A F2i limita **cada
retorno** a 255 e, quando um hook troca o valor, ao que falta para 255 no acumulador `u8` do alvo (decisão tomada no
plano depois desta análise); o coop multiplica depois, e a equivalência de números grandes continua sem garantia
([plano-f2-host.md §5.1.1](plano-f2-host.md); `H/soh/src/code/z_collision_check.c:3028–3033`).

O demo 0.1.0 é uma prova de XP simplificado: 15 por abate da categoria ENEMY, curva linear `20 + level*10`, sem teto 99,
sem dano/HP/HUD RPG, e carregamento no `game.ready` com chaves globais (`E/lua/leveling-demo/main.lua:16–43,51–67`;
`manifest.toml:3`). O texto “100% em Lua” do manifesto não comprova equivalência com o leveled.

## 3. O hook do contrato basta para o dano do leveled?

**Para o mesmo ponto central, sim; para todo o dano e a amplitude do leveled, não.** O posicionamento dentro de
`!AC_HARD`, depois da tabela e antes da soma/coop, corresponde ao patch original. O host atual já calcula `damage` pela
tabela ou pelo toucher menos defesa e ignora `BUMP_NO_DAMAGE`; esse ponto preserva tais decisões
(`H/soh/src/code/z_collision_check.c:3001–3033`; `P3:soh/src/code/z_collision_check.c`, hunk `@@ -3024...`;
[plano-f2-host.md §5.1.1](plano-f2-host.md)). Mover a F2i só para `Actor_ApplyDamage` perderia a origem e não cobriria
consumidores que fazem contas próprias (`H/soh/src/code/z_actor.c:4934–4941`; remendos de bosses da seção 1).

### Comparação entre função, chamadas e payload

| Aspecto | Resultado da comparação | Consequência/proposta, sem alterar o contrato |
|---|---|---|
| Entradas da fórmula | `Leveled_DamageModify` usa categoria do alvo, Power do atacante e Courage do alvo, ambos com modificadores; nível entra indiretamente nos stats. Não recebe tipo de ataque nem consulta nível diretamente. `P3:.../leveled_stat_math.c::Leveled_DamageModify`. | **Não acrescentar `level`, `power` ou `courage` ao host.** DLL consegue resolver stats laterais pelos ponteiros. Lua consegue política por tipo/cena, mas não estado fiel por instância. |
| Tipo de ataque | `dmg_flags`, `damage_effect`, `attacker_is_player` e `base_damage` já constam do contrato. O dano-base da tabela foi resolvido antes do hook. [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md); `H/.../z_collision_check.c:3010–3026`. | Para a fórmula central não falta “tipo de ataque”. Regras de chefes que recalculam espada/retorno continuam no ponto específico de aplicação; flags extras no hook central não as fariam consumi-lo. |
| Identidade Lua | `target_id`/`attacker_id` são `Actor.id`, identidade de **tipo**, sem handle, `params`, posição ou geração. O original diferencia variantes e subatores (`P3:.../leveled_actor_level_table.c::Leveled_*`; [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md)). | Propor posteriormente referências de instância seguras e snapshots (`params`, posição, relações quando necessárias), também nos eventos init/defeat. IDs de tipo não podem indexar estado de dois inimigos iguais. Não expor ponteiro a Lua. |
| Atacante jogador e projéteis | Original troca atacante por `GET_PLAYER(play)` se `info->acHit->atFlags & AT_TYPE_PLAYER`; contrato mantém ponteiro do AT e informa booleano. `P3:.../z_collision_check.c`; [plano-f2-host.md §5.1.1](plano-f2-host.md). | Mod deve normalizar a **origem das stats** para Link quando booleano for verdadeiro. Para outros projéteis, herança/owner é política do mod; Phantom Ganon copia nível ao fogo no patch `BossGanondrof_Throw`. Não usar nível da flecha como nível do jogador. |
| Atacante ausente | Contrato admite `NULL`/id -1; original desreferencia `attackingActor` sem guarda em `Leveled_DamageModify`. [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md); `P3:.../leveled_stat_math.c`. | Definir no mod o que fazer sem origem: manter o dano recebido da cadeia ou stats neutras; não copiar esse pressuposto inseguro do fork. |
| Multiplicadores e cadeia | Original aplica multiplicador ataque/HP (padrão 9) contra não Player e `DamageMult` contra Player **antes** da fórmula; espalhamento aleatório e arredondamento são internos à fórmula. F2i não implementa essas regras e chama nativo antes de Lua. `P3:.../z_collision_check.c`, `P3:.../leveled_stat_math.c`; [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md). | Implementar em um único dono no mod. Definir composição entre `base_damage` e `damage` já transformado para não apagar outro mod nem aplicar a fórmula duas vezes. Não interpretar `DamageMult` como regra já aplicada pela F2i. |
| Faixa de HP/dano | Original usa `u16`, limita fórmula a 9999 e HP até 65535; contrato limita a 255 e host mantém `u8`. `P1:.../z64actor.h`; `P3:.../leveled_stat_math.c`; `H/soh/include/z64actor.h:79–80`; [plano-f2-host.md §5.1.1](plano-f2-host.md). | F3 deve escolher balanceamento reduzido, saúde virtual na DLL ou futura primitiva ampla. Só aumentar o clamp do payload não resolve o armazenamento e os consumidores. |
| Momento do número de dano | Original normalmente emite em `Actor_ApplyDamage` ou após subtração manual; F2i roda por elemento antes da decisão do ator. `P3:.../z_actor.c::Actor_ApplyDamage`, remendos da seção 1; [plano-f2-host.md §5.1.1](plano-f2-host.md). | Propor observação pós-aplicação com identidade e delta efetivo; não chamar cada evento F2i de “golpe consumado”. Multielementos, invulnerabilidade e fases precisam validação. |

A extração dos patches encontra **27 chamadas adicionadas** de `Leveled_DamageModify` em **21 arquivos**: uma em
`z_collision_check.c`, uma no helper de knockback de `z_actor.c` e **25 em 19 arquivos de overlays**. Não inclui a
definição da função. A tabela de atores da seção 1 identifica todos esses 19 arquivos. `BossVa` é adicional e usa o dano
central, sem nova chamada; portanto “vida própria” não significa que nenhum dano passe pela F2i.

### Caminhos fora da cobertura da F2i

| Caminho | Evidência no host atual | O que F3 precisa considerar |
|---|---|---|
| Dano manual ao chefe | King Dodongo subtrai `this->health -= 2` e `-= swordDamage` em `H/soh/src/overlays/actors/ovl_Boss_Dodongo/z_boss_dodongo.c:737,1518`; o patch faz chamadas próprias. | O valor transformado de `colChkInfo.damage` não substitui esses literais. Adaptar consumidor na DLL ou instrumentar futura operação genérica de débito. |
| Vida própria consumindo dano central | Barinade usa `static s8 sPhase4HP`, inicializa em 4, subtrai `colChkInfo.damage` e rearma +3 (`H/.../ovl_Boss_Va/z_boss_va.c:406,1381,1419–1423`). | F2i pode escalar a entrada; faltam amplitude, limiares e estado de fase. Não aplicar fórmula outra vez ao débito. |
| Agarrões e contatos com `play->damagePlayer` | `H/.../ovl_En_Rd/z_en_rd.c:503,537`; `H/.../ovl_En_Floormas/z_en_floormas.c:824`; patch também cobre Bongo Bongo, Shabom e Skulltula. | Precisam de transformação com origem no ponto de dano direto, ou adaptação nativa desses caminhos. Interceptar só a colisão não alcança essas chamadas. |
| Knockback/efeitos com dano explícito | `H/soh/src/code/z_actor.c:2282–2293`, `Actor_SetPlayerKnockback`, apenas copia `arg6`; corresponde ao `func_8002F698` do patch. | Propor hook genérico aqui com alvo, fonte nullable e dano; preservar casos em que o chamador já escalou. Volvagia passa `NULL` ao helper depois de escalar explicitamente no patch. |
| Queda, fogo, perigos e multiplicador final | `Health_ChangeBy` aplica defesa/clamp depois do dano (`H/soh/src/code/z_parameter.c:2895–2956`); Lua só observa `hook.oot.player.health_change` (`C/schema/events.yml:55–57`). Contrato exclui esses caminhos ([plano-f2-host.md §5.1.1](plano-f2-host.md)). | Para o multiplicador geral do leveled, propor transform genérico do delta de vida com motivo/origem quando disponíveis, mantendo fase antes/depois das defesas definida. Emissor pós-mudança não oferece veto/transform. |

Proposta de direção: **manter a F2i no ponto do contrato do plano** e acrescentar separadamente, se necessário, (a)
identidade segura para Lua; (b) operações de saúde/capacidade e notificação de dano efetivo; (c) transformação de dano
direto ao Player com origem. Um hook só em `Health_ChangeBy` cobre uma convergência de vida de Link, mas já não sabe
qual ator agarrou Link; um hook só no helper de knockback não cobre `play->damagePlayer`. Nenhum deles substitui sozinho
as regras de fase e golpe final dos chefes (evidências das duas tabelas anteriores).

Esses acréscimos **não são todos bloqueios absolutos para DLL**: o core documenta `kind = core_extension`, ABI 1.3/1.4,
resolução de símbolos e detours por fingerprint (`C/docs/native-providers.pt-BR.md:114–166`). A DLL pode adaptar funções
hoje sem introduzir RPG no host, mas exige executável exato para desvios, compatibilidade de layout e trabalho
específico por ator. Chamar projeção/fanfarra pública com ABI 1.4 exige layout id; **desviar** essas ou outras funções
continua exigindo fingerprint. Esta análise não validou catálogo de símbolos de um executável ou viabilidade runtime de
cada detour.

## 4. Obrigações para a fase F3 (mods)

Lista em ordem de dependência. São propostas de trabalho futuro; nenhum contrato ou repositório foi alterado por esta
análise.

| Ordem | Classe | Obrigação mínima do port |
|---|---|---|
| 1 | **Dá com a F2i + serviços existentes** | Definir o pacote como Lua + DLL e o recorte de fidelidade. Portar curvas/tabelas/stats/opções para o mod; guardar XP versionado em namespace `linkspan.oot.save`, opções em `ship.storage`; tratar carga/save/cópia/novo arquivo. Reconstruir nível e registrar identidade/estado lateral por ator com limpeza, sem ampliar structs com RPG. `N/oot_save.h:11–44`; `N/oot_hooks.h:19–23,40–44`; patches de regras da seção 1. |
| 2 | **Dá com a F2i** | Aplicar a fórmula apenas no transform central, normalizar AT_TYPE_PLAYER, tratar origem ausente, escolher composição com outros mods e o alcance 0–255. XP por derrotas reais, menu, HUD 2D e bônus de vida até 320 já têm base disponível. Escrever cobertura explícita para bosses/tokens/grupos, em vez de promover o demo a port completo. [plano-f2-host.md §5.1.1 e tarefas 30–32](plano-f2-host.md); `B:5784–5813,7050–7066`; `C/schema/api.yml:108`. |
| 3 | **Precisa de outra primitiva genérica para Lua puro; DLL já supre partes** | Vincular o serviço de save ao Lua; oferecer referência de instância/snapshot e acesso limitado a saúde de atores, equipamento e projeção mundo→tela. Pontos: `C/schema/api.yml/events.yml/capabilities.yml`, bindings em `B`, init/defeat e funções de projeção citadas na seção 2. A DLL pode usar save/init/engine e chamar projeção pública agora, expondo respostas privadas via `ship.native.call`. Não criar API “leveled”. |
| 4 | **Precisa de outra primitiva genérica para equivalência sem detours, ou de adaptação na DLL** | Resolver HP/dano acima de 255, capacidade efetiva de vida/magia, recarga/clamp/HUD e observar dano efetivamente aplicado. Instrumentar operações genéricas em `Actor_ApplyDamage`, fluxo de recursos em `z_parameter.c` e consumidores manuais; só aumentar o payload é insuficiente. Alternativa F3: saúde virtual e apresentação próprias com adaptações de atores, declarando diferenças. `H/soh/include/z64actor.h:79–80`; `H/soh/include/z64save.h:299–300,370`; seção 2, exemplo 535 HP. |
| 5 | **Precisa de outra primitiva genérica para os caminhos excluídos; ou core extension específica** | Cobrir dano direto de `Actor_SetPlayerKnockback`, `play->damagePlayer` e a fase de alteração de vida; adaptar bosses que usam HP próprio, limiares e finalização por arma, inimigos que morrem sem consultar HP e respawns. Se a F3 inicial mantiver isso fora, publicar matriz de exclusões; não chamar o port de equivalente ao leveled. Evidências da seção 3 e `C/docs/native-providers.pt-BR.md:114–166`. |
| 6 | **Não portar** | Campos RPG em `Actor`/`SaveContext`, tabelas/fórmulas/menu específicos dentro do host; mudanças paralelas de randomizer, cutscenes, input, copyright e XML do PR. Não copiar os emissores manuais de XP sem reavaliar os hooks atuais, nem bugs como `GetPlayerStat_NextLevelExpAtLevel` calcular `nextLv` sem retorná-lo. `P1:.../z64actor.h/z64save.h`; `P3:.../leveled_stat_math.c`; evidências de mudanças paralelas da seção 1. |

Aceite proposto para o futuro port: dois saves independentes mais cópia/apagamento/recriação/rollback; XP único em
mortes, respawns e bosses por fase; dois atores do mesmo tipo com variantes diferentes; Link/espada/projétil/origem
nula; múltiplos elementos de collider e coop sem wrap; HP 255/256/535; 20/30 corações; magia simples/dupla 72/144 com
consumo e recarga; danos diretos e golpe final de boss; números iguais ao débito real; desativação do mod preservando
save e capacidades base. Esses casos derivam dos limites e remendos documentados acima, **não foram executados**.

Validação desta entrega: leitura em Python das quatro respostas paginadas da API, inspeção dos schemas e cruzamento com
headers/implementação do host e demo; contagem das chamadas da fórmula e verificação aritmética do exemplo de HP. A
análise produziu apenas este documento. Sem build, testes em jogo, commits ou alteração do contrato. Permanecem incertos
o patch omitido pela API, disponibilidade de símbolos de um binário concreto e o comportamento runtime das adaptações
propostas; isso não altera as diferenças de contrato e tipos comprovadas no fonte.
