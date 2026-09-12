# RFC 0014 — Movimentação Dinâmica Remake no OoT

## Motivação

Entregar como mod do Link-Span um perfil moderno para controles Nintendo. A
primeira fatia separa X para pulo e A para contexto, rolamento e sprint sem
cadastrar essa mecânica no modloader.

## Contrato nativo

`linkspan.oot.engine` v1 permanece sem alterações e fornece o acesso nativo já
existente. O novo serviço `linkspan.oot.movement` v1 acrescenta:

- leitura de input virtual, botões físicos SDL e analógicos por porta;
- bindings SDL transitórios com restauração no unload;
- leitura e escrita transitória de settings inteiros do host;
- consulta de chão e rolamento;
- `player_jump(vertical_velocity)`, aceitando valores finitos em `(0, 30]`;
- `player_roll()`.

Os nomes separados atendem à regra de serviço único do runtime e preservam os
providers antigos. O provider novo exige os dois serviços e o fingerprint exato
dos layouts usados na compilação.

## Contrato do mod

O perfil `nintendo` fica no `main.lua`. No Switch Pro, B/A/Y/X físicos chegam
como SDL A/B/X/Y. O mod liga A físico ao N64 A contextual, Y e B ao N64 B e lê X
diretamente para pular.

O mod habilita `gSettings.FreeLook.Enabled` enquanto está carregado e libera os
quatro atalhos C do analógico direito somente em memória. No unload, restaura o
setting anterior e recarrega os mappings persistidos. Quatro segundos sem entrada
do analógico direito desativam o free look; uma nova entrada fora da zona morta
reativa a câmera livre e reinicia o cronômetro.

Estados determinísticos:

```text
X: idle -> jump -> airborne -> landed
A: idle -> roll -> rolling -> run -> running
       ^                              |
       +---------- soltar A ----------+
```

A corrida impõe velocidade mínima `8.0` somente quando o analógico esquerdo está
fora da zona morta de 20 unidades. Pulo é recusado fora do chão, na água, a
cavalo, morto ou em cutscene. Rolamento usa a ação nativa do Player.
O pacote de referência usa impulso vertical `7.0`, validado em runtime para
aterrissar normalmente antes do rolamento. O host escolhe a animação nativa
`link_normal_jump` parado ou `link_normal_run_jump` ao correr e reproduz a voz
nativa de auto-jump.

## Compatibilidade

- host: Shipwright 9.2.3 Windows x64;
- runtime/API Lua: Link-Span 0.5;
- ABI do provider: 1.0;
- serviços OoT: `linkspan.oot.engine` v1 e `linkspan.oot.movement` v1.

## Segurança e erros

O serviço só opera na thread do jogo. Portas inválidas retornam input neutro.
Velocidade inválida retorna `SHIP_NATIVE_INVALID_ARGUMENT`; estado de gameplay
incompatível retorna `SHIP_NATIVE_UNSUPPORTED`. Ponteiros não podem ser retidos
entre frames ou cenas.

## Testes

- engine v1 e movement v1 publicados simultaneamente com nomes únicos;
- input, chão, pulo e rolamento na tabela real do host;
- provider compilado separadamente;
- X físico separado de A/Y/B, bindings restaurados no unload;
- câmera livre ativada pelo mod e setting anterior restaurado no unload;
- mappings persistidos inalterados após execução real;
- sequência A rolamento/sprint e rearme após soltura;
- build Release e playtest com pacote ZIP.

## Evolução do perfil

As próximas fatias ativam item em ZR, Quick Swap em R, escudo em L,
Ocarina/Navi, menu/mapa e seletores de Gear/Boots por
toque/pressão longa. Cada ação permanece no pacote do mod e reutiliza
primitivas gerais do host.
