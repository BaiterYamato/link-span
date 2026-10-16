# Handoff — WORLD-002

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- Envelope binário v1 transporta sessão, sequência, origem, destino e estado portátil.
- HMAC-SHA-256 autentica cabeçalho e payload antes da interpretação semântica.
- Comparação do tag é feita em tempo constante e chaves menores que 32 bytes são rejeitadas.
- Sequência monotônica permite ao supervisor rejeitar replay.
- Limites de 4 MiB, 256 itens e 4096 bytes por string restringem alocação e parsing.
- `WriteFile` publica o envelope por `AtomicFile`, sem expor filesystem ou segredo à API Lua.

## Commits

- `05cd347` — feat(world): add authenticated process handoff

## Arquivos alterados

- `rfcs/0006-authenticated-world-handoff.md`
- `include/shiplua/world/WorldHandoff.h`
- `src/world/WorldHandoff.cpp`
- `tests/unit/WorldHandoffTests.cpp`
- `tests/CMakeLists.txt`

## Validação executada

Windows 11, MinGW 15.2/Ninja, Debug:

```powershell
cmake -S . -B build-world002 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-world002
ctest --test-dir build-world002 --output-on-failure
```

Windows 11, Visual Studio 2022 v143, Release x64:

```powershell
cmake -S . -B build-world002-msvc -G "Visual Studio 17 2022" -A x64
cmake --build build-world002-msvc --config Release
ctest --test-dir build-world002-msvc -C Release --output-on-failure
```

O tag determinístico também foi comparado com `hmac.new(..., hashlib.sha256)` do Python 3.14.

## Resultado da validação

- MinGW/Ninja: 28/28 testes passaram.
- MSVC Release x64: 28/28 testes passaram.
- Round-trip, chave incorreta, adulteração, truncamento, replay, chave curta e substituição do arquivo foram cobertos.

## Pendências

- Executar Linux e macOS na CI.
- Implementar o supervisor que provisiona a chave, persiste a maior sequência aceita e inicia o host de destino.
- Integrar o consumidor do envelope aos adaptadores reais OoT/MM.

## Riscos

- O envelope autentica integridade e origem da chave, mas não oferece confidencialidade.
- Replay só é bloqueado quando o chamador fornece e persiste a última sequência aceita.
- Comprometimento do supervisor ou de um host com acesso à chave está fora deste modelo.

## Próxima ação recomendada

Criar o supervisor cross-process e uma conformance OoT → MM → OoT que consuma o mesmo envelope sem alterar o contrato portátil.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: link-span#28 mergeada (13/07); WorldHandoff.* e o RFC 0006 estão na main.
- Core: link-span#28; na main: 05cd347; a branch agent/WORLD-002-authenticated-handoff é ancestral da main.
- Ressalva mantida, não promovida a concluída: supervisor entre processos nunca virou tarefa.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
