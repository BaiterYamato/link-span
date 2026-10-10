[English](Home) · **Português (Brasil)**

# Wiki do Link-Span

O Link-Span é um modloader Lua 5.4 compartilhado para Ocarina of Time no Shipwright e Majora's Mask no 2Ship2Harkinian. A API Lua é **0.5.0**. Um mod que usa a API comum pode atender aos dois jogos; recursos específicos dependem das capabilities do host.

## Comece por uma tarefa

- [Primeiros passos](Getting-Started-pt-BR)
- [Seu primeiro mod](Writing-Your-First-Mod-pt-BR)
- [Mods nativos](Native-Mods-pt-BR)
- [Mods de exemplo](Example-Mods-pt-BR)
- [Assets entre mundos](Cross-World-Assets-pt-BR)
- [Empacotar e publicar](Packaging-and-Publishing-pt-BR)
- [Solução de problemas](Troubleshooting-pt-BR)
- [Perguntas frequentes](FAQ-pt-BR)

## Recursos

- Um estado Lua isolado por mod, manifestos, verificações de compatibilidade e ordem de dependências.
- Eventos, hotkeys reconfiguráveis e API de atores com handles opacos.
- Carregamento de pastas, ZIPs e `.shipmod`; CLI de desenvolvimento e mock runtime sem ROM.
- Namespaces de recursos entre mundos e launcher para seleção de jogo e viagens, quando os hosts instalados oferecem suporte.
- Providers nativos e core extensions. Código nativo roda no processo do jogo; o isolamento Lua não cria uma sandbox para DLLs.

A instalação OoT descrita aqui usa o **overlay oot-0.5.0 RC5 para Shipwright 9.2.3**. MM e launcher têm requisitos próprios; veja [Primeiros passos](Getting-Started-pt-BR).

## Referências completas

[Guias para modders](https://github.com/BaiterYamato/link-span/blob/main/docs/README.pt-BR.md) · [API Lua](https://github.com/BaiterYamato/link-span/blob/main/generated/docs/api-reference.pt-BR.md) · [Repositório de exemplos](https://github.com/BaiterYamato/link-span-examples)

Os exemplos ficam no repositório separado. As fixtures de testes do core não são os exemplos para modders.

## Research (pt-BR)

Estes registros internos de pesquisa continuam em português. São material de apoio, não guias de instalação.

- [Research-External-Item-Systems](https://github.com/BaiterYamato/link-span/blob/main/docs/wiki/Research-External-Item-Systems.md)
- [Research-Survival-System](https://github.com/BaiterYamato/link-span/blob/main/docs/wiki/Research-Survival-System.md)
