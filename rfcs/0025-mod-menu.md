# RFC 0025 — Menu declarativo por mod

Status: preview, MENU-001 (W-1). API 0.5.0, capability `core.menu`.

## Motivação

Mods precisam expor configuração no menu do jogo sem editar o `SohMenu`, sem
depender do ImGui e sem CVars. `set_setting_int` termina em `CVarSetInteger`, e
o SoH grava a configuração a toda hora: a chave do mod sobrevive ao unload no
`shipofharkinian.json`. Aqui os valores de um mod vivem só no `ship.storage`
dele.

O primeiro consumidor é o Wind Waker Style (CEL-004). A DLL faz o render; o Lua
declara as páginas e repassa a configuração à DLL com `ship.native.call`. O host
só conhece widgets genéricos.

## Contrato Lua

```lua
local ship = require("ship")

ship.menu.register({
  id = "cel_shading", title = "Wind Waker Style", sidebar = "Cel Shading",
  columns = 2,
  widgets = {
    { id = "enabled", type = "checkbox", label = "Enable Cel Shading",
      column = 1, storage_key = "toon.enabled", default = true,
      on_change = function(value)
        ship.native.call("configure", '{"toon_enabled":' .. tostring(value) .. '}')
      end },
    { id = "ramp_center", type = "slider_number", label = "Ramp Center",
      column = 2, storage_key = "toon.ramp_center", default = 0.5,
      min = 0.0, max = 1.0, step = 0.01, format = "%.2f",
      visible_if = { key = "toon.enabled", equals = true } },
    { id = "reset", type = "action", label = "Reset to Defaults", column = 1,
      on_activate = function() ship.storage.delete("toon.ramp_center") end },
  },
})
```

`register(page)` valida a tabela inteira antes de publicar; uma página inválida
levanta erro e deixa a anterior intacta. Um registro válido substitui a página
de mesmo `(mod, id)`, ganha uma geração nova e devolve `true`.

- **IDs** de página e widget: `[a-z][a-z0-9_.-]{0,63}`.
- **Página:** `id`, `title`, `sidebar`, `columns` de 1 a 4 (padrão 1) e até 256
  widgets. `column` do widget vai de 1 a `columns` (padrão 1).
- **Widgets:**

  | `type` | Campos próprios | Evento |
  |---|---|---|
  | `checkbox` | `storage_key`, `default` booleano | `on_change(value)` |
  | `slider_int` | `storage_key`, `min`, `max`, `step`, `format` | `on_change(value)` |
  | `slider_number` | `storage_key`, `min`, `max`, `step`, `format` | `on_change(value)` |
  | `choice` | `storage_key`, `choices = {{value, label}, ...}` | `on_change(value)` |
  | `action` | nenhum | `on_activate()`, sem argumentos |
  | `text` | `label` | nenhum |
  | `separator` | `label` opcional | nenhum |

- **Campos comuns:** `id`, `type`, `label`, `tooltip`, `column`, `visible_if` e
  `enabled_if`.
- **Widgets com valor** (`checkbox`, sliders e `choice`) exigem `storage_key`.
  Sem `default`, valem `false`, o `min` (arredondado para cima no `slider_int`) ou
  a primeira opção. Um `default` fora do widget (tipo errado, fora da faixa,
  opção inexistente) recusa o registro.
- **Sliders** exigem `min < max` e `step >= 0` (0 = contínuo).
- **`choice`:** de 1 a 64 opções; o valor pode ser booleano, inteiro, número ou
  texto.
- **Predicados:** `{key, equals, not?}`, lidos só do storage do próprio mod.

## Fluxo

1. O desenho do menu só lê cópias das páginas e enfileira `MenuInput`; nunca
   entra em Lua nem toca o storage.
2. `ModHost::PumpMenu()` roda na thread do jogo, uma vez por quadro. Para cada
   mod, drena os inputs e descarta os que:
   - apontam página ou widget desconhecidos ou uma geração antiga;
   - vêm de widget invisível ou desabilitado;
   - trazem valor que não cabe no widget (tipo errado, opção inexistente).

   Número fora da faixa é limitado a ela. O valor aceito vai para o storage, e
   o callback roda no `lua_State` do mod. `Activate` só vale para `action`.
3. A página só é republicada quando algo mudou: input do menu, novo `register`,
   ou `ship.storage.set`/`delete`/`clear` do próprio mod. Sem mudança, o pump só
   drena a fila.
4. Um callback que falha 3 vezes seguidas é desligado, com aviso no log. Um
   sucesso zera a contagem.
5. Durante o arraste de um slider, `on_change` roda a cada valor novo, no máximo
   uma vez por widget entre dois pumps.

## Capabilities

`core.menu`, status `contract`, só no host OoT por enquanto.
`ship.menu.register` é do núcleo (`availability: common`): sem os adaptadores,
levanta `unsupported`. Um mod portátil testa `ship.capabilities.has("core.menu")`
antes de registrar. Anunciar `core.menu` sem `MenuRegistry` e `KeyValueStorage`
no contexto é `invalid_state` na instalação da API.

## Comportamento em OoT

- **`SohMenuModRegistry`** guarda páginas e inputs sob mutex. A fila aceita até
  1024 inputs, e um `Change` do mesmo widget substitui o que ainda espera.
- **Onde aparece:** o `SohMenu` ganha a entrada `Mods`, com a barra lateral
  `Installed Mods`. Cada página vira um cabeçalho recolhível `title - sidebar`,
  com uma tabela de `columns` colunas. Nesta versão o campo `sidebar` não cria
  uma entrada própria na barra lateral: o menu do SoH monta as barras no início.
- **Slider em arraste** mostra o valor local até a página publicada alcançá-lo
  (no máximo 30 quadros de UI depois de soltar), para não tremer entre o desenho
  e o pump.
- **Formatos aceitos** (o resto cai no padrão):
  - `slider_int`: `%d` e `%lld` (padrão), ou `%d%%`;
  - `slider_number`: `%.0f` a `%.3f` (padrão `%.3f`) e `%.0f%%` a `%.2f%%`.
- **`ShipLuaBootstrap`** cria o registro, anuncia `core.menu` quando há storage e
  chama `PumpMenu()` no `OnGameFrameUpdate`, antes dos timers e de `game.frame`.
- **Persistência:** os valores ficam no `shiplua-storage.bin` (gravado a cada
  ~5 s e no encerramento). Nenhum widget usa CVar; a única CVar nova é a da seção
  `Mods` na navegação do menu.
- **Carona:** `shiplua_open_file` recusa um slot vazio em vez de lançar
  `std::filesystem_error`.

## Comportamento em MM

O host MM ainda não tem adaptador. Não anuncia `core.menu`, `register` levanta
`unsupported`, e nenhuma página aparece. Um adaptador futuro implementa o mesmo
SPI, com os mesmos limites, dono e ordem de unload. Não há contrato alternativo
nem ABI C exclusiva de MM.

## Erros

`register` levanta erro Lua:

- `unsupported` sem os adaptadores;
- mensagem descritiva para página inválida: tabela, id, tipo, faixa, coluna,
  opções, predicado, callback, `default` fora do widget, mais de 256 widgets ou
  64 opções;
- falha ou exceção do adaptador.

Um callback com erro vai para o log com o id do mod e o traceback. O pump segue
para os outros widgets e mods, e o callback é desligado depois de 3 falhas
seguidas. Input fora das regras do Fluxo é descartado sem erro.

## Compatibilidade

A superfície é aditiva e protegida por capability. A API continua em `0.5.0`,
ainda não publicada, e a função entra nela como `preview`. Se a `0.5.0` sair
antes do merge, schemas, gerados e ranges dos fixtures sobem para `0.6.0` numa
mudança mecânica separada.

Não há ABI C nova. `MenuRegistry.h` fica em `include/shiplua/menu`, fora do glob
do layout id do SDK OoT: esta RFC sozinha não troca o layout.

## Segurança

- O dono da página é preenchido pelo binding; o Lua não escolhe `mod_id`.
- O desenho nunca chama callbacks. O unload desliga os callbacks, remove páginas
  e inputs do mod e só então solta as referências Lua.
- Inputs de geração antiga são descartados, o que impede uso depois de
  substituição ou unload.
- IDs, textos, contagens e números são limitados; números não finitos são
  recusados.
- Formatos do ImGui vindos do Lua passam por uma lista curta no host, separada
  por tipo.
- Predicados só leem o storage do próprio mod.
- Nenhum ponteiro ou referência do ImGui chega ao Lua ou à DLL. DLLs seguem o
  contrato de `ship.native.call`.

## Plano de testes

Unitários do core:

- `menu_api_tests`, com quatro casos:
  - `RegisterPumpAndUnload`: página publicada, predicado lido do storage, input
    persistido, callback no pump, unload limpo;
  - `UnsupportedAdapter`: MM sem adaptador dá erro capturável;
  - `ValidationAndFailures`: registros inválidos recusados, `default` inteiro de
    slider numérico vira número, `choice` sem `default` pega a primeira opção,
    input de tipo errado ou de opção inexistente descartado, número limitado à
    faixa, `on_activate` sem argumentos e desligado após 3 falhas, pump sem
    mudança sem republicar, storage gravado pelo mod refletido no pump seguinte;
  - `ReRegisterFromCallback`: um `on_change` que registra a própria página de novo e falha não deixa o pump
    usar o callback substituído; input da geração antiga é descartado;
- `api_contract_tests`: o harness ganha um registro falso no OoT;
- checks de geração: `api_schema_validation`, `api_cpp_codegen_check`,
  `api_docs_codegen_check`, `api_contract_codegen_check`.

Roteiro em jogo (OoT), com um mod Lua de teste:

1. `Mods > Installed Mods` mostra a página, com colunas e tooltips.
2. Checkbox, sliders e choice mudam o valor; o callback roda; os predicados
   escondem e desabilitam widgets.
3. Os valores voltam depois de reiniciar o jogo (`shiplua-storage.bin`) e não
   aparecem no `shipofharkinian.json`.
4. Um callback que sempre falha é desligado depois de 3 tentativas.
5. Descarregar o mod com o menu aberto remove a página sem crash nem callback
   residual.
6. `shiplua_open_file` num slot vazio devolve erro; num slot válido abre o jogo.

## Depreciação

Nenhuma API existente sai. Enquanto a função for `preview`, o formato da tabela
pode mudar num minor, registrado nesta RFC. Ao estabilizar, vale a regra normal:
deprecated por ao menos um minor e remoção só em major. O menu C++ próprio do
Wind Waker Style pode coexistir durante a migração e sai do fork depois da
paridade.
