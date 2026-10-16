# Handoff — CORE-INTEGRATE-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review

## Resultado

- `origin/main` (64a6f4a) unida à `agent/MENU-001-mod-menu` (4dc1754) no merge `416d227`.
- Conflitos resolvidos com uma regra fixa:
  - Código do runtime e do binding: versão da MENU. Só ela tem ABI 1.2/1.3, escape hatch, boot guard e menu.
  - Schemas, gerados e guias: versão da main.
  - O que só a MENU tinha foi acrescentado depois: `ship.menu.register` e `core.menu` no schema, com `description_en`.
- Erros da API Lua reaplicados em inglês (95 literais), incluindo `hooks.result` e `set_speed_multiplier`. A rejeição da proteção de boot também passou para o inglês.
- `shipmod` da MENU com o doctor de mods nativos em inglês: checks, mensagens e help. Testes ajustados.
- Os guias EN + pt-BR receberam o conteúdo da MENU:
  - `writing-mods`: `[resources] required`, `hooks.result`, velocidade, menu, storage no OoT;
  - `native-providers`: hooks 1.2, escape hatch 1.3, crash/boot, randomizer, contrato do SDK, lote/quarentena;
  - `shipmod-cli`: doctor nativo, com saída real do pacote NEI 0.3.20;
  - `manifest`: `[resources]` e `host_fingerprints`;
  - `native-mods`: ajuste do randomizer v1/v2.
- Links para `CREDITS.md`, que não existe, foram removidos dos READMEs. O `NOTICE.md` cobre código portado.
- Headers `include/shiplua/native` idênticos aos da MENU, então o layout id do SDK OoT não muda.

## Commits

- `416d227` — merge: integrate main into MENU-001 (CORE-INTEGRATE-001)
- commit seguinte — docs: conteúdo da MENU-001 nos guias EN e pt-BR

## Validação executada

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/validate_api_schemas.py
python tools/generate_cpp_api.py --check
python tools/generate_api_contracts.py --check
python tools/generate_api_docs.py
python -m unittest tests.tools.test_shipmod
```

## Resultado da validação

- Windows 11, MSVC v143, Release: ctest 61/61.
- Schemas válidos (API 0.5.0), sem drift nos geradores.
- `test_shipmod` 30/30; links relativos dos .md sem quebra.

## Pendências

- Decidir se a `main` absorve esta branch por PR. Até lá, o host OoT segue esta branch em `extern/ship-lua`.
- `docs/skijer-modapi-adoption.md`, que veio da MENU, segue só em português. É registro de pesquisa.

## Riscos

- O porte dos guias foi feito por Codex (gpt-6.1-sol) a partir dos diffs da MENU. Revisei os fatos: marcadores do doctor, exemplo real, RFC 0025 e versão da ABI. Mesmo assim, o texto não passou por leitura humana.

## Próxima ação recomendada

Apontar `extern/ship-lua` do host OoT para esta branch, compilar o `soh.exe`, confirmar o layout id `9a593a6e` e gerar o SDK bundle.

## Arquivos alterados

- Merge `416d227` (main → MENU-001): claims e handoffs, `docs/wiki`, `docs/api`, `docs/plans`, `docs/architecture`, `generated/docs`, `tools/shipmod.py`, `tools/generate_api_docs.py`, `tools/manifest_validator.cpp`, `tools/mod_test_runner.cpp` e testes em `tests/schema`, `tests/tools` e `tests/unit`.
- `4766532`: `README.md`/`README.pt-BR.md`, `docs/README*`, `docs/manifest*`, `docs/native-mods*`, `docs/native-providers*`, `docs/shipmod-cli*`, `docs/writing-mods*`, `generated/docs/api-reference*`, `generated/lua/shiplua.lua` e este handoff.
- Seção completada em 2026-10-16 pela CLAIMS-CLEANUP-001 a partir do registro acima, do Git e do PR. Fonte: `git diff --name-only 4dc1754 4766532`.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: link-span#48 mergeada (797a13a, 10/10); o extern/ship-lua do host aponta para o 4766532.
- Core: link-span#48; na main: 416d227; a branch agent/CORE-INTEGRATE-001-main-menu é ancestral da main.
- Host: 7764c2f45 d01e63fa0 (RC6–RC9).
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
