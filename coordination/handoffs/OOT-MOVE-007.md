# Handoff — OOT-MOVE-007

## Estado

review — DMR 0.2.12 compilado e instalado em pasta jogável separada. Testes
automatizados e inicialização real conferidos; falta playtest com controle físico.

## Resultado

A retomada da câmera livre usa a vista exibida, descartando o estado do mapa
anterior. Cobre troca de cena, recarga do mesmo mapa, volta de câmera automática,
subcâmera e menus/mira. Distância e ângulos são sincronizados antes de habilitar
FreeLook. Seguir Link após 500 ms e centralizar o stick após menus/mira continuam.

Executável: `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001\build\oot-upstream-001\playable-camera-continuity-20260930\soh.exe`.
Host preservado, SHA-256 `F82782A7B12A4477AB3ADAF0B5130254B1A242F99B0BB0DAACFAEE5A7478A6AC`.

## Commits

- 2026-10-02, por Claude com autorização do usuário: host `62ea72d89` (DMR 0.2.14). Locais, sem push.

Alterações anteriores de outros trabalhos foram preservadas.

## Arquivos alterados

No checkout `OOT-UPSTREAM-78DC-001`: `soh/native-sdk/example/provider.cpp`,
`camera_continuity.h`, `camera_continuity_tests.cpp`, `CMakeLists.txt`,
`manifest.toml`, artefatos em `build/camera-continuity-20260930/` e pasta jogável.
Claim e este handoff; `coordination/STATUS.md` não alterado.

## Validação executada

Build Release, CTest 2/2, validador de manifesto, diff-check e boot real.
O teste usa o UpdateCamera do provider com input controlado, incluindo mudança
de mapa, contador reiniciado, auto/manual, subcâmera, menus e vista inválida.
DLL/pacote usam SDK layout `fa1e78a116ede14c59ee1aa4cb26b4543be13b24518acff4b347427a58d6de9f`
e exigem o fingerprint do host.

## Resultado da validação

Build e testes passaram. O log carregou DMR 0.2.12 e os quatro outros providers
nativos sem erro/critical. Pacotes originais e save mantiveram seus hashes.
O carregador gerou `.shiplua-cache`; a primeira sessão que acompanhava a pasta
inteira identificou essa criação. A sessão final acompanha os arquivos reais.
Registros: `OOT-UPSTREAM-78DC-001/build/camera-continuity-20260930/RESULT.md` e
`runtime-evidence/`.

## Pendências

Conferir com controle físico a retomada após sair da casa de Link, atravessar
outra entrada e retornar de uma cena. O log confirmou ausência de gamepad.

## Riscos

O provider escreve nos tipos nativos do host exato, como as demais funções do
mod. Não usar esse ZIP com outro executável/layout. A prova automatizada não
substitui a sensação de câmera em gameplay físico.

## Próxima ação recomendada

Usar a pasta jogável indicada no playtest de controle e, após resultado positivo,
fechar OOT-MOVE-007. Preservar NEI 0.3.12, HUD recente e o conjunto dos sete
pacotes da pasta jogável.
