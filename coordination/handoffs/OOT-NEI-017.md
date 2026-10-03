# Handoff — OOT-NEI-017

## Estado

review

## Resultado

- NEI 0.3.13 implementado e instalado em runtime isolado com o host da versão Ocarina/Mic atual.
- Slate, Rod of Seasons e Phantom Hourglass registrados como itens estendidos normais; seleção de 20 habilidades/modos pela grade em inglês.
- Troca original de tipos da Cane, modos da Wand em saves comuns sem alterar medalhões e persistência dos pixels da Pictograph.
- Chamadas de atualização de quatro itens restauradas; corrigido crash da bomba em recarga da mesma cena e transparência do rastro do Hourglass.
- Relatório: OOT-UPSTREAM-78DC-001/build/nei-content-20260930/RESULT.md.
- Jogável: OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-content-20260930/soh.exe.

## Commits

- 2026-10-02, por Claude com autorização do usuário: host `a76045848` (randomizer v2 e hooks), `d6ac301e4` (NEI 0.3.16) e `62ea72d89` (controles do DMR); core `4a75770` (RFC renumerada para 0026). Locais, sem push.


## Arquivos alterados

- Arquivos do nei-core, novas pontes item_mapping/progression, três patches 0007/0008/0009 e tools/package-nei-core.ps1. Lista individual no claim OOT-NEI-017.
- Build, pacote, resultado e evidências em build/nei-content-20260930; runtime novo em playable-nei-content-20260930.
- Claim MENU-001 recebeu apenas a transferência de ownership de fork_glue.cpp e package-nei-core.ps1 durante a abertura desta tarefa; estado de review anterior preservado.

## Validação executada

```text
cmake --build build/nei-content-20260930/native --config Release --parallel 2
ctest --test-dir build/nei-content-20260930/native -C Release --output-on-failure
tools/package-nei-core.ps1 (validador de manifestos do host)
finish-runtime-session.ps1 -Strict -RequireTrackedUnchanged (sessões 004/006/007/008)
Concessão/remoção na UI final, equipar no inventário, save normal/reload,
Slate/bomba/recarga de cena, modelo/seletor de estações, mira/Recall do Hourglass.
```

## Resultado da validação

- Build e três testes passaram; pacote validado; hash final 6D6E3676CA02DA2F4A11F02C673C676BCE56F33564E15445860D92B41EE01C0D.
- A sessão 005 reproduziu crash em RemoteBomb_Tick (RVA anterior 0x7DFDE); a sessão 006 reproduziu a recarga sem crash após correção.
- Sessão 007 executou Recall; sessão 008 final carregou seis mods, 26 itens NEI, zero erros e nenhum probe.
- Sessões estritas válidas, sem alteração de host/pacotes/arquivos monitorados durante os respectivos testes.
- Save final restaurado à cópia inicial; hash D7C09D62D5196C16BE7888E958E24397080F0A9040E68B5DAF01F2A16C03FF72 coincide com o original.

## Pendências

- Sensor, adaptadores de água/areia/beans das estações, clones, Kite Surf e garrafas avançadas.
- Playtest físico e matriz completa das habilidades/Recall em outros alvos; captura real da Pictograph.
- Pendências de outros mods permanecem nos handoffs/relatórios correspondentes.

## Riscos

- Parte do playtest usa um probe privado de frame de botão; removido antes da sessão final. Isso não equivale a prova de controle físico.
- A grade concede habilidades com as rotinas do fork, mas não certifica todos os alvos/cenas. Winter herda conflito de precipitação com o ator de fadas em certas cenas.
- Assets locais do fork seguem fora do pacote distribuível. Runtime depende do executável/layout fixados.
- Checkout já possuía alterações de outras tarefas; nenhuma foi revertida. Sem commit, push, merge ou edição de coordination/STATUS.md.

## Próxima ação recomendada

- Revisar este recorte e testar as habilidades com controle físico; tratar adaptadores de cena/Sensor como tarefas próprias.

## Continuação — 01/10/2026 — NEI 0.3.15 / DMR 0.2.14

- O resultado anterior permanece como histórico da 0.3.13. Sensor/estações/clones/Kite Surf/garrafas agora têm backends compilados; não permanecem como stubs. Consulta real do randomizer v2 e hooks genéricos actor.init/play.draw_end adicionados ao host, preservando os prefixos dos contratos.
- Nova pasta: `OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-completion-20260930/soh.exe`.
- Relatório atualizado: `OOT-UPSTREAM-78DC-001/build/nei-completion-20260930/RESULT.md`.
- Modificações adicionais de controles: helper de ambos os analógicos/D-pad, captura anterior ao update de Link durante o menu R, distinção de A toque/hold de 400 ms e navegação das rodas sem DPadOnPause. Ownership dos arquivos DMR transferido de OOT-MIC-004; câmera e ocarina preservadas.
- Testes atuais: NEI 5/5, DMR 3/3 e host engine com saída 0. Clones/ataque/limpeza observados na sessão 006; poção Bottomless real nas 008/009; Kite Surf 0→1→2→0 nas 013b/015. A segurado + D-pad selecionaram Tornado Rod; toque A + analógico esquerdo + confirmação equiparam C-down e salvaram na 015.
- Sessão 016 final: seis mods, pacote de produção exato, nenhum probe, 26 itens NEI registrados, save comum aberto, zero erros/crashes. Validador da pasta e sessão estrita aprovaram hashes/layout/arquivos monitorados.
- Save, configuração, ImGui e storage restaurados. Save SHA-256 D7C09D62D5196C16BE7888E958E24397080F0A9040E68B5DAF01F2A16C03FF72.
- Host SHA-256 42F8632466CE3A3E703A66BA519BEB2FA3F53C2ADBEA8D6A5686B42D1E5FEB5D; layout 9a593a6e. NEI pacote 9F468F4FAAEFFFE76C53A55E7A7AE0D5A7F193EC2BC1C9E390523FE3CF9EFD8A; DMR pacote 5154CC75567F3BE8A98ED90AE202F274C656E29A97EE85CB96379D37EDACB800.
- Gates ainda abertos: controle físico, Sensor em seed/save/reload, matriz por estação/cena, formação visual dos clones, asset/ícone local faltante e gates físicos prévios dos demais mods. Review permanece; não declarar todo o projeto done.
- Probe de input e grant temporário foram usados apenas em cópia isolada. Não houve commit, push, merge, alteração de STATUS.md nem modificação da sessão do usuário.
- Gate de coordenação `test-task-lifecycle -Phase review -Strict`: valid=false por sobreposição com 14 claims anteriores em review nos arquivos compartilhados. Detalhes em `build/nei-completion-20260930/lifecycle-review.json`; nenhum claim alheio foi fechado. O validador da pasta e o finish de runtime passaram separadamente.

## Roc's Cape e gelo - 01/10/2026 - NEI 0.3.16

- Recorte concluído e instalado na pasta `OOT-UPSTREAM-78DC-001/build/oot-upstream-001/playable-nei-completion-20260930`. Handoff geral continua em review pelos gates anteriores.
- Segundo salto do Cape agora agenda a pirueta OoT mesmo com animações MM desativadas; primeiro salto/limite de dois saltos preservados.
- Winter suprime as três ondulações de entrada (life=0/4/8) junto aos pés sobre a superfície congelada, inclusive WATER_TOUCH antes do pin. Água normal/natação seguem o caminho original. Desvio de EffectSs_Spawn instalado/removido pelo próprio NEI.
- Build Release e seis testes passaram. Sessão 002: pirueta frame14/14 em voo. Sessão 007: rastro branco desapareceu no gelo e diagnóstico confirmou as três ondas. Sessão 008 com DLL final: deslocamento sobre gelo e natação ao desligar Winter. Input automatizado interno não equivale a controle físico certificado.
- Sessão limpa 009: seis mods, 26 itens NEI, zero erros/crashes, nenhum probe; pacote e símbolos inalterados. Validador da pasta e log: valid=true. Evidências de encerramento 008/009: valid=true.
- Pacote final SHA-256 `a2aed5eee8a1f1f72655cda7eb9c4dde42a488d4925f5d16d20390cde206753d`; DLL `af659981ea3cc90a9c2d4e16ff7c90f14c102b39a4c8fd861e44a696680e6ed1`; host/layout preservados. Probes, fixtures Winter e diagnóstico não instalados.
- `build/nei-motion-visuals-20261001/evidence/installation.json` comprova preservação dos saves/configuração/storage/ImGui e outros cinco mods pelos hashes atuais de antes/depois. Pacote 0.3.15 guardado em backup.
- Relatório: `OOT-UPSTREAM-78DC-001/build/nei-motion-visuals-20261001/RESULT.md`. Sem commit, push, merge ou alteração de STATUS.md. Controle físico e matriz completa de estações/cenas continuam pendentes.