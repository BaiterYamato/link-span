# Handoff — UNBOUND-016

## Estado

review: formato do Unbound 0.8 no framework (0.5.0).

## Resultado

- **Números obsoletos:** `sceneId` (cena) e `entrances.*.index` (entrada) do `unbound/scenes.json` são ignorados com
  nota (`"<dono>: sceneId é obsoleto e foi ignorado; o jogo numera e tudo usa o nome (SPEC §7)"`). O registro pede
  sempre `LINKSPAN_OOT_SCENES_AUTO` ao host, e o jogo numera na ordem de chave da §3.5. Antes, dois mods que fixavam o
  mesmo número colidiam e o segundo sumia.
- **Saídas:** número entre `ENTR_MAX` (1556) e `0x7FF8` recusa o documento com a mensagem
  `"...: a saída N está no intervalo que o jogo usa para entradas de mod; use o nome \"<cena>/<entrada>\""`. Vanilla
  (0–1555) e as de retorno dinâmico (`0x7FF9`–`0x7FFF`) continuam valendo como número. Os nomes `ENTR_RETURN_*`
  passam a valer e resolvem para `0x7FF9`+i.
- **`sound`:** `seq` fora de 0–109/127 vira 127 (sem música), e `natureAmbience` fora de 0–19 vira 19 (nenhuma),
  cada um com nota. `song` com `seq` 127 ganha nota ("não toca"). O `U8()` antigo embrulhava o 255 dos exports do
  Prelude, e o motor de áudio indexava a tabela de ambiência fora do fim.
- **README** com a seção "Unbound 0.8 (novo na 0.5.0)" e `tools/package-unbound-framework.ps1` em 0.5.0.
- **Faixas de dependência:** as antigas recusariam o framework 0.5.0. Os pacotes que dependem dele passam a
  aceitar `<0.6.0`:
  - Scene Demo 0.1.2 (`>=0.2.0 <0.6.0`);
  - Field Demo 0.1.1 (`>=0.3.0 <0.6.0`);
  - E-fixtures 0.1.1 (`>=0.4.0 <0.6.0`).
- A parte de host do mesmo delta (save por nome, trava no áudio, `SetMeshFactory`, Grave Hole Jumps, combobox e
  `global.sav`) está na UNBOUND-017.

## Commits

Local, no worktree `OOT-UPSTREAM-78DC-001`, sem push:

- `e77f09d17` — UNBOUND-016: formato do Unbound 0.8 no framework (0.5.0)

A parte de host está no `6fa3f2a2a` (UNBOUND-017).

## Arquivos alterados

- soh/native-sdk/unbound-core/transcode.cpp
- soh/native-sdk/unbound-core/scene_registry.cpp
- soh/native-sdk/unbound-core/scene_registry.h
- soh/native-sdk/unbound-core/unbound_core.cpp
- soh/native-sdk/unbound-core/tests.cpp
- soh/native-sdk/unbound-core/format_tests.cpp
- soh/native-sdk/unbound-core/README.md
- tools/package-unbound-framework.ps1
- tools/package-unbound-scene-demo.ps1
- tools/package-unbound-e-fixtures.ps1

## Validação executada

Windows 11, MSVC 19.44 (VS 2022), Release x64:

```powershell
cmake --build build/unbound-scene-native --config Release
ctest --test-dir build/unbound-scene-native -C Release --output-on-failure
```

## Resultado da validação

- `ctest`: 3/3 (json, format, converter).
- `format_tests`:
  - saídas 1556, 1560, `"0x0618"`, 32760, `"0x7FF8"` e 70000 recusadas;
  - 1555, 32767, 32761, `"0x7FF9"`, `"ENTR_RETURN_GROTTO"` (32767) e `"ENTR_RETURN_YOUSEI_IZUMI_YOKO"` (32761)
    aceitas sem nota;
  - `sound` 255/255 vira `<SetSoundSettings Reverb="3" NatureAmbienceId="19" SeqId="127"/>` com duas notas;
  - cenário de três setups (109/19 sem nota; 127 com `song`; -1 e 20), com três notas;
  - número em string (`"255"`/`"20"`) passa pela mesma faixa, com duas notas, e `sound: {}` sai 0/0/0 sem nota.
- `tests` (registro): `sceneId` e `index` anotados como obsoletos, e a cena que antes era recusada por número fixo
  agora registra.
- Base vanilla (conferida contra a conversão local): as 1118 saídas numéricas são estáveis, e nenhum `sound` sai da
  faixa, então nenhuma cena vanilla ganha nota ou é recusada.

- **Revisão independente** (Codex `gpt-5.6-sol`, esforço alto, só leitura, sobre o diff exportado). Seis achados:
  - os testes de limite (`0x7FF8`/`0x7FF9`, hexadecimal) e de `sound` em string ou vazio foram acrescentados acima;
  - o teto do grupo sobre as entradas de retorno era real no host e foi corrigido na UNBOUND-017;
  - a troca de ids num novo `ready` fica: o AUTO do host nunca reaproveita id na sessão, para uma referência velha
    não cair noutra cena (`SceneRegistryTests.cpp`, "novo registro da mesma cena"), e o `ready` roda uma vez por
    `game.ready`;
  - um teste com serviço falso para provar o AUTO não foi feito: o `RegistryScene` não tem mais campo de número;
  - `tools/package-unbound-factory-demo.ps1` segue no framework 0.1.0: a demo das fábricas está fora desde o
    framework 0.4.0.

## Riscos

- Mod antigo que dependia do número fixo (ex.: warp point gravado na config, ou saída numérica para uma cena de
  mod) passa a ser recusado ou a apontar para outra cena. O caminho é o nome, como a SPEC 0.8 manda.
- O teto de grupo da referência é `0x7FF4`. O `oot_scenes.h` segue declarando 32764 porque entra no layout id, e
  o host passou a aplicar o `0x7FF4` no `.cpp` (UNBOUND-017).

## Pendências

- Nenhuma dentro do recorte.

## Próxima ação recomendada

Review conjunta com a UNBOUND-017 (rodada de host do mesmo delta).
