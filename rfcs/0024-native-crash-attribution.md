# RFC 0024 — Atribuição de crash e proteção de boot

Status: implementação experimental COREEXT-009.

## Motivação

Com hooks (RFC 0021) e escape hatch (RFC 0023), um mod nativo roda dentro do
fluxo do jogo e pode derrubá-lo. O crash log do host mostrava registradores e
pilha, sem dizer qual mod estava ativo. Um core extension que cai no próprio
init derrubava todos os inícios seguintes até o usuário achar e remover o
arquivo certo.

## Decisão

### Atribuição

- `NativeActivityScope` registra, por thread, o mod e a fase: `init`, `call`
  (com o nome da função), `hook` (com o ponto) e `shutdown`. O `NativeProvider` e
  o `NativeHookRegistry` abrem esses escopos.
- `RegisterNativeModule` guarda base e tamanho de cada biblioteca carregada numa
  tabela fixa de 64 posições.
- `WriteNativeCrashReport`, chamado pelo crash handler na thread que falhou, sem
  alocar, escreve:
  - a pilha de atividades da thread;
  - os mods cujas bibliotecas aparecem na pilha capturada;
  - os módulos carregados.

  Isso cobre também detours do escape hatch, que não passam por escopo.
- O culpado é o mod da biblioteca mais interna na pilha. Sem frame de mod, vale a
  atividade mais interna.

### Proteção de boot

`ModHost::EnableBootGuard(pasta)` liga dois arquivos na pasta de mods:

| Arquivo | Conteúdo |
|---|---|
| `.shiplua-boot` | `loading<TAB>id` antes de carregar cada mod nativo, `running` depois; `crashed<TAB>id` quando o crash handler aponta um culpado |
| `.shiplua-disabled` | uma linha `id<TAB>motivo` por mod desativado |

- No início seguinte, `loading` ou `crashed` com id acrescenta o mod à lista, e
  `LoadModsFromRoot` recusa os ids listados com o motivo. Dependentes caem pela
  regra normal de dependência.
- `ConfirmBoot()` apaga o marcador e desliga a escrita pelo crash handler; crashes
  depois disso só aparecem no log.
- O host OoT confirma depois de 300 frames de `OnGameFrameUpdate`.
- Para reativar, o usuário apaga a linha do mod em `.shiplua-disabled`.

## Limites

- Estouro de pilha e corrupção de heap podem impedir o crash handler de rodar.
  Nesse caso vale só o marcador `loading`, que cobre o init.
- Um crash na thread de áudio ou de save sem atividade registrada, e sem DLL de
  mod na pilha, não aponta culpado.
- Fechar o jogo à força durante o init de um mod o desativa. É proposital: não
  há como distinguir isso de um travamento.
