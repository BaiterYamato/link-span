# Handoff — STORE-003

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Escrita interna publica arquivos por substituição atômica no mesmo diretório.
- O conteúdo temporário é sincronizado antes do commit.
- No POSIX, o diretório pai também é sincronizado após o `rename`.
- Falhas anteriores ao commit preservam o destino existente e removem o temporário.
- Escritores concorrentes publicam somente payloads completos.
- Nenhum acesso novo ao filesystem foi exposto à API Lua.

## Commits

- `85f5e6d` — feat(storage): add atomic file replacement

## Arquivos alterados

- `include/shiplua/storage/AtomicFile.h`
- `src/storage/AtomicFile.cpp`
- `tests/unit/AtomicFileTests.cpp`
- `tests/CMakeLists.txt`

## Validação executada

Windows 11, MinGW/Ninja:

```powershell
cmake --build build-store003
ctest --test-dir build-store003 --output-on-failure
```

Windows 11, Visual Studio 2022 v143, Release x64:

```powershell
cmake --build build-store003-msvc --config Release
ctest --test-dir build-store003-msvc -C Release --output-on-failure
```

## Resultado da validação

- MinGW/Ninja: 25/25 testes passaram.
- MSVC Release x64: 25/25 testes passaram.

## Pendências

- Executar a matriz em Linux e macOS na CI.
- Consumir `AtomicFile` no envelope autenticado de `WORLD-002`.

## Riscos

- No POSIX, uma falha ao sincronizar o diretório após `rename` informa incerteza de durabilidade, embora o novo arquivo já possa estar visível.
- A garantia cobre publicação íntegra no mesmo filesystem; não cobre coordenação de leitura nem autenticação do payload.

## Próxima ação recomendada

Implementar `WORLD-002` usando `AtomicFile` como etapa final da publicação do envelope autenticado.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: link-span#27 mergeada (13/07).
- Core: link-span#27; na main: 85f5e6d; a branch agent/STORE-003-atomic-write é ancestral da main.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
