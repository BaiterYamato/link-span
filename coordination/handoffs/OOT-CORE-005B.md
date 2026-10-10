# Handoff — OOT-CORE-005B

## Estado

review

## Resultado

- **Serviço `linkspan.oot.skeletons` v1 (`oot_skeletons.h`)**, publicado no fim da lista (17 serviços):
  - `create` carrega um `SkelAnime` normal ou flex, com as tabelas de juntas no heap do host (fora da arena da
    cena). Esqueleto de curva ou recurso ausente dá `FAILURE`.
  - `play_animation` aceita loop ou once, velocidade e morph.
  - `update` informa quando uma animação once terminou.
  - `get_frame`, `destroy`.
  - `draw` desenha opaco e só no escopo de draw de `linkspan.oot.render` (`InOotRenderScope`).
  - Os caminhos ficam internados porque o `SkelAnime` guarda o ponteiro da animação. O reset do host libera o que
    sobrar.
- **Demo `familiar-demo` 0.1.0 (slice D4):**
  - item `linkspan-demo.keese-whistle`, recebido por `give_item`;
  - o `receive` equipa num C vazio ou com item vanilla, na ordem C-Down, C-Left, C-Right;
  - usar solta um Keese (`object_firefly`) animado em loop, com cilindro AT de espada Kokiri;
  - ao acertar, o Keese troca para animação once mais rápida e some quando ela termina.
- **Slice D5 e Gate D:** framework Unbound, demo do campo, demo de cenas, demo de itens e demo do familiar
  instalados juntos como `.shipmod`, com o mesmo `soh.exe` `6BF3B438…` em todas as sessões. `package-item-demo.ps1`
  e `package-familiar-demo.ps1` aceitam `-Extension shipmod`.
- **Rebuild para o layout `4d74f423`:**
  - DMR 0.2.9 (`D37E8A8E…`) e Mic 0.1.4 (`39F4AB19…`) em `build/distribution`. O Mic não confere layout; o
    rebuild é só do nome.
  - Overlay `LinkSpan-Shipwright-9.2.3-Upstream-78dc6d970-layout-4d74f423-DMR-0.2.9-Mic-0.1.4-Win64-overlay.zip`
    (`977C9266…`).

## Commits

- `OOT-UPSTREAM-78DC-001`: `f4f6ad525`, que inclui também os testes do motor e a documentação do OOT-UNBOUND-004A.
- `link-span`: commit de coordenação deste fechamento.
- Sem push.

## Arquivos alterados

### OOT-UPSTREAM-78DC-001

- `soh/soh/native/oot_skeletons.h`, `OotNativeSkeletons.h`, `OotNativeSkeletons.cpp`, `OotNativeSkeletonsGame.cpp`
- `soh/soh/native/OotNativeEngine.cpp`, `OotNativeView.h`, `OotNativeView.cpp`
- `soh/soh/OTRGlobals.cpp`, `soh/soh/ShipLuaBootstrap.cpp`, `soh/native-sdk/ExportSdk.cmake`
- `soh/native-sdk/tests/EngineTests.cpp`, `docs/native-mods.md`
- `soh/native-sdk/familiar-demo/`, `tools/package-familiar-demo.ps1`, `tools/package-item-demo.ps1`

## Validação executada

```text
cmake --build build/x64 --config Release --target soh oot_native_engine_tests oot_native_scenes_tests soh_symbols -- /m:1
build/x64/Release/oot_native_engine_tests.exe; build/x64/Release/oot_native_scenes_tests.exe
python soh/native-sdk/tests/test_independent.py (probe/host/sdk/example)
cmake --build build/familiar-demo-native, build/item-demo-native, build/dynamic-movement-4d74f423, build/mic-ocarina-4d74f423 (ctest do Mic)
tools/package-familiar-demo.ps1, package-item-demo.ps1 (zip e shipmod), tools/package-linkspan-mod.ps1 (DMR, Mic), validador
tools/package-linkspan-overlay.ps1
Provas em jogo: RELEASE-UNIFICADA/OOT-teste-cenas-20260914, evidência em build/oot-upstream-001/evidence/
  core005b-familiar-ingame, slice-d5-conjunto-ingame, dmr-mic-4d74f423-boot
```

## Resultado da validação

- **Testes do motor:** exit 0. Cobrem:
  - 17 serviços na ordem;
  - ausência de ponte;
  - caminhos vazios, com `__OTR__`, longos e nulos;
  - falha da ponte e `create`;
  - `play_animation` com modo, velocidade, morph e handle inválidos;
  - fim da animação once, `get_frame`, draw fora e dentro do escopo;
  - chamada de outra thread;
  - `destroy` que invalida o handle e `reset` que libera o resto.
- **Prova independente:** exit 0.
- **Slice D4 sozinho** (`core005b-familiar-ingame`):
  - na cena de abertura do título, `recusas=12`;
  - em gameplay, a caixa "You got the Keese Whistle!" e `recebido no C2`;
  - o uso cria o Keese, com `draws=73`, `drawfail=0` e frames da animação avançando;
  - `mosaico.jpg` mostra as asas em poses diferentes.
- **Slice D5, sessão 1:**
  - 5 mods `.shipmod` carregados;
  - o campo viaja para o HF e a demo de cenas para a cópia do HF (cena 128), com o patch aplicado nas duas
    (73 → 74);
  - os dois itens recebidos (C2 e C3) e usados, e o jogo salvo;
  - o Keese não acertou: o levantar do item vira o Link para a câmera, de costas para a árvore.
- **Sessão 2:** recarga com `restaurado no C2` e `restaurado no C3`, sem receber de novo. O toque para baixo virou o
  Link para a câmera, e o Keese voou para o lado errado.
- **Sessão 3:**
  - `hits=1 alvo=0x77 once=1 destroyed=1`: o Keese do item sintético acertou a árvore `En_Wood02` que o JSON do
    Unbound colocou na frente do spawn;
  - evidência em `mosaico-s3.jpg`.
- **Gate D:** todos os slices instalados como `.shipmod`, com o `soh.exe` idêntico, `6BF3B438…`, entre as sessões.
- **DMR e Mic:** os dois carregaram como nativos no host novo, e o gameplay abriu com o DMR `idle`.
- O `file1.sav` da cópia de teste voltou ao original (`1788f5dd…`), e as demos foram para `mods-anteriores`.

## Pendências

- Animação só do esqueleto do mod. Não há animação do Player nem ação do Link ao usar o item.
- Não há override/post limb draw nem posição de limb, e o esqueleto flex não tem desenho translúcido.
- O host não confere se a animação combina com o esqueleto. Usar animação de Link (`LinkAnimationHeader`) num
  esqueleto comum não é suportado.
- O host `03ee0131` que estava na cópia de teste foi sobrescrito sem ir para `host-anteriores`; é reconstruível a
  partir de `83b5b5b42`.

## Riscos

- Esqueletos sobrevivem à troca de cena por design. Um mod que esqueça o `destroy` acumula até 256 até o reset.
- O teste de colisão do familiar com o collider de árvore (`AC_HARD`) só prova o AT contra um alvo que ricocheteia.
  O dano em inimigo não foi exercitado.

## Próxima ação recomendada

- Fase C: OOT-CORE-007 (layouts ampliados) para fechar o `COREEXT-GATE`, e o validador de exports no
  `shipmod.py doctor` (Fase B).
- Depois, as fases E e F.
