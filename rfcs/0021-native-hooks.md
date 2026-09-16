# RFC 0021 — Hooks nativos (Core Extension ABI 1.2)

Status: implementação experimental COREEXT-006.

## Motivação

A ABI 1.1 só deixa um mod chamar o host (serviços) ou ser chamado pelo Lua
(funções). Frameworks como Not Enough Items mudam o comportamento de dezenas de
atores, do jogador e do pause menu; sem um ponto em que o mod entra no fluxo do
jogo, cada mudança viraria uma função nova compilada no host. O plano
`OOT-COREMODS-001` (§8.3) exige hooks `observe`, `transform` e `replace` com
fase, prioridade, dono e política de conflito.

## Decisão

A ABI passa a 1.2, aditiva. `ShipNativeRuntime` ganha quatro callbacks no fim:

| Callback | Quem pode | Quando |
|---|---|---|
| `register_hook` | qualquer provider | init ou depois, na thread do jogo |
| `unregister_hook` | dono do hook | idem |
| `declare_hook_point` | core extension | só no init |
| `dispatch_hook` | dono do ponto | na thread do jogo |

Um ponto tem nome, versão, tamanho exato do payload e a máscara de modos aceitos.
O hook declara modo único, fase (`BEFORE`/`AFTER`, só observe), prioridade e o
tamanho mínimo de payload que conhece.

Ordem de um despacho:

1. `observe BEFORE` por prioridade;
2. `transform` em cadeia por prioridade, cada um com snapshot do payload;
3. o `replace` exclusivo, que recebe `call_original` (no máximo uma chamada), ou a
   operação original;
4. `observe AFTER`.

Empate de prioridade segue a ordem de registro, que segue a ordem de carga.

## Falhas e conflito

- `OK` conclui; `UNSUPPORTED` é recusa voluntária e não conta como falha.
- Outro status ou exceção conta; três seguidas desativam o hook, com log.
- `transform` que não conclui tem o payload restaurado.
- `replace` que não conclui e não chamou o original faz o host rodar o original.
- Segundo `replace` no mesmo ponto recebe `LIMIT`, com log que nomeia o dono atual.

## Lifetime

- O host cria um `NativeHookRegistry` e o passa em `NativeProviderPolicy::hooks`.
- O `NativeProvider` remove os hooks e pontos do seu mod antes de chamar
  `shutdown`, inclusive no rollback de init.
- Pontos de uma core extension somem no unload dela; hooks de outros mods nesses
  pontos saem junto, com log.
- Registrar ou remover durante um despacho é permitido: a remoção vale na hora,
  o registro entra no próximo despacho.

## Compatibilidade

Providers 1.0 e 1.1 continuam carregando; recebem a runtime maior e ignoram os
campos novos. Uma DLL que declara `abi_minor` 2 é recusada por hosts 1.1. Core
extensions aceitam `abi_version` "1.1" ou "1.2". Mudar a ABI muda o layout id dos
hosts que o calculam sobre `shiplua/native/*.h`.

## Fora do escopo

- Desvio de funções arbitrárias do executável (escape hatch, COREEXT-008).
- Hooks para Lua.
- Pontos específicos de cada jogo: o host OoT declara `oot.play.update`,
  `oot.actor.update` e `oot.actor.draw` v1 em `soh/soh/native/oot_hooks.h`.

## Testes

- `native_hook_tests`: ordem, modos, falhas, mutação durante despacho,
  despacho aninhado e remoção por dono.
- `native_provider_tests`: uma core extension ABI 1.2 declara e despacha um
  ponto, e um mod comum registra transform, observe e replace. Os testes também
  cobrem o unload dos dois e a recusa de `declare_hook_point` fora de core
  extension.
