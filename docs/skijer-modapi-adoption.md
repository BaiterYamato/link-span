# Aprendizados do ModAPI do Skijer para o Link-Span OoT

Fonte analisada: [`skijer/Shipwright`, branch `unbound-modapi`, commit
`b48a5b7`](https://github.com/skijer/Shipwright/tree/b48a5b7da7e3ec94c49bc613270de2567d8b0961/soh/soh/ModApi)
(25/09/2026). O código foi consultado para comparar contratos; este trabalho
não copia as implementações do fork nem os seus assets.

## Ideias adotadas

| Contrato | Skijer | Link-Span OoT |
|---|---|---|
| Assets obrigatórios | `manifest.json` contém `requires`; `ResourceMgr_FileExists` é consultado no início | `manifest.toml` contém `[resources] required`; o host consulta `HasFile` depois de montar os archives e antes de carregar a DLL ou executar Lua. A checagem ocorre em cada início. |
| Identidade do item | Chave textual persiste; `runtimeId` representa a sessão | NEI usa chave namespaced e traduz ID lógico da página para ID runtime do botão. Os ícones do inventário e do HUD usam resolvedores distintos. Se o botão C contém item nativo de outro mod, o fork entrega o ID runtime ao host. |
| Colisão de registro | Chaves repetidas são recusadas em item/equipamento/forma | Registro genérico V2 rejeita chave e ID repetidos, informa o dono e remove as entradas do mod ao descarregar, inclusive após falha de inicialização. |
| Mod independente | DLL e assets dentro de O2R | ZIP Link-Span com DLL e manifesto separados da build do host; `shovel-demo` usa apenas serviços públicos do OoT. |

## Limites da adoção

- O Skijer usa hooks C++ internos do Shipwright e IDs de item de 16 bits; o
  Link-Span mantém ABI C versionada e o NEI conserva a própria página. Copiar
  `CustomItemRegistry` inteiro criaria uma segunda fonte de identidade e de save.
- O registro V2 exige que o provider use seu ID como `owner`. Código nativo já
  tem acesso privilegiado ao processo; o campo de dono serve a ciclo de vida e
  diagnóstico, não a isolamento de segurança.
- A API de formas do Skijer substitui ações, animações, modelo e equipamento do
  Player. Ainda é necessário definir um contrato próprio, de callbacks e
  restauração de estado, e provar transições de cena, save e morte no OoT antes
  de oferecer esse serviço a mods. A mesma regra vale para equipamentos
  personalizados com página própria no pause menu.
- A fonte analisada contém o framework, mas não a fonte dos mods Shovel/O2R
  encontrados na instalação binária. `shovel-demo` é uma prova original e
  pequena, com modelo e ícone vanilla provisórios.

## Gate de integração

O aceite exige, em runtime isolado de OoT: ícones corretos na página NEI e nos
botões, equipar como Link criança, uso após troca de cena e save/reload, recusa
de asset faltante antes de `ShipNative_Query`, conflito de chave sem entrada
parcial, e descarregamento que libere a chave para nova carga. Build, validator
e testes unitários não substituem essa observação em jogo.

No layout `c97ff67f`, NEI, `shovel-demo`, Unbound Framework, Unbound Scene Demo
e Wind Waker Style carregaram juntos no runtime isolado. Link criança equipou
a Deku Leaf e a pá; o ícone da folha apareceu corretamente na página NEI e no
HUD. O Unbound registrou a conversão de 101 cenas, 388 salas, 101 colisões e
143 paths sem falhas, a transição para a cena 128 e o uso posterior da pá
(`attempts=1, digs=1`). Após salvar pelo menu, fechar o jogo e recarregar
`File1`, os dois itens continuaram nos botões; um novo uso da pá registrou
`attempts=1, digs=1`. A captura `reload-final-nei-leaf.png` mostra também a
Deku Leaf com nome e ícone corretos no inventário após a recarga.

Um pacote diagnóstico com `required` apontando para
`objects/nonexistent/gMissingSkijerGateDL` foi aceito pelo validator, mas o
host o recusou no início do jogo antes de tentar carregar sua DLL inválida ou
executar sua Lua. Os outros cinco mods carregaram normalmente. O pacote foi
retirado de `mods` ao terminar o teste. Os 61 testes do core, os três testes
do conversor Unbound, o teste de WW Style e o teste do registro OoT passaram.

Evidências locais: `build/oot-upstream-001/smoke-oot-combined-c97ff67f-20260927/evidence/`
(`five-mod-field.png`, `five-mod-reload-gameplay.png`,
`reload-final-nei-leaf.png`, `Ship of Harkinian.combined-c97ff67f-20260927.post.log`,
`Ship of Harkinian.asset-negative-c97ff67f-20260928.post.log`).

Permanece sem observação em jogo o cenário de colisão de chave seguido por
descarregamento e recarga na mesma sessão. O teste de registro cobre a recusa
sem entrada parcial, a remoção por dono e o reuso de chave e ID. O log do jogo
mostra o descarregamento dos cinco mods no encerramento, mas isso por si só não
prova o reuso do registro em uma recarga na mesma sessão. O host OoT ainda não
oferece uma operação de recarga em partida; esse último gate depende dela.
