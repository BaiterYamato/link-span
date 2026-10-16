> English translation of [coordination/handoffs/CODEGEN-002-003.md](CODEGEN-002-003.md). The Portuguese document remains the canonical project record.

# Handoff — CODEGEN-002 / CODEGEN-003

## State

done (2026-10-16, CLAIMS-CLEANUP-001; see “Closure 2026-10-16” at the end). Previous record below.

review

## Result

- LuaDoc generated with aliases, classes, payloads, namespaces and signatures.
- Event name alias used by `ship.events.on` for autocomplete.
- Markdown reference in Brazilian Portuguese covers all contracts.
- The two artifacts derive from the three schemas after canonical validation.
- Gate `--check` reports drift of each output separately.

## Commits

- `6a718f6` — `feat(codegen): generate LuaDoc and API docs`

## Changed files

- `tools/generate_api_docs.py`
- `generated/lua/shiplua.lua`
- `generated/docs/api-reference.md`
- `tests/schema/test_generate_api_docs.py`
- `tests/CMakeLists.txt`
- `docs/api/generated-docs.md`
- `coordination/claims/CODEGEN-002-003.md`
- `coordination/handoffs/CODEGEN-002-003.md`
- `coordination/STATUS.md`

## Validation performed

```powershell
rtk python tools/generate_api_docs.py --check
rtk python tests/schema/test_generate_api_docs.py
rtk cmake --build build-verify
rtk ctest --test-dir build-verify --output-on-failure
rtk git diff --check
```

## Validation result

- Windows 11, Python 3.14, MinGW g++ 15.2, Ninja.
- 5/5 Generator Python tests passed.
- Full build completed.
- 19/19 CTest tests passed.
- Synchronized UTF-8 outputs and visible documentation in pt-BR.

## Tests not executed

- Linux and macOS.
- Validation within a real Lua Language Server installation.
- Publication on documentation website.

## Pending

- Package `generated/lua/shiplua.lua` in SDK distribution.
- Connect the signatures to the real Lua binding.
- Set website pipeline in `DOC-001` when referral grows.

## Risks

- The current schema does not have individual function descriptions; LuaDoc exposes
  availability, capability and errors, but not a specific narrative.
- `callback` remains a generic type until schemas model data signatures
  callbacks per event.

## Recommended next action

Implement the minimum binding of `ship.game`, `ship.api`, `ship.runtime`,
`ship.capabilities`, `ship.events` and `ship.log` for the first vertical slice.

## Closure 2026-10-16 (CLAIMS-CLEANUP-001)

- Final state: done.
- Delivery evidence: link-span#12 was closed as part of the stack; integrated by 64a76fa; 6a718f6 is on main.
- Pending items recorded above remain as history; this closure attests the delivery of the claim scope, not of those items.
- Survey: `claims-review-2026-10-16.csv` in the workspace `logs/claims-cleanup-2026-10`, approved by the user on 2026-10-16.
