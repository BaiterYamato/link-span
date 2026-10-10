# RFC 0022 — Save por namespace no OoT

Status: implementação experimental OOT-CORE-006.

## Motivação

Mods nativos precisam guardar estado no arquivo de save: itens do NEI, flags de
cenas novas do Unbound, progresso de mods pequenos. O SaveManager do Shipwright
aceita seções, mas registrar uma por mod exigiria código no host e deixaria o
dado preso ao id da seção. As flags das cenas de mod (RFC 0020) também só
existiam durante a sessão.

## Decisão

O host registra uma única seção `linkspan` (versão 1) e publica
`linkspan.oot.save` v1 (`soh/soh/native/oot_save.h`):

```json
"linkspan": { "version": 1, "data": { "namespaces": {
  "autor.mod": { "version": 2, "required": true, "data": { "qualquer": "JSON" } }
} } }
```

- Namespace é um nome com ponto (`[A-Za-z0-9._-]`, até 128 bytes). O prefixo
  `linkspan.` pertence ao host: `linkspan.scenes` guarda as flags das cenas de mod
  pelo nome da cena.
- O mod abre o namespace com a versão do seu schema; `write` grava qualquer JSON
  de até 1 MiB e marca `version`; `get_stored_version` permite migrar.
- A escrita vale na memória e chega ao disco no próximo save do jogo. O save roda
  em outra thread: o store tem mutex e o host tira o snapshot das flags na thread
  do jogo, antes de `SaveSection` copiar o `SaveContext`.
- `begin`/`commit`/`rollback` por namespace; durante a transação o save grava o
  estado anterior.
- `set_required` marca a dependência; carregar o arquivo sem mod que abra o
  namespace gera aviso no log.

## Eventos

Em vez de callbacks no serviço (que sobreviveriam ao unload da DLL), os eventos
são pontos de hook observe da RFC 0021, removidos junto com o mod:

| Ponto | Quando |
|---|---|
| `oot.save.loaded` | depois de carregar um arquivo |
| `oot.save.saving` | antes de gravar a seção base |
| `oot.save.deleted` | arquivo apagado |
| `oot.save.copied` | arquivo copiado (`other_slot` → `slot`) |

## Ciclo de vida dos dados

- `InitFile` (jogo novo e início de toda carga) esvazia os blocos e as flags.
- Carregar importa a seção; entradas malformadas são ignoradas.
- Blocos de namespaces que nenhum mod abriu voltam ao arquivo no próximo save:
  remover e reinstalar um mod não perde o dado.
- Copiar e apagar arquivos já levam a seção inteira, porque o SaveManager copia
  o JSON do arquivo.

## Fora do escopo

- Recusar a carga de um save com dependência ausente (hoje é só log).
- Dados por slot fora do arquivo de save.
- API Lua.
