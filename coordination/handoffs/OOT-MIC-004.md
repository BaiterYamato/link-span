# Handoff — OOT-MIC-004

## Estado

review — interface implementada, build/pacotes e cinco testes passaram. Partitura,
songbook e abertura do instrumento conferidos no jogo. Captura real recusada pelo
Windows; execução de notas e músicas com controle físico ainda não validada.

## Resultado

Mic Ocarina 0.2.0 inclui partitura em pergaminho, clave, glifos L/R/Y/X/A,
confirmação da música e songbook das músicas aprendidas. A tela de áudio inclui
badge do microfone, volume real, nota/frequência e histórico de afinação, com
prática e reset. Os textos de interface estão em inglês; erros localizados do
driver ficam no log e recebem mensagem em inglês no painel.

O desenho nativo de músicas continua executando a lógica de notas, reconhecimento,
timers e diálogos. Só sua saída gráfica é substituída durante a ocarina. A leitura
dos botões físicos evita herdar o cancelamento N64 B ao tocar o novo botão A.

DMR 0.2.13 cede controles à ocarina, preservando a continuidade da câmera 0.2.12.
A abertura do microfone foi movida para a thread de captura; o áudio nativo é
convertido para o formato original do detector. O jogo responde durante a abertura.

Executável: `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001\build\oot-upstream-001\playable-ocarina-remake-20260930\soh.exe`.
O conjunto preserva NEI 0.3.12, Wind Waker, Unbound, Shovel e os dois pacotes de mapas.

## Commits

- 2026-10-02, por Claude com autorização do usuário: host `51c417edc` (Mic) e `62ea72d89` (DMR). Locais, sem push.

Preservadas alterações anteriores do checkout; sem push ou merge.

## Arquivos alterados

`OOT-UPSTREAM-78DC-001/soh/native-sdk/mic-ocarina/`: provider, Lua, CMake,
manifesto, README, remake_ui.h, package_assets.cpp/h, remake_ui_tests.cpp,
hud_tests.cpp/lua e recursos próprios. `soh/native-sdk/example/`: provider.cpp,
manifest.toml e camera_continuity_tests.cpp. Gerador de arte em
`tools/generate-ocarina-hud-assets.ps1`; artefatos em
`build/ocarina-remake-20260930/` e pasta jogável acima.

Claims OOT-MIC-004 e OOT-MOVE-007, este handoff. A propriedade dos três arquivos
compartilhados do DMR foi transferida do review OOT-MOVE-007 para esta integração;
suas pendências físicas e seu histórico permanecem. Nenhum outro agente estava
ativo na árvore ao confirmar a transferência. coordination/STATUS.md não alterado.

## Validação executada

Build Release, CTest Mic 3/3 e DMR 2/2, validador de manifesto, diff-check,
inicialização e inspeção na janela real, sessões com hashes de host, símbolos,
oito pacotes e save. HUD executado pelo teste Lua real com limite de 350 unidades,
reservando margem ao orçamento compartilhado do host.

Regressões cobrem notas físicas, A sem cancelamento herdado, B, teclado,
estado nativo de músicas/diálogos, songbook, prática/reset, abertura lenta/cancelada
e conversão SDL real de PCM estéreo 48 kHz para mono 22.05 kHz sem dispositivo.

## Resultado da validação

Cinco testes passaram. Os seis providers carregaram juntos. Partitura, cabeçalho
sem sombra duplicada, glifos, abertura do instrumento e songbook conferidos.
Host/símbolos/save/pacotes mantiveram seus hashes durante as sessões concluídas.

O pacote final Mic tem SHA-256
`A91446CF9217A54551E756B9FDBB7FF79D24A532FED5C689D6CFCFF60D3DF92D`.
Seu boot com os seis providers foi conferido na sessão 005-final-package;
os testes físicos de microfone e controle permanecem pendentes.

O teste 002 encontrou bloqueio síncrono de oito minutos e erro WASAPI. O 003
experimentou DirectSound em segundo plano; o driver continuou esperando. O 004,
com negociação de formato e WASAPI original em segundo plano, manteve o jogo
respondendo, mas recebeu novamente `WASAPI can't initialize audio client` após
oito minutos. DirectSound não foi mantido no pacote final.

Registros detalhados: `OOT-UPSTREAM-78DC-001/build/ocarina-remake-20260930/RESULT.md`
e `runtime-evidence/`. Não confundir validade dos hashes de sessão com sucesso
de captura. A causa específica da recusa do dispositivo ainda não foi isolada.

## Pendências

Validar abertura de um microfone real, volume/afinação, prática/reset e reconhecimento
de música em gameplay. Resolver a recusa do cliente de áudio observada nesta máquina.
Conferir L/R/Y/X/A, ZL, +, -, B e efeitos de músicas com controle físico. A câmera
de OOT-MOVE-007 continua com seu playtest físico pendente.

## Riscos

Pacote nativo depende do host exato e layout SDK. Testes automatizados não comprovam
dispositivo/controle físicos. Se o driver ficar preso durante a abertura, a interface
do jogo responde, mas o unload deve aguardar a thread para evitar descarregar código
em execução; fechar o aplicativo pode aguardar o driver nesse caso.

## Próxima ação recomendada

Usar a pasta jogável indicada para playtest com o microfone e controle do usuário.
Investigar a recusa de abertura antes de declarar a captura pronta. Não substituir
as pastas jogáveis anteriores nem liberar o gate físico da câmera sem evidência.

## Correção — 01/10/2026 — Mic 0.2.1

- Relato do usuário: Y sai da ocarina. Causa confirmada no código: AudioOcarina_ReadControllerInput já mapeava a nota, mas Message_DrawMain usava o BTN_B de gameplay entregue pelo Y do DMR.
- Correção no provider Mic: override de B somente durante a lógica musical, com cancelamento físico B, supressão na liberação da nota e restauração imediata do input. Sem modificar DMR/NEI/host.
- Build e três testes passaram, incluindo o caminho de cancelamento independente da tela. Sessão mic-y-release-001 carregou os seis mods e Mic 0.2.1 sem erros. Sessão estrita e validador passaram. Controle/microfone físicos ainda não certificados.
- Pacote instalado em `OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-completion-20260930`. SHA-256 D704A4EAFBC6E17AA91C063F47A734F3ECCF84A38113A8C8F3C4EC2DFBA4F33A.
- Evidências em `OOT-UPSTREAM-78DC-001/build/mic-y-cancel-fix-20261001/RESULT.md`. Save e sessão original do usuário preservados. Review permanece pelos gates físicos anteriores.
