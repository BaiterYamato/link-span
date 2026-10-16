# Handoff — OOT-WORLD-002

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- O adaptador OoT sonda Kokiri Sword, Master Sword e Biggoron Sword no archive ativo.
- Cada referência só resolve após `HasFile`, validação da versão do archive e carga real pelo `ResourceManager`.
- O adaptador deixou de aceitar qualquer string com prefixo `oot.`.

## Commits

- `8f19f1220` — feat(oot): validate native world assets

## Arquivos alterados

- `soh/soh/OotWorldAdapter.h`
- `soh/soh/OotWorldAdapter.cpp`
- `extern/ship-lua`

## Validação executada

```powershell
cmake -S . -B build-msvc-assets2 -G "Visual Studio 17 2022" -A x64
cmake --build build-msvc-assets2 --config Release --target soh -- /m:1
```

## Resultado da validação

- Windows 11, MSVC 19.44, Release: `soh.exe` gerado com sucesso.
- PR: https://github.com/BaiterYamato/Shipwright-HyliaFoundry/pull/12

## Pendências

- Confirmar em runtime, com `oot.o2r` legítimo, que os três paths permanecem válidos.
- Linux e macOS não foram executados localmente.

## Riscos

- Um path removido pelo host fica indisponível e registra warning; não há fallback inseguro.

## Próxima ação recomendada

Integrar após WORLD-004 e executar o mod de conformidade de troca de mundo.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: Host 8f19f1220 (HyliaFoundry) é ancestral do 8f19f1220, topo da pilha OOT-001→OOT-WORLD-002 (conferido pela API de compare). A pilha foi portada para o host atual no 8778337 (18/07, "integrar modloader ShipLua no Shipwright upstream puro"), que está na integração desde o RC2: RC3–RC9.
- Host: 8778337 (port; HyliaFoundry 8f19f1220) (RC3–RC9; já no RC2 local).
- Ressalva mantida, não promovida a concluída: runtime com oot.o2r legítimo não registrado.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
