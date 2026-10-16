# Handoff — OOT-HOTKEY-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- O Shipwright injeta o registry comum no bootstrap ShipLua.
- Keydowns são despachados no thread principal e a configuração é persistida por mod/hotkey.
- O unload remove registros antes de destruir o estado Lua.

## Commits

- `24ebcc161` — Bridge comum de hotkeys no OoT.

## Arquivos alterados

- `extern/ship-lua`
- `soh/soh/OotHotkeyRegistry.h`
- `soh/soh/OotHotkeyRegistry.cpp`
- `soh/soh/ShipLuaBootstrap.h`
- `soh/soh/ShipLuaBootstrap.cpp`
- `soh/soh/OTRGlobals.cpp`

## Validação executada

```powershell
cmake --build build-msvc-shiplua --config Release --target soh
```

## Resultado da validação

- Windows 11, Visual Studio 2022/v143, Release: `soh.exe` gerado.
- Rebuild incremental concluído em 11,2 s.

## Pendências

- Smoke em jogo com ativos legítimos.
- Builds Linux e macOS.

## Riscos

- A lista inicial de nomes de teclas é intencionalmente limitada e rejeita defaults desconhecidos.

## Próxima ação recomendada

Revisar o PR #10 após integrar HOTKEY-001 e a pilha OOT-001 a OOT-005.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 24ebcc161 (HyliaFoundry) é ancestral do 8f19f1220, topo da pilha OOT-001→OOT-WORLD-002 (conferido pela API de compare). A pilha foi portada para o host atual no 8778337 (18/07, "integrar modloader ShipLua no Shipwright upstream puro"), que está na integração desde o RC2: RC3–RC9.
- Host: 8778337 (port; HyliaFoundry 24ebcc161) (RC3–RC9; já no RC2 local).
- Outras evidências: OotHotkeyRegistry.cpp está na ponta do host.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
