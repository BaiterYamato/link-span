> English translation of [coordination/handoffs/EXAMPLE-001.md](EXAMPLE-001.md). The Portuguese document remains the canonical project record.

# Handoff — EXAMPLE-001

## State

done (2026-10-16, CLAIMS-CLEANUP-001; see “Closure 2026-10-16” at the end). Previous record below.

review

## Result

- `examples/hello-world` contains manifest, entrypoint and instructions in pt-BR.
- Target `package_examples` generates `hello-world.shipmod` without versioning the ZIP.
- The generation fixes order, timestamp and permissions and is byte-by-byte reproducible.
- The same packet carries through the `oot` and `mm` contexts, receives `game.ready` and records the host identity.
- `UnloadAll` removes the mod, callback and temporary extraction.
- The `package-examples` workflow recompiles, validates and publishes the `.shipmod` accompanied by the SHA-256 checksum.

## Commits

- `6e42d36` — feat(example): add reproducible hello-world package
- `af004a5` — ci(examples): published validated shipmod artifact
- `134c2c8` — ci(examples): use Node 24 artifact actions

## Changed files

- `CMakeLists.txt`
- `examples/hello-world/manifest.toml`
- `examples/hello-world/main.lua`
- `examples/hello-world/README.md`
- `tests/CMakeLists.txt`
- `tests/conformance/HelloWorldConformanceTests.cpp`
- `tests/conformance/PackageHelloWorld.py`
- `tests/conformance/test_package_hello_world.py`
- `.github/workflows/package-examples.yml`

## Validation performed

```powershell
cmake -S . -B build -G Ninja
cmake --build build -j 2
ctest --test-dir build --output-on-failure
cmake --build build --target package_examples shiplua_manifest_validator hello_world_conformance_tests -j 2
ctest --test-dir build -R "hello_world_(conformance_tests|package_validation|packaging_tests)" --output-on-failure
```

## Validation result

- Windows 11, MinGW g++ 15.2, Ninja, C++20.
- 24 out of 24 tests passed.
- Two independent wrappers produced identical SHA-256.
- The validator accepted the generated `.shipmod`.
- The package's three focal tests passed after including the workflow.
- The local package retained SHA-256 `b6275f720c93129e53dfc0a455f7b6109af45cc94f956a71230384808b16a3ed`.
- GitHub Actions: `package-examples` and `core-linux` passed PR #20.
- The package and checksum were published by `actions/upload-artifact@v7` without the Node.js 20 notice.

## Pending

- Copy the package to the real directories of both hosts and observe the logs with legitimate assets.
- Validate macOS.

## Risks

- The current test uses the real identity contexts, but does not initialize the game executables.

## Recommended next action

After integrating OOT-005 and MM-005, execute the generated package on both hosts and record the log evidence.

## Closure 2026-10-16 (CLAIMS-CLEANUP-001)

- Final state: done.
- Delivery evidence: link-span#20 merged (2026-07-13). Since 2026-10-10 hello-world lives in link-span-examples/lua/hello-world.
- Pending items recorded above remain as history; this closure attests the delivery of the claim scope, not of those items.
- Survey: `claims-review-2026-10-16.csv` in the workspace `logs/claims-cleanup-2026-10`, approved by the user on 2026-10-16.
