# RFC 0015 — Core extensions e serviços entre mods

Status: implementação experimental COREEXT-001. ABI C 1.1, compatível com 1.0.

## Motivação

Mods estruturais como Unbound e Not Enough Items precisam ampliar o motor sem
incorporar cada item, registry ou mecânica ao Link-Span e sem recompilar o jogo a
cada atualização. O provider nativo 1.0 já carrega código independente, mas só
consulta serviços do host. Falta carregar uma extensão estrutural cedo e deixar
que ela publique contratos para outros mods.

## Manifesto

Dois campos top-level opcionais são adicionados:

```toml
kind = "core_extension"
load_phase = "pre_game"

[provider]
abi_version = "1.1"
win64 = "provider/core.dll"
```

Os defaults permanecem `kind="mod"` e `load_phase="runtime"`. Uma core extension
exige provider ABI 1.1 e fase pre_game. A fase pre_game é reservada a core
extensions. Core extensions carregam antes de mods runtime; dependências e hints
que contradizem essa fronteira são rejeitados pelo resolver.

## ABI C 1.1

`ShipNativeRuntime` recebe ao final da struct:

```c
ShipNativeStatus (*register_service)(context, name, version, size, table);
```

O registro só é válido durante `init`, para `core_extension`, com nome não vazio,
versão e tamanho não zero e tabela não nula. Nome e versão formam a identidade.
A tabela pertence à extensão e deve permanecer válida até `shutdown`. O loader
não interpreta seu conteúdo e não mantém catálogo de mecânicas.

`get_service` passa a consultar tanto serviços do host quanto serviços publicados
por core extensions já carregadas. A consulta continua exata por nome/versão e
aceita `minimum_size` para evolução aditiva de tabelas.

## Ordem, rollback e lifetime

O root loader resolve toda core extension saudável antes dos mods runtime. Um
serviço só entra no registry global depois que provider e entrypoint Lua carregam
com sucesso. Falha de init ou Lua não deixa serviço residual.

Cada serviço distribuído aos providers carrega internamente o id do owner.
Unload individual da extensão é recusado enquanto houver mod dependente por
manifesto ou provider vivo com acesso à tabela. Isso cobre consultas tardias e
ponteiros armazenados pelo consumidor. `UnloadAll` usa ordem inversa de carga.
Serviços do host não têm owner de mod e seguem com lifetime controlado pelo host.

## Compatibilidade

A ABI major permanece 1. Providers 1.0 continuam funcionando porque o callback
novo foi anexado ao fim da struct e a negociação usa size/major/minor. Manifestos
com `[provider] abi_version="1.0"` continuam válidos para mods normais. Apenas
core extensions exigem 1.1. Nenhuma API Lua existente é removida.

## OoT e MM

O contrato pertence ao core compartilhado e não inclui headers de jogo. O corte
inicial é validado sem ROM e será integrado primeiro ao Shipwright/OoT. A migração
de Unbound e NEI dependerá de serviços OoT amplos publicados pelo adaptador. MM
fica adiado até o gate funcional completo no OoT, conforme decisão do usuário.

## Segurança e falhas

Core extensions são código nativo confiável no mesmo processo, sem sandbox. A
política `NativeProviderPolicy.enabled/authorize` continua sendo a fronteira de
confiança. O manifesto não se autoriza. Registro duplicado ou inválido falha de
forma determinística; exceções não atravessam a fronteira C; tabela não pode ser
usada depois do unload de seu owner.

## Plano de testes

- provider ABI 1.0 continua carregando;
- core ABI 1.1 publica serviço e consumidor obtém/chama a tabela;
- core carrega antes do consumidor mesmo com ordem lexical contrária;
- unload do core é bloqueado até unload do consumidor;
- serviço duplicado rejeita apenas a extensão conflitante;
- falha do entrypoint da extensão não deixa serviço disponível;
- suíte nativa e root loader passam no Windows sem ROM.

## Fora deste incremento

Registries concretos de cenas, mensagens, itens, atores e assets; patching de
código; hot reload com referências ativas; migração do código Unbound/NEI; port
para MM. Esses contratos serão construídos sobre esta base, sem funções dedicadas
por mod no loader.
