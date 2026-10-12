[English](Troubleshooting) · **Português (Brasil)**

# Solução de problemas

## Encontre o primeiro diagnóstico útil

Leia o log ao lado do executável: `logs/Ship of Harkinian.log` no OoT ou `logs/2 Ship 2 Harkinian.log` no MM. Para Unbound, leia também `logs/linkspan-unbound.log`. Comece pelo primeiro erro ou mod recusado, não pelos sintomas posteriores.

## Um mod não aparece ou é recusado

- Confirme que abriu o executável com Link-Span e colocou o pacote em `mods/` daquele jogo.
- O ZIP precisa de `manifest.toml` na raiz; ZIPs sem ele são ignorados. Confira arquivos de entrypoint/provider e evite versões duplicadas do mesmo id.
- Valide o pacote realmente instalado com `python tools/shipmod.py validate <pacote>`. Use o validador compilado, não só o fallback básico da CLI.
- Confira a faixa de API com **0.5.0**, jogo, versão do host, capabilities obrigatórias e dependências. Uma faixa antiga `>=0.4 <0.5` exclui 0.5.0. Ampliar a faixa não corrige código incompatível; teste antes de alterar.
- Uma capability obrigatória ausente exige host adequado ou revisão do mod. Para recurso opcional, use `ship.capabilities.has(...)` antes de chamar.
- No Windows, experimente um caminho curto, como `C:\Jogos\SoH`. O RC8 ainda tem limite de 260 caracteres e extrai pacotes em `mods/.shiplua-cache/`.

## Layout ou fingerprint nativo diferente

Use o provider compilado contra o SDK deste host. Recompile ou baixe pacote correspondente; não contorne a verificação de layout. Providers com escape hatch também precisam do fingerprint exato do executável e do `soh.symbols` correspondente. Mantenha `soh.exe`, `soh.o2r` e `soh.symbols` do mesmo overlay. Veja [Mods nativos](Native-Mods-pt-BR).

## O boot guard desativou um mod

Se uma falha nativa atribuída ocorre durante a carga ou antes de 300 frames de gameplay, o início seguinte desativa o culpado em **`mods/.shiplua-disabled`**. O pacote continua em `mods/`.

Feche o jogo, leia o log e mova o pacote suspeito para fora de `mods/`. Depois de corrigir ou substituí-lo, remova só sua linha de `.shiplua-disabled` para reativar. Não limpe a lista inteira sem investigar. Se o boot continuar falhando, restaure o backup de host/mods correspondente descrito em [Primeiros passos](Getting-Started-pt-BR).

## Tecla ou recurso sem efeito

Confira o binding configurado e outros mods na mesma tecla. Criar atores pode exigir um objeto carregado na cena atual; use a cena recomendada pelo exemplo. Confira capabilities em vez de assumir que mock e host real oferecem os mesmos serviços.

Para conteúdo entre mundos, confira o diagnóstico de montagem e corrija as pastas irmãs ou `SHIPLUA_MM_ROOT` / `SHIPLUA_OOT_ROOT`. Gere ambos os archives pelos fluxos normais dos jogos. Veja [Assets entre mundos](Cross-World-Assets-pt-BR).

Para NEI, se itens aparecerem sem ícone ou modelo, procure a linha de status do NEI no log do jogo e confira se ela diz `assets: pacote: core+...`. A partir do NEI 0.3.24 os assets vão dentro do pacote `LinkSpan-NEI-*.shipmod`, então não há nada a gerar; reinstale o pacote se essa linha não mostrar o núcleo. Um pacote antigo (anterior à 0.3.24) ainda depende de `nei-assets/nei-assets-core.o2r` ao lado do jogo; sem ele só Roc's Feather e Roc's Cape ficam disponíveis. Para dados Unbound, instale o Unbound Framework exigido; sem ele, o RC8 ignora essas camadas de archive.

## Relate um problema reproduzível

Inclua versão do host e overlay, nomes/versões dos mods, ações e cena exatas e trecho relevante do log com eventual stack trace. Para falhas nativas, inclua informação do SDK/layout e se o boot guard desativou algum pacote. Retire segredos e informações pessoais dos anexos. Abra relatos nas [issues do Link-Span](https://github.com/BaiterYamato/link-span/issues).
