# RFC 0019 — Serviço de ocarina do OoT

Status: implementação experimental OOT-MIC-001.

## Motivação

O Mic Ocarina reconhece pelo microfone a música que o jogador cantarola. Para
isso o mod precisa:

- saber quando o jogo espera uma música e quais ela aceita;
- ler o padrão de notas de cada música;
- entregar o resultado ao fluxo nativo, que aplica efeito, texto e warp.

Tudo sem abrir áudio no host nem simular botões nota a nota.

## Decisão

O host publica `linkspan.oot.ocarina` v1 (`soh/soh/native/oot_ocarina.h`) com
`is_active`, `get_available_song_flags`, `get_song_count`, `get_song_pattern` e
`submit_song`.

- `func_800EE6F4`, o update de input da ocarina chamado por `func_800F3054` na
  thread do jogo (`graph.c:393`), publica o estado a cada update. Ele fica ativo
  com instrumento, input habilitado e `sOcarinaFlags` armado; as flags são as de
  `sAvailOcarinaSongFlags`.
- Os padrões vêm de `gOcarinaSongNotes` com os índices de nota do jogo (0 a 4 =
  D4, F4, A4, B4, D5). A música do Espantalho entra no catálogo quando gravada.
- `submit_song` guarda uma música pendente. No update seguinte, `func_800EE6F4`
  confere de novo se a ocarina segue ativa e a música aceita, e aplica o
  resultado do reconhecimento nativo (`D_80131878 = música + 1`, input desligado,
  `sOcarinaFlags = 0`, como em `code_800EC960.c:1635-1638`). A entrega é
  descartada se a ocarina fechar antes.

## Compatibilidade

Serviço novo, sem alterar os existentes. O header entra no `LINKSPAN_OOT_LAYOUT_ID`,
por isso mods nativos que conferem o layout precisam ser recompilados contra o SDK
do host novo. Um provider que só consulta serviços por tabela, como o Mic
Ocarina, não depende do layout.

## Limites

- somente na thread do jogo; estado em `std::atomic`;
- sem notas individuais nem jogo da memória (`OCARINA_SONG_MEMORY_GAME`);
- o microfone, o detector e as dependências de áudio pertencem ao mod;
- música fora das flags do momento é recusada com `SHIP_NATIVE_UNSUPPORTED`.

## Validação

- serviço publicado na policy junto de engine, movement, resources e registry;
- consulta de tamanho, `SHIP_NATIVE_LIMIT` e cópia dos índices de nota;
- recusa de padrão vazio, id fora do catálogo, ocarina fechada, música fora das
  flags e thread externa;
- música pendente consumida uma única vez e descartada ao fechar a ocarina;
- matcher do mod reconhece contorno transposto, ignora música não aceita, recusa
  ruído e tolera nota espúria antes da frase.
