§G

Survival OoT jogável: HUD nativo, estado seguro, bolsa persistente, 4 itens direcionais visíveis & consumo explícito.

§C

- Shipwright OoT primeiro; contratos genéricos sem nomes survival.
- API pública nova → RFC antes de código.
- Mods definem itens, efeitos, balanceamento & slots; host fornece mecanismos.
- ⊥ escrever `SaveContext` padding, slots trade ou garrafas sem transação validada.
- ⊥ dreno em pause, dialog, cutscene, transition, death ou loading.
- Falha de uso → ⊥ remover item.
- D-pad padrão; perfil C opcional preserva/restaura equip vanilla.
- Assets protegidos, ROMs, `.o2r`, saves & binários → ⊥ versionar.
- Build Release Windows + package smoke ! antes de conclusão.

§I

- api: `ship.game.state()` → `{mode:string, save_slot:integer?}`
- api: `ship.hud.draw_icon(path, x, y, w, h, options?)` → boolean
- event: `input.action` → `{action:string, pressed:boolean, source:string, consumed:boolean}`
- capability: `game.state`
- capability: `input.actions`
- capability: `hud.icons`
- mod: `examples/survival-demo/**` → state v1, bag, quickslots, simulation, HUD
- artifact: `x64/packages/oot/Release/mods/survival-demo.shipmod`
- host: `D:\Desenvolvimento\ship-lua-worktrees\shipwright-limpo\Shipwright`

§V

V1: simulation tick ∧ `mode != "gameplay"` → meters unchanged
V2: input action accepted → exactly 1 bag decrement ∧ exactly 1 effect
V3: input action rejected|cancelled|empty → bag unchanged
V4: assigned directional slot → icon + quantity visible at matching HUD button
V5: unassigned directional slot → vanilla input behavior unchanged
V6: mod unload|slot unassign → intercepted input released ∧ vanilla state restored
V7: storage unknown schema → explicit migration/error; ⊥ silent reset
V8: hunger|thirst damage → player health ≥ 4
V9: HUD draw total ≤ host primitive budget ∧ wheel uses native `draw_ring`
V10: public API schema = generated binding = docs
V11: host missing capability → mod fail-closed with actionable log
V12: package contains no protected asset & validates via `shipmod.py`
V13: host build command → configured directory containing `CMakeCache.txt`
V14: native helper call → linked definition exists in target

§T

id|status|task|cites
T1|x|register RFC + claims for state, directional actions & HUD icons|I.api,I.event,I.capability,V10
T2|x|implement schemas, generated contracts & conformance tests|I.api,I.event,I.capability,V2,V3,V5,V6,V10
T3|x|implement Shipwright game-state, directional-input & icon providers|I.host,V1,V4,V5,V6,V11
T4|x|refactor survival demo into state v1, bag, quickslots & safe simulation|I.mod,V1,V2,V3,V7,V8,V11
T5|x|finish wheel/gauge HUD & directional item presentation|I.mod,V4,V9
T6|x|build Release host, package mod & run static/runtime smoke matrix|I.artifact,I.host,V1,V2,V3,V4,V5,V6,V8,V10,V11,V12
T7|x|fix runtime findings & publish verified handoff|V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12

§B

id|date|cause|fix
B1|2026-07-25|used package output `x64` as CMake build dir|V13
B2|2026-07-25|resource width/height declared in header but no linked definition|V14
B3|2026-07-25|HUD text used POLY_OPA and was covered by later OVERLAY primitives|draw text in OVERLAY with the other HUD primitives
