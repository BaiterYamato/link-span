# RFC 0023 — Escape hatch nativo (ABI 1.3)

Status: implementação experimental COREEXT-008.

## Motivação

Os hooks da RFC 0021 cobrem pontos que o host declara. Portar forks como o Not
Enough Items envolve mudanças em dezenas de funções do jogo, várias delas
`static`, que não valem uma API própria cada uma. O plano `OOT-COREMODS-001` §8.4
prevê um caminho de liberdade total, restrito a core extensions e preso ao
executável exato.

## Decisão

A ABI passa a 1.3, aditiva. `ShipNativeRuntime` ganha no fim:

| Callback | Quem pode |
|---|---|
| `get_host_fingerprint` | qualquer provider |
| `resolve_symbol` | core extension com fingerprint aceito |
| `install_patch` | idem |
| `remove_patch` | idem, só patches próprios |

O manifesto de uma core extension ABI 1.3 declara os executáveis aceitos:

```toml
[provider]
abi_version = "1.3"
win64 = "provider/framework.dll"
host_fingerprints = ["<sha256 do soh.exe>"]
```

O fingerprint é o SHA-256 do executável, calculado pelo host. Sem a lista, ou
num executável diferente, as chamadas retornam `UNSUPPORTED`; o mod deve cair
num caminho sem patch ou recusar o próprio init. O parser recusa
`host_fingerprints` em mod comum ou em ABI diferente de 1.3.

## Divisão de responsabilidade

- **Core** (`NativeProvider`): permissão por manifesto e fingerprint, registro
  dos patches por mod e remoção em ordem inversa antes do `shutdown`, inclusive
  no rollback de init.
- **Host** (`NativeProviderPolicy::escapeHatch`): cálculo do fingerprint,
  resolução de nomes e o mecanismo de desvio. O host OoT usa um arquivo de
  símbolos gerado do PDB no build e a MinHook v1.3.4 (BSD-2).

## Nomes de símbolo

`resolve_symbol` aceita `Funcao` quando o nome é único no executável e
`arquivo.c!Funcao` para funções `static` com nome repetido. Nome ausente ou
ambíguo retorna `UNSUPPORTED`. Funções inlined pelo compilador não têm endereço
próprio e não aparecem.

## Riscos aceitos

- Não há sandbox: o detour roda com os privilégios do jogo.
- Todo patch depende do binário exato; atualizar o host exige publicar o mod com
  o fingerprint novo.
- Remover um patch enquanto outra thread executa a função original é indefinido.
  O host remove no unload do mod, que acontece na thread do jogo.
- Dois mods no mesmo alvo: o segundo recebe `LIMIT`; não há encadeamento.

## Testes

`native_provider_tests` usa um host falso de patches. Ele cobre o fingerprint
aceito (patch instalado e removido no unload), o fingerprint recusado, mod comum
e host sem escape hatch, além das recusas do parser.
