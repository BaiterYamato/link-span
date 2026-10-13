# Handoff — RELEASE-ORG-001

## Estado

done — PR #52 integrada e RC9 central publicado em 13/10/2026 às 14:06. Prova de publicação em releases/oot-0.5.0-rc9/publication.json.

## Resultado

- Três repositórios de desenvolvimento e uma entrega pública no link-span, com quatro downloads.
- Definição histórica do RC9 em releases/oot-0.5.0-rc9/release.json: núcleo 4766532, host 3a3e43d328, exemplos 19e53f9, API 0.5.0, Shipwright 9.3.0, layout 7376240d e hashes completos.
- Overlay e SDK originais preservados byte a byte. Mod Examples ZIP (16,46 MiB) com 41 entradas de catálogo; Source (29,59 MiB) reúne fontes dos três commits e registra dependências Git externas.
- 23 exemplos Lua (native-dash tem DLL compilada), 12 demos OoT (incluindo Shovel já validado no RC), mods principais e Actors Demo. MM exclusivo separado; crash-demo em diagnosticos/.
- Script Python monta a entrega com paths explícitos, verifica commits, fingerprints, hashes e conteúdo; ZIPs determinísticos e saída nova publicada atomicamente em disco. Nenhuma operação de publicação no script.
- Script PowerShell constrói demos nativos usando fonte fixada, SDK/layout e executável verificados; native-lock.json associa seus hashes à revisão dos exemplos.
- Página Markdown com quatro botões SVG e prévia HTML local. A publicação antiga no Shipwright permanece como origem histórica.

## Commits

- 3671c56: implementação da organização, arquivos e CI.
- c493841: merge da PR #52 em main; tag central oot-0.5.0-rc9.
- Encerramento e prova de publicação em commit posterior, sem mover a tag ou alterar os pacotes.

## Arquivos alterados

- README.pt-BR.md (somente a seção Organização das entregas).
- docs/agents/Plans.md (somente PLAN e UPDATE desta tarefa).
- docs/releases/README.pt-BR.md; docs/assets/download-{overlay,sdk,examples,source}.svg.
- releases/oot-0.5.0-rc9/release.json.
- tools/release/product_release.py; tools/release/build-native-examples.ps1.
- tests/tools/test_product_release.py.
- coordination/claims/RELEASE-ORG-001.md; este handoff.

## Validação executada

- python -m unittest discover -s tests/tools -p test_product_release.py -v: 15/15.
- Native Windows/MSVC 19.44.35213.0 (mesmo compilador do SDK): 12 builds/pacotes aprovados, manifest validator aplicado.
- Todos os 40 ZIPs/.shipmod do catálogo com manifesto passaram no shiplua_manifest_validator; a entrada restante é o Actors Demo .o2r original.
- product_release.py prepare/verify: quatro downloads, hashes de 9 entradas originais e RESUMO.txt conferidos; núcleo gravado pelo host coincide com a ficha.
- Duas montagens finais independentes: os quatro downloads têm SHA-256 idênticos. Exe do overlay B9046F1A… confirmado.
- BrowserOS neo: quatro links locais conferidos e captura visual da prévia; viewport 1100x850, sem clipping observado.
- Sintaxe Python/PowerShell e git diff --check aprovados.
- Ciclo de tarefa: campos e branch verificados; há sobreposição conservadora de documentação com claims históricas em review (ver abaixo). Não se declarou gate estrutural verde.

## Resultado da validação

Montagem, proveniência, conteúdo, reprodutibilidade e apresentação aprovados localmente. As evidências de gameplay do RC9 são as originais de 12/10/2026; não foi repetida a bateria do host nem afirmado gameplay para os demos adicionais. Playtest físico continua pendente.

## Pendências

- Git e publicação concluídos; restam somente gameplay físico e gameplay dos demos extras, fora desta organização.
- Revisão do integrador dos fragmentos compartilhados de README.pt-BR.md e Plans.md. O gate aponta Plans.md com CEL-004, COREEXT-006/008/009/010, MODSDK-001-MSVC/004-MSVC, OOT-CORE-006, OOT-COREMODS-PLAN-001, OOT-MOVE-004 e OOT-UNBOUND-003B/003C; README com MODSDK-005 e NATIVE-001. README.md foi declarado inicialmente, mas não modificado. Nenhuma claim alheia foi fechada para passar o gate.
- Playtest físico do RC9; testes de gameplay dos exemplos extras conforme seus requisitos.

## Riscos

- Source preserva licenças e avisos existentes; não inclui ROM, saves ou alterações locais. Dependências Git externas são registradas, não vendorizadas.
- ZIP de exemplos é um catálogo, não uma instalação automática de todos os mods. Diagnósticos deliberadamente destrutivos ficam isolados.
- Botões da página pública referenciam assets SVG do main do link-span; integrar os arquivos antes de publicar a página.
- Falhas corrigidas durante a montagem: README inexistente do CEL não é entrada obrigatória; parâmetros do empacotador usam hashtable; escape-demo recebe HostExecutable explícito; native-dash exige build da DLL; código de save em fontes é permitido, arquivos de save são recusados. Cada falha teve causa identificada; SPEC.md do Survival permanece fora do escopo.

## Próxima ação recomendada

Revisar build/central-rc9-v2/preview.html e RELEASE.md. Após autorização, integrar o código/documentação e publicar os quatro arquivos de build/central-rc9-v2 no BaiterYamato/link-span. Preservar a release original do Shipwright e mencionar sua origem.

## Autorização e revisão antes do PR — 2026-10-13

- Mantenedor autorizou commit/push, PR, integração em main após revisão/CI e publicação do RC9 central, preservando o original do Shipwright.
- Revisão do diff: origem dos binários fixada, quatro downloads, fontes por commit, diretório de saída novo, hashes do host/SDK preservados, diagnostics isolados, nenhum fonte de jogo/API alterado.
- Fragmentos compartilhados revisados: quatro botões e destino central nos READMEs, instrução do ZIP completo nos guias de instalação, apenas acréscimos no Plans.md. Ownership desta tarefa autorizado; claims históricas permanecem intactas.
- Correções da revisão: identidade da definição normalizada para JSON UTF-8/LF; rótulos Unicode cobertos por regressão; registro da montagem usa assembly_state=verified; publicação real será conferida e registrada em publication.json após os uploads.
- Escopo de CI declarado na claim: teste Python adicionado ao core-linux e core-windows, sem remover checks anteriores.
- Testes locais atualizados: 16/16. Artefatos finais: build/central-rc9-publish-v2 (quatro ZIPs; originais preservados).
- PlanId: PLN-RELEASE-ORG-001, associado ao plano RELEASE-ORG-001 do ledger.

## Resultado publicado — 2026-10-13

- Release: https://github.com/BaiterYamato/link-span/releases/tag/oot-0.5.0-rc9 (pre-release, não draft).
- PR: https://github.com/BaiterYamato/link-span/pull/52, merge c493841. CI Linux/Windows aprovado no PR e em main.
- Uploads: 11/11 com digest SHA-256 idêntico aos arquivos locais; quatro botões públicos e SVGs conferidos no BrowserOS neo.
- Binários do host e SDK mantêm os hashes do RC9 original; os nove pacotes originais e a data de publicação no Shipwright foram rechecados.
- A página pública usa somente os quatro botões de download no topo. Notas arquivadas nos assets e tag permanecem como snapshots da montagem; não houve alteração de ZIPs após publicação.
- publication.json registra commit da tag, PR, data, URLs e hashes reais dos downloads.
- Próxima etapa de produto: playtest físico ou próximo candidato. A autorização de Git/publicação foi atendida.
