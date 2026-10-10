# RFC 0027 — Recompilações do host após COREEXT-GATE (Link-Span OoT)

Status: **aceita pelo mantenedor em 2026-10-09** (rascunho de 2026-10-03). Ver "Decisão do mantenedor" logo abaixo.

## Decisão do mantenedor (2026-10-09)

- A regularização do passado fica aceita como enumerada no apêndice B: as peças B, as correções D e o tooling E são
  exceções históricas registradas. A rodada de host dos patches 1 a 15 (UNBOUND-031) já tinha sido aprovada em
  2026-10-04.
- **Ownership do menu de equipamento (`a76045848`):** sai pela RFC 0028, fase 1, no candidato `oot-0.5.0-rc5`:
  setting transitório genérico `linkspan.pause.equip_selector`, helper `LinkSpan_PauseEquipSelectorEnabled` e
  remoção do símbolo com nome de mod. O lease (fase 2) entra junto da próxima mudança que já altere o layout id.
- **Áudio de cena por caminho (`b13913077`)** e **montagem de camadas declarada por provider
  (`IsUnboundLayerArchive`, `a76045848`):** dívida C aceita. Dono: o mantenedor do Link-Span. Marco de saída: a
  próxima rodada de host que mude o layout id e, no mais tardar, antes de abrir a Fase H (Majora's Mask). Critério de
  saída: o "Aceite" de cada seção abaixo.
- O requisito correspondente do §18 do plano passa a usar a redação proposta em "Regularização do passado".

Escopo: regularização documental proposta dos 24 commits do host entre `16c4622d5` e `fc2709989` que tocam `soh/soh`, `soh/src`, `soh/include`, `libultraship` ou CMake (`git log --stat 16c4622d5..fc2709989 -- soh/soh soh/src soh/include libultraship CMakeLists.txt soh/CMakeLists.txt`), após o baseline solicitado `16c4622d5` (2026-09-18), e contrato para futuras recompilações. Não autoriza publicar, instalar ou alterar hosts. Número 0027 reservado pela tarefa COREEXT-011. Levantamento feito pelo Codex (gpt-6.1-sol) e conferido por amostragem no git (Claude); os achados históricos continuam sujeitos à revisão.

## Motivação

O plano `OOT-COREMODS-001` separa infraestrutura do host de mods independentes. §4 proíbe acrescentar chamada específica ao host depois de `COREEXT-GATE` sem demonstrar insuficiência da Core Extension ABI. §8 oferece serviços, hooks e escape hatch; §15.2 define evolução aditiva e fingerprints; §18 mantém aberto o requisito de recompilar apenas DLLs, salvo mudança formal de ABI em RFC.

Essas formulações não descrevem bem a história observada. O próprio plano registra uma rodada W, uma exceção NEI-HOST-001 escolhida em 25/09 e a reabertura W em 26/09. Houve correções de host, suporte genérico e tooling que recompilaram o executável sem acrescentar callback à ABI C pública. H/OOT-RELEASE-001.md:240 continua marcando a regra como não atendida. Uma RFC de mecanismo não é uma justificativa específica para qualquer mudança posterior.

Há também uma ambiguidade em §4: a linha "Adicionar função compatível ao fim de uma tabela ABI" marca "Não" para recompilar Shipwright. Isso vale quando a implementação/tabela pertence a uma DLL de coremod. Acrescentar um callback implementado no adaptador compilado do host exige recompilar esse adaptador. Esta RFC propõe explicitar a diferença; não apresenta a tabela atual como autorização que ela não contém.

Resultado do levantamento: **A=0, B=6, C=2, D=12, E=4**, por categoria principal. Incrementos A existem dentro de commits mistos, como mostra o apêndice. Os dois commits C contêm três frentes específicas: binding nominal de áudio Unbound, seletor do DMR no kaleido e montagem de camadas Unbound no bootstrap.

## Decisão e contrato propostos

### Fronteira após o gate

Instalar, remover ou atualizar um pacote, item, cena, tradução, modelo, textura, áudio ou regra de um mod **não recompila o host**. Código específico e identificação de formatos do framework ficam na DLL/Lua do provider. O host oferece transporte, dados e pontos de extensão com contratos genéricos.

Uma recompilação é aceitável quando houver uma destas razões documentadas:

| Classe | Condição para recompilar o host | Registro mínimo |
|---|---|---|
| A — incremento compatível | Callback genérico implementado no host; prefixo/size preservados e lacuna identificada. Tabela publicada por DLL continua exigindo apenas rebuild da DLL. | RFC nova ou emenda explícita de contrato; negociação e testes de prefixo. |
| B — serviço/hook/substrato genérico novo | A capacidade precisa do adaptador ou transporte do motor; implementação em coremod/serviços/hooks existentes foi avaliada. Contrato sem nomes, IDs fixos ou políticas de mod. | RFC com nome/versão, fase, payload, dono, lifetime, erros e prova da lacuna. |
| C — dependência específica | Exceção temporária, somente após demonstração e aprovação explícita; não é autorização permanente por esta RFC. Preferir serviço/hook genérico ou escape hatch. | RFC da exceção, alternativas rejeitadas com evidência, responsável pela remoção, prazo ou marco e critério de saída. |
| D — correção do host | Corrida, crash, bootstrap, segurança, persistência ou implementação genérica incorreta. Preserva a ABI C pública; alteração semântica relevante precisa ser descrita. | Handoff e regressão adequada. Emendar a RFC se mudar thread, timing ou lifetime que o contrato promete. |
| E — build/tooling/fingerprint | Upstream/toolchain, exportação de símbolos, retenção de endereços, testes/empacotamento ou cálculo de identidade. Necessidade e impacto reproduzíveis. | Handoff; RFC se alterar contrato/identidade negociada. SDK e pacote coerentes com o executável final. |

Categoria não concede autorização de distribuição nem substitui validação. Correção urgente D/E pode sair separada do lote quando esperar prolongar falha; deve registrar a justificativa e invalidar fingerprints antigos. A mera existência de um consumidor NEI/Unbound/CEL não justifica C, nem torna um serviço genérico específico.

### Demonstração de insuficiência

Antes de novo código específico no host, a tarefa deve apresentar uma fixture mínima e responder:

1. Qual operação o mod precisa realizar e em que fase/thread?
2. Qual serviço/tabela atual foi consultado e qual campo/resultado falta?
3. Por que hooks `observe/transform/replace` da RFC 0021 não bastam? Falta ponto de despacho, payload ou uma política segura de conflito?
4. Por que escape hatch da RFC 0023 não basta? Mostrar símbolo ausente/fundido/inlined, efeito de detour incompatível, layout/lifetime não exposto ou custo medido. "É mais fácil no host" não é demonstração.
5. Pode-se ampliar genericamente um contrato existente? Apresentar contrato reutilizável e consumidor de fixture sem nome/regra do mod.
6. Qual alternativa mantém o comportamento sem mod e remove tudo no unload?

Nem toda correção D exige redemonstrar a ABI; o requisito é proporcional ao contrato afetado. Evidência de impossibilidade do escape hatch não permite incluir toda a lógica do fork no host: pode justificar tooling E para habilitar o próprio escape hatch, como NEI-004A/NEI-HOST-001.

### Regularização do passado

Propõe-se reconhecer as peças genéricas B, correções D e tooling E do apêndice como exceções históricas enumeradas, condicionadas à revisão de contratos e evidências de cada rodada. Isso corrige a promessa absoluta de "só DLLs" sem apagar o que ocorreu.

- **OOT-CORE-008 / E+F:** registrar formalmente items/resources V3, scenes V2, text V1 e fábricas genéricas. A chamada nominal de áudio fica como dívida C com saída definida abaixo. H/OOT-CORE-008.md comprova motivação funcional, não uma demonstração prévia completa de insuficiência.
- **Rodada W inicial:** reconhecer CEL-002/CEL-003 e host UNBOUND-013/014/015, já enumerados no plano. Registrar versões de render/scenes, hooks e mudança estrutural de scroll; separar o layout intermediário `d491fbf7` do final `ce528052`.
- **NEI-HOST-001:** manter a exceção registrada de uma rodada antes das ondas para preservar endereços de 314 funções. O diff de host altera flags; o fork continua na DLL. A decisão é reportada pelo plano/handoff, não uma aprovação obtida nesta auditoria.
- **NEI-HOST-002/003:** registrar randomizer V1 e Anchor como contratos genéricos de itens/namespaces. Handoffs registram a decisão de suportá-los no modloader. RFC específica contemporânea de cada V1: **não confirmado**; a RFC 0026 posterior não cobre automaticamente Anchor/ofertas V1.
- **Reabertura W de 26/09:** reconhecer COREEXT-010, MENU-001 e CEL-004A. RFCs 0023 e 0025 estão vinculadas às respectivas tarefas. Render V3/lights/fairy/point_color precisam de descrição contratual específica; a RFC 0021, sozinha, é apenas mecanismo.
- **Consolidação de 02/10 e RC:** RFC 0026 cobre consultas randomizer V2/init/draw_end. Acrescentar contrato de registry V2/actor-models e atualizar menu/manifesto quando necessário. Não usar essa RFC para regularizar implicitamente o DMR ou montagem Unbound.

Os itens C só podem ser reconhecidos como **dívida histórica explicitamente aceita**, se os responsáveis aprovarem o desvio e sua saída; a demonstração anterior continua **não confirmado**. Aprovar este rascunho retrospectivamente não prova conformidade anterior ao §4. Sua aprovação também não resolve gameplay, distribuição/licenças ou outros critérios do Gate OoT.

Proposta de redação para substituir somente o requisito correspondente de §18, após revisão:

> Instalação e atualização de coremods não exigem recompilar o host. Recompilações de infraestrutura, correção ou tooling após COREEXT-GATE estão enumeradas em RFC/handoff com versão e evidência; chamadas específicas exigem demonstração anterior de insuficiência e exceção aprovada ou foram generalizadas conforme a RFC 0027. O inventário histórico tem destino explícito para cada dívida.

Esta RFC não altera o plano.

## Capacidades

Esta RFC define governança e **não anuncia uma capability nova já implementada**. Os serviços/hook genéricos existentes do inventário continuam negociados por nome/versão/tamanho. Conforme RFC 0015, a implementação consultada usa `get_service` com **nome/versão exatos e minimum_size**, não uma faixa SemVer automática.

Para fechar C, propõem-se os contratos abaixo. Nomes/versões são propostas sujeitas a revisão, não capacidades observadas no host atual.

### Áudio de cena por caminho — b13913077

**Fato:** `SetSoundSettingsFactoryXML::ReadResource` passa a chamar `SOH::Unbound::SequenceIdForPath(song)` e preencher campos `unboundSongPath/unboundSongSeqId`. `UnboundAudio.h` e esses campos já existiam em `16c4622d5`; não houve criação desse helper nesse commit. Fonte: G(b13913077) e blobs do baseline descritos no apêndice.

**Justificativa possível, ainda sem demonstração formal anterior:** sequências têm IDs atribuídos em runtime; uma cena referida por caminho precisa resolver a sequência montada. O objetivo é de áudio genérico, embora o nome do helper e dos campos seja do Unbound.

**Saída proposta:** encapsular a resolução de sequência/binding de tema sob API interna genérica e, se o provider precisar da operação, sob serviço `linkspan.oot.audio` versionado. O factory XML trata `Song` como caminho genérico, sem parser/schema Unbound. Tradução JSON e regras do formato ficam no framework. Renomear campos internos quando possível; preservar um adaptador privado temporário se outro código do substrato já os usa. Não apresentar essa adaptação C++ como callback estável da ABI C.

**Aceite:** XML gerado por fixture independente do Unbound toca a sequência correta; caminho ausente não acessa memória inválida; trocar/desativar o provider/cena não mantém tema indevido; sem `Song` o loader vanilla conserva o comportamento.

### Ownership do menu de equipamento — a76045848

**Fato:** `LinkSpan_DynamicMovementInventoryMenuEnabled` retorna `dpadHudOwned`; `z_kaleido_item.c` implementa escolha de C por A/analógico/D-pad e chama o helper. O handoff de release reconhece explicitamente a dívida. Fonte: G(a76045848), H/OOT-RELEASE-001.md:227,266-267.

**Justificativa possível:** o mod assume controles e HUD, então o pause menu precisa equipar itens sem os gestos vanilla conflitantes. Prova de que hooks/escape hatch não poderiam fazê-lo: **não confirmado**. A seleção demonstrada em handoff é evidência funcional, não prova arquitetural de insuficiência.

**Saída proposta:** um contrato de inventário/input genérico, por exemplo `linkspan.oot.inventory` com lease de ownership e configuração explícita de seleção/equipamento. Separar "possui HUD do D-pad" de "pede seletor de equipamento": o flag existente não deve ligar uma segunda função implicitamente. Um helper interno `LinkSpan_InventoryEquipMenuEnabled` pode consultar a política genérica; só trocar o nome do helper sem definir ownership/timing não encerra a dívida.

- Dono obtido do contexto do provider, sem aceitar owner arbitrário como autorização.
- Lease exclusivo quando houver conflito de consumo de input; segundo dono recebe `LIMIT`.
- Desligado por padrão; estados de escolha, latch e item pendente limpos no unload, saída da página, transição/reset e erro.
- Fluxo respeita checks de idade/equipamento e não perde IDs runtime de itens de outros providers.
- Se a UI não for uma necessidade genérica aprovada, retirar a máquina de estados do overlay e implementá-la na DLL usando hooks/escape hatch existentes.

**Aceite:** fixture com outro nome de mod usa o contrato; DMR/NEI/add-on coexistem; desligar DMR não deixa seletor/input pendente; sem dono o pause vanilla funciona; cancelamento/confirmar/save-reload têm prova real.

### Montagem de camadas declarada por provider — a76045848

**Fato:** `IsUnboundLayerArchive` lê `unbound.json`, exige `format="unbound"` e `formatVersion=2`; o bootstrap adia esses archives e os monta na raiz depois da carga de providers, antes de `game.ready`. Isso vai além de comentários ou nome de tarefa: o host decide pelo formato do framework. Fonte: G(a76045848), `ShipLuaBootstrap.cpp`. Handoff específico de insuficiência/RFC para esse desvio: **não confirmado**.

**Justificativa possível, descrita no diff:** a base é convertida durante init; as camadas precisam dos caminhos originais e da fase posterior para mesclar cenas/entradas sem contaminar a conversão.

**Saída proposta:** mecanismo genérico de camadas do VFS declaradas/solicitadas por provider aceito, com escopo de montagem, fase, ordem e ownership. O framework inspeciona seu próprio manifesto de archive e pede montagem; o bootstrap não conhece `unbound.json` ou versões Unbound. Aproveitar `linkspan.oot.resources`/VFS existentes antes de criar outro serviço.

**Correção do RC5 (2026-10-09), dentro desta mesma dívida:** sem o mod `linkspan.unbound.framework` carregado, o
bootstrap ignora toda camada Unbound, solta em `mods/` ou dentro da pasta de um mod, e grava `ShipLua ignorou a camada
Unbound '…': o framework Unbound ('linkspan.unbound.framework') não carregou`. Antes, a camada solta entrava na raiz
mesmo sem ninguém para ler os documentos dela, sem diagnóstico (plano §10.4 e §18: recusa com diagnóstico em vez de
carga parcial). O host passa a conhecer também o id do framework; a saída proposta acima remove os dois.

Preservar a correção `4c45d5c3a`: provider rejeitado nunca sombreia o VFS. Ordenação deve ser determinística; erro/init parcial desfaz montagem do dono; unload invalida/retira camadas com tratamento explícito do cache e dos recursos vivos. Arquivo solto sem manifesto mantém uma política de data mod documentada; compatibilidade Unbound antiga pode ser reconhecida pelo framework, sem detector permanente no host.

**Aceite:** camada de dois providers genéricos respeita prioridade; camada de provider rejeitado/ausente não entra na raiz; init falho não deixa sombra residual; JSON mescla no caminho/fase esperados; unload/reload e fallback vanilla são exercitados. Não prometer descarregamento seguro de recursos ainda usados sem contrato e teste.

## Comportamento em OoT

O adaptador OoT permanece primeiro. O host só conhece dados de jogo, contratos genéricos, transporte e operações de baixo nível. Instalar outro item, ator, mapa ou efeito de um framework não requer outra compilação. Serviços introduzidos para um consumidor inicial continuam disponíveis para fixtures e outros providers, sem testar IDs/nome do consumidor.

Alterar `z_actor.c` ou `En_Elf` para despachar um hook genérico é uma mudança B justificável; precisa de vanilla/fase/payload/lifetime, não de um nome disfarçado. Render toon/stencil continua inativo sem emissão/estado do mod. Sem coremods, nenhum probe de áudio ou input dispara; a remoção `fc2709989` é parte obrigatória dessa garantia.

A revisão deve listar qualquer campo/enum estrutural novo usado por nativos. Não confundir teste de engine com funcionamento em cena real: transições, save/reload, itens e UI requerem os gates correspondentes.

## Comportamento em MM

Nenhuma mudança no host MM é autorizada ou requerida por este rascunho. MM fica para a expansão depois do Gate OoT, conforme o plano. Contratos comuns de provider/serviço/hook/lifecycle mantêm semântica independente do jogo; `linkspan.oot.*` não é anunciado pelo MM. Consultas ausentes produzem `UNSUPPORTED` e o provider recusa a função dependente antes de gastar recursos.

Uma implementação MM posterior tem inventário/gate/fingerprint próprios. Não reutiliza layouts/payloads/IDs OoT nem a exceção histórica NEI-HOST-001 como licença para alterar o segundo host. Recursos específicos poderão usar namespace `linkspan.mm.*`; serviço comum novo só se a semântica for realmente comum.

## Erros e recuperação

Aplicar os erros já contratados, sem inventar sucesso parcial:

- `UNSUPPORTED`: serviço/hook/layout/fingerprint ou fase de operação indisponível; fallback ou recusa explícita do provider.
- `INVALID_ARGUMENT`: tamanho, versão, ID, argumento ou buffer inválido, conforme a tabela específica.
- `LIMIT`: lease exclusivo, capacidade finita, segundo replace/patch ou buffer insuficiente quando assim definido.
- `FAILURE`: operação/backend não concluído; rollback do registro/carga quando aplicável.

Diagnóstico identifica contrato, versão, dono e operação; não imprime credenciais. Nenhuma exceção cruza a fronteira C. O erro de transcode não autoriza publicar recurso parcial; o fallback vanilla deve evitar recursão. Quarentena de unload da RFC 0023 conserva DLL/contexto enquanto houver patch vivo, permitindo retry; não libera memória para "concluir" um unload falho.

## Compatibilidade e versionamento

Há quatro identidades distintas:

| Identidade | Regra proposta | Efeito no consumidor |
|---|---|---|
| Core Extension ABI C (`ShipNativeRuntime`) | Acrescentar callback ao fim, mantendo offsets/assinaturas e negociação por size: **MINOR**. Corrigir implementação sem alterar contrato: **PATCH**. Remover/reordenar/alterar contrato binário público: **MAJOR**. | Host mais velho recusa minor exigido; consumidor antigo ignora cauda compatível. |
| Tabela de serviço / payload de hook | Versão explícita por nome; publicar nova Vn e conservar V anterior quando ainda suportada. Conferir prefixos e `size`. Payload incompatível recebe versão nova; não reinterpretar payload antigo. | Não obriga bump da ABI C do runtime só porque existe serviço novo. |
| Layout id do SDK OoT | Recalcular conforme `ExportSdk.cmake` e toolchain; publicar hash **completo**, inputs e causa. Não fixar o hash manualmente. | Mods que declaram layout exato podem ser recusados mesmo com prefixo C compatível. |
| Fingerprint de `soh.exe` | SHA-256 do executável realmente empacotado, distinto do SHA Git. Atualizar lista de fingerprints/símbolos de cada provider que usa escape hatch. | Qualquer rebuild que altere bytes exige nova autorização de fingerprint/reempacote; recompilar DLL se offsets/layout/contrato exigirem, não automaticamente em toda troca de SHA. |

ABI minor não é versão da API Lua, da release do overlay, da tabela V3 ou o hash de layout. Uma nova API `linkspan.oot.*` pode manter ABI C 1.3. Uma mudança de layout estrutural público que quebra binários exige contrato major novo ou layout explícito incompatível, nunca rotular como adição compatível; só comentário/hash/toolchain não é prova de quebra major da ABI C.

A implementação consultada calcula o layout sobre `soh/include/*.h`, `soh/soh/native/*.h` e `shiplua/native/*.h`, incluindo headers C++ internos e comentários, mais toolchain/flags. Portanto `cd6691cef` pode ser D e trocar layout; `d1b76ff56` troca layout só com comentários. `NativeEscapeHatch` da COREEXT-010 é ponte C++ interna, não os callbacks C `beginBatch/commitBatch` adicionados a `ShipNativeRuntime` — estes não existem no diff analisado.

O algoritmo atual normaliza EOL desde `75a420414`; o handoff CEL-004 distingue `b3e57ace` bruto de `b459725c` normalizado. Esta RFC não muda o algoritmo nem presume equivalência de ids divergentes. Trocar a cobertura do hash para excluir comentários/internals seria outra mudança de tooling contratual, com validação explícita.

No `d61b30877`, point_color V2 muda payload; a mensagem informa que V1 não chegou a release. Tratar isso como justificativa histórica declarada, não como prova de compatibilidade com toda DLL antiga. Futuramente, descontinuar versão experimental exige listar consumidores e distribuição afetada; versões publicadas precisam de período/política de suporte.

### Agrupamento de recompilações

1. Abrir uma rodada identificada com base do host/core/libultraship, RFCs, lacunas e commits previstos. Separar modificações de pacote das de substrato.
2. Agrupar alterações de headers/tabelas/layout e ajustes de ABI/símbolos antes de publicar SDK/pacotes. Builds intermediárias de desenvolvimento são permitidas e devem ser rotuladas, sem tratá-las como entrega final.
3. Congelar inputs/toolchain e gerar o conjunto final host, SDK, símbolos e recursos. Qualquer mudança posterior que altere identidade reabre a rodada explicitamente.
4. Revalidar dependentes; reempacotar fingerprints e recompilar quem usa layout/offset incompatível. Testar também DLL antiga que deveria continuar compatível.
5. Comparar vanilla e combinação de mods no mesmo conjunto final. Publicar hashes e rollback somente após gates correspondentes.

"Uma rodada" significa uma entrega coerente, não fingir que vários commits/compilações são um executável. É permitido iterar antes do freeze; trocar o executável durante uma sessão invalida a evidência daquela sessão. Correções posteriores não podem esconder novo contrato em "carona de layout".

## Segurança

Nativos são código confiável no processo do jogo, sem sandbox. Manifesto não se autoautoriza. Fingerprint/owner/contexto e capacidade precisam ser validados pelo runtime; não afrouxar a verificação para reutilizar ZIP antigo.

Registro, montagem, UI, luzes, hooks e patches têm ownership e rollback. Não aceitar strings de mod como prova de autorização para remover estado de outro. Ponteiros/payloads respeitam lifetime e thread; nenhuma DLL sai com callback/patch ainda vivo. Camadas rejeitadas não sombreiam assets e serviços não executam código de provider rejeitado.

Entrega conserva limites de conteúdo protegido: teste não autoriza empacotar ROM/asset sem licença. Sondas temporárias ficam em fixtures/pacotes isolados e não disparam em host sem mods. Este levantamento não leu credenciais nem modificou dados de jogo.

## Testes e evidências exigidos

Plano de validação para futura implementação, **não executado nesta tarefa**:

- ABI: offsets/prefixos por static assertions, negociação de tamanho/major/minor, DLL anterior sem recompilar, recusa em host antigo e consulta de serviço indisponível.
- Hooks: fase/thread/payload e init tardio; conflitos, rollback, callback falho, unload e repetição/reentrância; native_hook_tests/native_provider_tests e suites do adaptador conforme contrato.
- Escape hatch: fingerprint errado, símbolo ausente/ambíguo/inlined/folded; lote init/unload, erro parcial, quarentena/retry/reload e isolamento de donos.
- Layout/build: mesma entrada normalizada reproduz mesma identidade sob mesma toolchain; mudança real gera novo id; executável/símbolos/SDK/mods/soh.o2r da mesma entrega.
- Genéricos: consumers de fixture independentes dos mods originais para áudio, inventário/input, VFS, randomizer, Anchor, render/luzes e actor-models.
- Vanilla e mods: boot sem mod/probe, install/uninstall, mod rejeitado, troca de cena, save/reload e rollback em cópia isolada; combinação Unbound/NEI/DMR/Mic/CEL conforme o gate. Controle/microfone físicos não são certificados por testes de engine.
- CI: CTest raiz deve enumerar e executar testes; zero testes não é sucesso. Verificar rotinas D/E com testes que reproduzam seus defeitos.

**Validação desta auditoria:** leitura de Git/mensagens e fontes primárias locais; confronto dos 24 SHAs, datas e listas de arquivos; fontes por linha; conferência da classificação e estrutura dos dois Markdown. Resultados históricos citados são declarações dos handoffs/mensagens, sem repetição de testes/build/jogo.

## Handoff obrigatório para próxima rodada

O handoff deve conter:

1. ID da tarefa/rodada, owner, motivação, categoria principal e facetas; RFC/emenda e decisão de exceção quando houver.
2. Fixture da lacuna e alternativas (DLL, serviço/hook existente, escape hatch, generalização), inclusive os resultados negativos necessários.
3. Base e SHAs finais completos de host/core/libultraship, estado local relevante e toolchain/flags; lista de arquivos efetivamente compilados.
4. ABI C antes/depois; serviços/payloads com versões/sizes; diff de contrato; layout id completo antes/depois e causa, separando intermediários do final.
5. Se houve recompilação do host, apenas DLLs ou só reempacote; executável exato/SHA-256, símbolos/SDK/recursos e hashes dos pacotes finais.
6. Dependentes afetados, lista de fingerprints, rebuild/reempacote realizado e teste de compatibilidade das DLLs antigas que deveriam funcionar.
7. Comandos/resultados/evidência, separados em estático, unidade, build, pacote e runtime; limitações físicas e gates ainda abertos.
8. Sem mod/erro/unload/rollback/conflitos, destino de probes, restauração da cópia anterior e ausência de alteração em instalações fora da tarefa.
9. Dívidas C com responsável designado, marco de remoção e critério verificável. Dono/prazo real desta dívida ainda precisa ser definido na revisão; não há atribuição inventada aqui.

## Depreciação e migração

Não há remoção nem alteração de código nesta RFC em rascunho. Propõe-se:

- Descontinuar `LinkSpan_DynamicMovementInventoryMenuEnabled` em favor da política genérica ou implementação na DLL. Não prometer alias binário para símbolo interno: o uso por escape hatch continua preso ao fingerprint.
- Migrar nomes/adapter de áudio Unbound para operações genéricas, preservando compatibilidade dos dados de cena que já usam `Song`.
- Retirar detector de formato Unbound do bootstrap após disponibilidade e migração do mecanismo genérico de camadas. Compatibilidade de dados pertence ao framework.
- Manter versões públicas antigas de tabela/hook durante a janela definida na revisão; registrar explicitamente versões experimentais não publicadas que forem abandonadas.
- Encerrar a dívida histórica somente com implementação revisada, prova do consumidor genérico/vanilla e handoff atualizado. Não marcar §18 verde apenas pela existência do inventário.

Aprovação, responsáveis e marco de remoção das dívidas: **não confirmado / pendente de revisão**.

## Apêndice A — fontes e limites

### Fontes e método

- **G(SHA)**: mensagem e diff do commit no host `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001`. Consulta reproduzível: `git -c safe.directory=D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001 -C D:/Desenvolvimento/ship-lua-worktrees/OOT-UPSTREAM-78DC-001 show <SHA> -- soh/soh soh/src soh/include libultraship CMakeLists.txt soh/CMakeLists.txt`.
- **H/**: `D:\Desenvolvimento\ship-lua-worktrees\link-span\coordination\handoffs`; números após o nome indicam linhas da cópia consultada.
- **CH/**: `D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001\extern\ship-lua\coordination\handoffs`.
- **R0021/R0023**: modelos locais `0021-native-hooks.md` e `0023-native-escape-hatch.md`; **R0022/R0025/R0026**: RFCs correspondentes em `extern/ship-lua/rfcs/` do host.
- **plan.md**: `docs/plans/oot-coremods-unbound-nei/plan.md` do repositório de coordenação link-span, commit `1c5c6ea` (os números de linha citados são dessa versão), sobretudo §4, §8, §13 Fases F/W, §15.2 e §18.
- O filtro inclui `soh/CMakeLists.txt`, que consta da entrada em dois commits, embora a descrição inicial do filtro cite apenas o CMake raiz. A lista de arquivos foi conferida com `git diff-tree --no-commit-id --name-only -r <SHA>`; não usa nomes abreviados do `--stat`.
- Datas são datas do autor retornadas pelo Git (`--date=short`); nos 24 commits autor e committer têm a mesma data civil. Handoffs podem descrever trabalho anterior ao commit consolidado.
- O marco `16c4622d5`, 2026-09-18, é o baseline solicitado. H/OOT-CORE-007.md:33,43,99 identifica host/layout `3e3f59ad` e ainda formula o fechamento do gate como próxima ação. A data exata de aprovação formal do gate não foi confirmada independentemente.
- Um commit não equivale a uma compilação executada. Mensagens/handoffs comprovam intenções e algumas builds históricas; esta auditoria não contou execuções de compilador, não inspecionou binários e não certifica gameplay.
- O escopo é exatamente o conjunto dos 24 commits do host entre `16c4622d5` e `fc2709989` que tocam `soh/soh`, `soh/src`, `soh/include`, `libultraship` ou CMake (`git log --stat 16c4622d5..fc2709989 -- soh/soh soh/src soh/include libultraship CMakeLists.txt soh/CMakeLists.txt`), e não todos os commits/builds desde o baseline. `75a420414` aparece apenas como contexto confirmado de normalização do layout, pois não está na lista.
- Git do submódulo usa seu próprio `safe.directory`, sem escrever configuração. `docs/agents/Plans.md` no host está ausente; a análise usa o plano local, mensagens e handoffs.

Categorias principais exclusivas: A=0, B=6, C=2, D=12, E=4. A aparece como faceta nos quatro commits mistos indicados. D preserva a ABI C pública, mas pode trocar headers internos/hash ou semântica operacional documentada. E cobre comentários que alteram fingerprint, além de build/tooling.

Os ids curtos são abreviações históricas. "Não pelo diff" não comprova uma build. RFC de base citada é suporte ao mecanismo, não prova de formalização da mudança específica. A ausência de prova fica marcada como "não confirmado". O inventário completo com explicações complementares está em [0027-inventario.md](0027-inventario.md).

## Apêndice B — inventário dos 24 commits

| SHA | Data | Arquivos de host (lista completa do filtro) | Categoria principal e conteúdo | Layout id | RFC/justificativa e tarefa motivadora | Fontes por linha |
|---|---|---|---|---|---|---|
| `b13913077` | 2026-09-18 | `soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeItems.cpp`<br>`soh/soh/native/OotNativeItems.h`<br>`soh/soh/native/OotNativeItemsGame.cpp`<br>`soh/soh/native/OotNativeJsonTypes.cpp`<br>`soh/soh/native/OotNativeJsonTypes.h`<br>`soh/soh/native/OotNativeJsonTypesGame.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/native/OotNativeScenes.h`<br>`soh/soh/native/OotNativeScenesGame.cpp`<br>`soh/soh/native/OotNativeText.cpp`<br>`soh/soh/native/OotNativeText.h`<br>`soh/soh/native/OotNativeTextGame.cpp`<br>`soh/soh/native/oot_items.h`<br>`soh/soh/native/oot_resources.h`<br>`soh/soh/native/oot_scenes.h`<br>`soh/soh/native/oot_text.h`<br>`soh/soh/resource/importer/CollisionHeaderFactory.cpp`<br>`soh/soh/resource/importer/SceneFactory.cpp`<br>`soh/soh/resource/importer/SceneFactory.h`<br>`soh/soh/resource/importer/scenecommand/SetAlternateHeadersFactory.cpp`<br>`soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.cpp`<br>`soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h`<br>`soh/soh/resource/importer/scenecommand/SetLightListFactory.cpp`<br>`soh/soh/resource/importer/scenecommand/SetLightingSettingsFactory.cpp`<br>`soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp`<br>`soh/soh/resource/importer/scenecommand/SetSoundSettingsFactory.cpp`<br>`soh/soh/z_message_OTR.cpp`<br>`soh/soh/z_message_OTR.h`<br>`soh/soh/z_play_otr.cpp`<br>`soh/src/code/z_actor.c`<br>`soh/src/code/z_parameter.c` | **C** — Nova chamada SOH::Unbound::SequenceIdForPath em SetSoundSettingsFactoryXML e uso de unboundSongPath/unboundSongSeqId. Também A: items/resources V3, scenes V2; B: text V1/JSON transcode; fábricas XML genéricas. | Sim: 3e3f59ad → 31899f1f (handoff/mensagem). | OOT-CORE-008 para NEI-002 e fase E. Música por caminho é justificativa funcional; helper/campos já existiam no gate, mas nova chamada nominal é pós-gate. Demonstração de insuficiência/RFC específica: não confirmado. | G(`b13913077662300db4f789dc61b47876a2e9a8b8`); H/OOT-CORE-008.md:9-28,64; git show 16c4622d5:soh/soh/unbound/UnboundAudio.h; git show 16c4622d5:soh/soh/resource/type/scenecommand/SetSoundSettings.h; mensagem |
| `56193b774` | 2026-09-18 | `soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeJsonTypes.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/z_play_otr.cpp` | **D** — Transcode no pool, acesso controlado a VFS/nomes e fallback vanilla; console genérico linkspan_travel (E acessório), sem nova ABI. | Não: 31899f1f (mensagem). | UNBOUND-E/UNBOUND-007, correção de OOT-CORE-008; RFC específica não confirmada. | G(`56193b77492cb80ea28a191a2a2783dd6dd21214`); H/UNBOUND-E.md:27-37; H/OOT-CORE-008.md:23; mensagem |
| `660bd3085` | 2026-09-19 | `soh/soh/native/OotNativeJsonTypes.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/native/OotNativeScenesGame.cpp`<br>`soh/soh/resource/importer/CollisionHeaderFactory.cpp` | **D** — Valida colisão/title card e sincroniza mapas lidos no pool; symdump de dados é E, fora dos arquivos de host filtrados. | Não: 31899f1f (mensagem). | Revisão E+F, UNBOUND-E/NEI-002; RFC específica não confirmada. | G(`660bd3085b3ff741ffeb9632e49f1d96cf119820`); H/UNBOUND-E.md:89-104; mensagem |
| `40530741c` | 2026-09-19 | `soh/CMakeLists.txt`<br>`soh/soh/native/OotNativeHostKeep.c` | **E** — Tabela gerada de 2102 endereços + /OPT:NOICF; recupera símbolos eliminados/fundidos, sem implementar política de mod. | Não: 31899f1f (mensagem/handoff). | NEI-004A; insuficiência concreta da resolução de símbolos registrada; mecanismo RFC 0023, RFC específica de tooling não confirmada. | G(`40530741ca4e08e119b317a6d24b6f38f8b7ad24`); H/NEI-004A.md:37-49,99-113; mensagem |
| `b8c0f6e8d` | 2026-09-19 | `libultraship` | **B** — Gitlink libultraship 69ac4eb0d → 00733547: transporte genérico toon/stencil/sombra, opcodes e backends, inativo sem emissões. É substrato genérico; não serviço C novo. | Não: 31899f1f (mensagem). | CEL-002, rodada W. Justificativa/capturas sem mod no handoff; RFC específica não confirmada. | G(`b8c0f6e8dd5c28da4118bd8e2f299e4b8b472f69`); H/CEL-002.md:10,35-44; git do libultraship show 00733547; mensagem |
| `f811f71d6` | 2026-09-19 | `soh/include/z64scene.h`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeHooks.cpp`<br>`soh/soh/native/OotNativeHooks.h`<br>`soh/soh/native/OotNativeHooksGame.cpp`<br>`soh/soh/native/OotNativeSaveGame.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/native/OotNativeScenes.h`<br>`soh/soh/native/OotNativeScenesGame.cpp`<br>`soh/soh/native/OotNativeView.cpp`<br>`soh/soh/native/OotNativeView.h`<br>`soh/soh/native/OotNativeViewGame.cpp`<br>`soh/soh/native/oot_hooks.h`<br>`soh/soh/native/oot_render.h`<br>`soh/soh/native/oot_scenes.h`<br>`soh/soh/resource/importer/scenecommand/SetAnimatedMaterialListFactory.cpp`<br>`soh/soh/z_scene_otr.cpp`<br>`soh/src/code/z_actor.c`<br>`soh/src/code/z_horse.c`<br>`soh/src/code/z_lights.c`<br>`soh/src/code/z_play.c`<br>`soh/src/code/z_scene_proc.c`<br>`soh/src/overlays/actors/ovl_Bg_Jya_Cobra/z_bg_jya_cobra.c`<br>`soh/src/overlays/actors/ovl_En_Dekubaba/z_en_dekubaba.c`<br>`soh/src/overlays/actors/ovl_En_Heishi1/z_en_heishi1.c`<br>`soh/src/overlays/actors/ovl_En_Horse/z_en_horse.c`<br>`soh/src/overlays/actors/ovl_En_Kanban/z_en_kanban.c`<br>`soh/src/overlays/actors/ovl_En_Karebaba/z_en_karebaba.c`<br>`soh/src/overlays/gamestates/ovl_file_choose/z_file_choose.c`<br>`soh/src/overlays/gamestates/ovl_select/z_select.c` | **B** — Hooks genéricos de render/céu/luz e substrato de atores; também A: render V2/scenes V3. Floats de scroll e suporte Epona/save/warp genéricos; sem políticas/CVars de CEL no host. | Sim: d491fbf7 intermediário (mensagem); rodada final ce528052 (handoffs), não confundir as identidades. | CEL-003 + UNBOUND-013/014/015, rodada W prevista no plano. RFC 0021 é base de hooks; RFC específica dos novos contratos não confirmada. | G(`f811f71d6c1030d5805e00e187b7f2e7d6619c25`); H/CEL-003.md:5,22,32,47; H/UNBOUND-013.md:27; H/UNBOUND-014.md:26; H/UNBOUND-015.md:21; H/CEL-002.md:35-36; plan.md:1321-1341; mensagem |
| `cd6691cef` | 2026-09-19 | `soh/soh/native/OotNativeSaveGame.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/native/OotNativeScenes.h`<br>`soh/src/code/z_actor.c`<br>`soh/src/code/z_horse.c`<br>`soh/src/overlays/actors/ovl_En_Horse/z_en_horse.c` | **D** — Corrige pré-passe de sombras e resolução/consumo do nome salvo da Epona; grava pose de chamada. Acrescenta declaração em header interno de native. | Sim pelo glob: OotNativeScenes.h mudou. Rodada W terminou em ce528052; atribuição exclusiva desse valor ao commit não confirmado. | UNBOUND-013 + revisão CEL-003; RFC específica não confirmada. | G(`cd6691cefa5d0f15b32ff9475ef7f7354964b2da`); H/UNBOUND-013.md:20,27-29,45; H/CEL-003.md:34; H/CEL-002.md:35-36; git show 16c4622d5:soh/native-sdk/ExportSdk.cmake:7-20; mensagem |
| `224cd14af` | 2026-09-19 | `soh/src/code/main.c` | **D** — LC_NUMERIC=C após locale UTF-8; impede tinyxml2/CRT de truncar valores com ponto. | Não pelo diff; id individual não confirmado; rodada final ce528052 (handoff). | UNBOUND-014: diagnóstico pt-BR e teste de locale documentados; RFC específica não confirmada. | G(`224cd14afc7a2373ea093af30269f79aa4940fe5`); H/UNBOUND-014.md:26-27,42; mensagem |
| `4f40d1126` | 2026-09-25 | `soh/CMakeLists.txt` | **E** — Desativa inline/LTCG por lista hookable_sources.txt em MSVC; mudanças com nomes NeiFork da mensagem são na DLL, não neste diff de host. | Não: ce528052 (handoff). | NEI-HOST-001/NEI-008: 314 funções sem endereço próprio; exceção de uma rodada antes das ondas escolhida em 25/09. RFC 0023 fornece escape hatch; RFC específica desta rodada não confirmada. | G(`4f40d1126be4bd710ebe92cab8725410e3aaf998`); H/NEI-HOST-001.md:9-19,66; H/NEI-008.md:50; plan.md:1314-1318; mensagem |
| `e5a0a4836` | 2026-09-26 | `soh/soh/Enhancements/randomizer/3drando/item_pool.cpp`<br>`soh/soh/Enhancements/randomizer/SeedContext.cpp`<br>`soh/soh/Enhancements/randomizer/hook_handlers.cpp`<br>`soh/soh/Enhancements/randomizer/item_list.cpp`<br>`soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerGet.h`<br>`soh/soh/Enhancements/randomizer/randomizerEnums/RandomizerSettingKey.h`<br>`soh/soh/Enhancements/randomizer/settings.cpp`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeItemsGame.cpp`<br>`soh/soh/native/OotNativeRando.cpp`<br>`soh/soh/native/OotNativeRando.h`<br>`soh/soh/native/OotNativeRandoGame.cpp`<br>`soh/soh/native/OotNativeSaveGame.cpp`<br>`soh/soh/native/oot_randomizer.h` | **B** — Novo linkspan.oot.randomizer V1 e integração de itens sintéticos no pool/save/get-item; fallback sem provider. Nome de tarefa NEI não torna o serviço específico. | Sim: d2aece6b (mensagem/handoff). | NEI-HOST-002/NEI-013; decisão do usuário de 25/09 registrada no handoff. RFC específica da V1 não confirmada; RFC 0026 posterior cobre consultas V2. | G(`e5a0a48367f6c14b3159be59e185d0955cf0b128`); H/NEI-HOST-002.md:9-40,68; H/NEI-013.md:23; mensagem |
| `1d6287b46` | 2026-09-26 | `soh/soh/Network/Anchor/HookHandlers.cpp`<br>`soh/soh/Network/Anchor/Packets/GiveItem.cpp`<br>`soh/soh/Network/Anchor/Packets/UpdateTeamState.cpp`<br>`soh/soh/native/OotNativeAnchor.cpp`<br>`soh/soh/native/OotNativeAnchor.h`<br>`soh/soh/native/OotNativeAnchorGame.cpp`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeHooks.cpp`<br>`soh/soh/native/OotNativeHooks.h`<br>`soh/soh/native/OotNativeSave.cpp`<br>`soh/soh/native/OotNativeSave.h`<br>`soh/soh/native/OotNativeSaveGame.cpp`<br>`soh/soh/native/oot_anchor.h`<br>`soh/soh/native/oot_hooks.h` | **B** — Novo linkspan.oot.anchor e oot.anchor.state; itens e namespaces por nomes de registro/schema, sem conhecimento de NEI. | Sim: 0d86df42 (handoff). | NEI-HOST-003/NEI-014; handoff registra decisão de manter suporte genérico no modloader. RFC específica não confirmada. | G(`1d6287b46f4dd0bb35667e01e0e2d230361ba6fd`); H/NEI-HOST-003.md:9-34,74; H/NEI-014.md:22; mensagem |
| `6fa3f2a2a` | 2026-09-26 | `soh/soh/Enhancements/Restorations/GraveHoleJumps.cpp`<br>`soh/soh/SaveManager.cpp`<br>`soh/soh/SohGui/UIWidgets.hpp`<br>`soh/soh/native/OotNativeSaveGame.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/resource/importer/scenecommand/SetMeshFactory.cpp`<br>`soh/src/code/code_800EC960.c` | **D** — Save de entradas/Farore por nome estável e fallback sem mod; valida mesh, ambience, combobox/global.sav e override real de Grave Hole Jumps. Repara persistência genérica, sem chamada nominal nova de mod. | Não: 0d86df42 (mensagem/handoff). | UNBOUND-017 (host do delta UNBOUND-016); correções de host documentadas. RFC 0022 é base de namespaces, não confirmação de RFC específica para cada correção. | G(`6fa3f2a2a5d6e3771ca666049e51e0a11d537d2b`); H/UNBOUND-017.md:5-35,67; H/UNBOUND-016.md:35; mensagem |
| `5c4138b80` | 2026-09-26 | `soh/soh/native/OotNativeEscapeWin.cpp`<br>`soh/soh/native/OotNativeScenes.cpp`<br>`soh/soh/native/oot_resources.h`<br>`soh/soh/native/oot_scenes.h` | **D** — Batch genérico do escape hatch; acerta limite de entradas/comentário de thread. beginBatch/commitBatch são campos opcionais ao fim da ponte C++ NativeEscapeHatch no core, não tabela C da ABI do provider. | Sim: 0d86df42 → 07cb0af3 (handoff; mensagem). | COREEXT-010; RFC 0023 emendada, reabertura W. Não contabilizado A de ABI C estável. | G(`5c4138b80ca62403dc8cdb604e61c15d337642c9`); H/COREEXT-010.md:24-29,59-60; core git show 6731d60 -- include/shiplua/native/NativeProvider.h; R0023; mensagem |
| `6ba757d7b` | 2026-09-26 | `soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/ShipLuaBootstrap.h`<br>`soh/soh/SohGui/SohMenu.cpp`<br>`soh/soh/SohGui/SohMenu.h`<br>`soh/soh/SohGui/SohMenuModRegistry.cpp`<br>`soh/soh/SohGui/SohMenuModRegistry.h` | **B** — Menu declarativo genérico e pump de callbacks; serviço comum de UI (core.menu/ship.menu), sem nome de mod. Inclui correção de slot vazio (D). | Não: 07cb0af3 (mensagem). | MENU-001; RFC 0025 vinculada ao core 8110748. | G(`6ba757d7b5d4d253b137573ff6a9161e481bac14`); H/MENU-001.md:56-59; R0025; plan.md:1343-1345; mensagem |
| `d61b30877` | 2026-09-26 | `soh/soh/OTRGlobals.cpp`<br>`soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeHooks.cpp`<br>`soh/soh/native/OotNativeHooks.h`<br>`soh/soh/native/OotNativeHooksGame.cpp`<br>`soh/soh/native/OotNativeItemsGame.cpp`<br>`soh/soh/native/OotNativeLights.cpp`<br>`soh/soh/native/OotNativeLights.h`<br>`soh/soh/native/OotNativeLightsGame.cpp`<br>`soh/soh/native/OotNativeView.cpp`<br>`soh/soh/native/OotNativeView.h`<br>`soh/soh/native/OotNativeViewGame.cpp`<br>`soh/soh/native/oot_hooks.h`<br>`soh/soh/native/oot_lights.h`<br>`soh/soh/native/oot_render.h`<br>`soh/src/overlays/actors/ovl_En_Elf/z_en_elf.c` | **B** — Novo serviço genérico de luzes e hook fairy; point_color V2. Também A: render V3 preservando prefixo V2. Overlay En_Elf despacha contrato genérico. | Sim: 1fe73b60 (mensagem). | CEL-004A/CEL-004; pontos previstos na reabertura W de 26/09. RFC 0021 é mecanismo, não RFC específica dos novos contratos; esta não confirmada. | G(`d61b30877887be50eeb184e0ae59655d855bfff6`); H/CEL-004.md:54-58,90; plan.md:1343-1348; mensagem |
| `845b771a9` | 2026-09-26 | `soh/soh/native/OotNativeHooksGame.cpp`<br>`soh/soh/native/OotNativeLights.cpp`<br>`soh/soh/native/OotNativeView.cpp`<br>`soh/soh/native/OotNativeViewGame.cpp`<br>`soh/src/overlays/actors/ovl_En_Elf/z_en_elf.c` | **D** — Corrige despacho da Navi, reentrância/raio de luz, estado toon e desarme de sombra; chamadas no overlay são hooks genéricos, não chamadas de CEL. | Não conforme mensagem (comentários nos headers adiados); valor individual não confirmado. | CEL-004A revisão; RFC 0021 cobre mecanismo de hook, mas RFC específica desta correção não confirmada. | G(`845b771a98cadb25a4f74d10331c3ce778a5d49d`); H/CEL-004.md:47-50,90-91; mensagem |
| `600f83b06` | 2026-09-27 | `soh/soh/SohGui/SohMenuModRegistry.cpp`<br>`soh/soh/SohGui/SohMenuModRegistry.h` | **D** — Corrige overflow de slider, ordem/coalescência de inputs e lifetime de edição do menu. | não confirmado por commit; header SohMenuModRegistry.h não pertence ao glob de layout. | MENU-001 revisão; contrato RFC 0025; justificativa no handoff. | G(`600f83b06d05f76b350defdab837342757e6e865`); H/MENU-001.md:45-59; R0025; mensagem |
| `7d26819bb` | 2026-09-27 | `soh/soh/native/OotNativeEscapeWin.cpp` | **D** — Corrige escape hatch por dono, remoção pendente e quarentena. ABI C dos providers preservada; assinaturas C++ internas install/beginBatch mudam no core f29505c (não é extensão A). | Sim pelo header do core f29505c; id isolado não confirmado. b3e57ace só confirmado no conjunto com d1b76ff56; final normalizado b459725c. | COREEXT-010 rodada 2; RFC 0023 emendada e revisão documentada. | G(`7d26819bbfe5a4bdc0e0106ed2f2e6f7aa249f55`); H/COREEXT-010.md:33-63,97; core git show f29505c -- include/shiplua/native/NativeProvider.h; R0023; mensagem |
| `d1b76ff56` | 2026-09-27 | `soh/soh/native/oot_hooks.h`<br>`soh/soh/native/oot_lights.h`<br>`soh/soh/native/oot_render.h` | **E** — Só comentários dos contratos de hooks/luzes/render, mas os headers integram o fingerprint de layout. | Sim: 1fe73b60 → b3e57ace, junto do core da COREEXT-010 rodada 2 (mensagem). Depois 75a420414 normaliza EOL e publica b459725c, fora dos 24. | CEL-004/revisão CEL-004A; registro no handoff/plano W. RFC exclusiva desses comentários não confirmada. | G(`d1b76ff56761a02cba3317f6444238db8cd0dbad`); H/CEL-004.md:47-50,175-182; plan.md:1343-1348; mensagem |
| `26b8ca2a8` | 2026-10-02 | `CMakeLists.txt` | **E** — CTest raiz, Python no escopo raiz e prefixo miniz dos testes embutidos; workflow está fora do filtro. | não confirmado; diff não altera headers de layout nem flags do jogo explicitamente. | OOT-CI-001; justificativa: CTest raiz listava zero testes. RFC específica não confirmada. | G(`26b8ca2a8a3ef8919482a821f7306091bcaa3260`); CH/OOT-CI-001.md:9-16,28; mensagem |
| `a76045848` | 2026-10-02 | `soh/soh/Enhancements/debugconsole.cpp`<br>`soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/SohGui/SohMenu.cpp`<br>`soh/soh/SohGui/SohMenu.h`<br>`soh/soh/SohGui/SohMenuModRegistry.cpp`<br>`soh/soh/SohGui/SohMenuModRegistry.h`<br>`soh/soh/native/OotActorModelType.h`<br>`soh/soh/native/OotNativeActorModels.cpp`<br>`soh/soh/native/OotNativeActorModels.h`<br>`soh/soh/native/OotNativeEngine.cpp`<br>`soh/soh/native/OotNativeHooks.cpp`<br>`soh/soh/native/OotNativeHooks.h`<br>`soh/soh/native/OotNativeHooksGame.cpp`<br>`soh/soh/native/OotNativeRando.cpp`<br>`soh/soh/native/OotNativeRando.h`<br>`soh/soh/native/OotNativeRandoGame.cpp`<br>`soh/soh/native/OotNativeRegistry.cpp`<br>`soh/soh/native/OotNativeRegistry.h`<br>`soh/soh/native/oot_actor_models.h`<br>`soh/soh/native/oot_hooks.h`<br>`soh/soh/native/oot_randomizer.h`<br>`soh/soh/native/oot_registry.h`<br>`soh/src/code/z_actor.c`<br>`soh/src/code/z_play.c`<br>`soh/src/overlays/misc/ovl_kaleido_scope/z_kaleido_item.c` | **C** — DMR: função nominal + seletor C no kaleido. Bootstrap reconhece unbound.json/formatVersion=2 e monta camadas na raiz após provider. Também A: registry/randomizer V2; B: actor-models e hooks init/draw_end; D: ponte de velocidade; UI genérica. | Sim: 9a593a6e (mensagem); hashes/layouts c97ff67f em handoffs anteriores são snapshots diferentes. | BIND-002, SKIJER-001, OOT-NEI-017, NEI-UI-001; actor-models Unbound 0.9 na mensagem. RFC 0026 cobre randomizer/hooks; RFC 0025 é base do menu. Demonstração de insuficiência para DMR/montagem específica e RFC própria: não confirmado. | G(`a760458480442bb6bcaddbce2401de48b6960dd5`); H/BIND-002.md:15; H/OOT-NEI-017.md:18,65-72; H/OOT-RELEASE-001.md:227,266-267; CH/SKIJER-001.md:20,32; CH/NEI-UI-001.md:12,19; R0026; mensagem |
| `4c45d5c3a` | 2026-10-02 | `soh/soh/ShipLuaBootstrap.cpp`<br>`soh/soh/native/OotNativeActorModels.cpp` | **D** — Impede camada de pacote rejeitado de sombrear o VFS; permite reuso de tipo de ator inativo. Corrige a dívida C do bootstrap introduzida em a76045848, sem a generalizar. | Não pelo diff; id individual não confirmado; release 9a593a6e. | OOT-RELEASE-001, revisão do Codex. RFC 0026 citada é do randomizer, não formaliza estas correções. | G(`4c45d5c3aab528f3c30f464a8232107829371542`); H/OOT-RELEASE-001.md:73,103,235; mensagem |
| `4ae2fc4ed` | 2026-10-03 | `soh/soh/ShipLuaBootstrap.cpp` | **D** — Corrige UTF-8 do aviso no bootstrap. Há também tooling de release fora do filtro do host (E). | Não pelo diff de host; id individual não confirmado; release 9a593a6e. | OOT-RELEASE-001: correção de revisão/empacotamento; RFC específica não confirmada. | G(`4ae2fc4ede5c4ee68e1a80cb73f7b3681c7bd3bb`); H/OOT-RELEASE-001.md:73-74; mensagem |
| `fc2709989` | 2026-10-03 | `soh/soh/ShipLuaBootstrap.cpp` | **D** — Retira 95 linhas de sondagem de áudio MM que disparavam sem mod; restaura ausência de efeito de teste. | Não pelo diff; release final 9a593a6e (H/OOT-RELEASE-001:27). | OOT-RELEASE-001; origem OOT-AUDIO-001. Remoção pedida no handoff e explicada na mensagem; RFC específica não confirmada. | G(`fc27099890eb204991ef07c3c7e38bf3462ab541`); H/OOT-RELEASE-001.md:54,78,216; H/OOT-AUDIO-001-port-mm-sfx.md; mensagem |
