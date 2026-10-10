[English](Packaging-and-Publishing) · **Português (Brasil)**

# Empacotar e publicar

## Empacote o conteúdo do mod

Durante o desenvolvimento, instale uma pasta. Para distribuir, use um **ZIP** com `manifest.toml` na raiz. `.shipmod` é uma extensão alternativa para o mesmo ZIP, não outro formato. Inclua o entrypoint declarado, scripts, assets originais, binários de provider e avisos legais necessários.

```text
first-mod.zip
├── manifest.toml
├── main.lua
├── LICENSE
└── NOTICE.md
```

Num diretório que contém `first-mod/`:

```powershell
Compress-Archive -Path first-mod/* -DestinationPath first-mod.zip
python tools/shipmod.py validate first-mod.zip
```

Isso empacota o conteúdo, sem uma pasta `first-mod/` envolvendo tudo. Para usar a extensão alternativa:

```powershell
Rename-Item -LiteralPath first-mod.zip -NewName first-mod.shipmod
python tools/shipmod.py validate first-mod.shipmod
```

A CLI tem `new`, `validate`, `test` e `doctor`; não existe subcomando `pack` ou `publish`.

## Valide antes de publicar

1. Execute o **validador de manifesto compilado** no pacote final por `shipmod.py validate`. Compile `shiplua_manifest_validator` antes ou configure `SHIPLUA_MANIFEST_VALIDATOR`; para mods nativos, use o validador do SDK/host correspondente. Se a CLI informar validação básica de fallback, conclua a validação canônica antes de publicar.
2. Confira id, versão do mod, faixa de API, jogos, capabilities obrigatórias, dependências e todos os caminhos declarados. Providers nativos precisam de ABI, plataforma, layout e eventual fingerprint do host correspondentes.
3. Execute testes no mock quando aplicável e instale o pacote final em `mods/` de um jogo de teste limpo. Confirme carga, comportamento esperado, save/reload, remoção e combinações relevantes de mods no host real. Registre versão do host/SDK e resultados.
4. Inclua instruções de instalação, requisitos, limitações, atualização e rollback. Retire saídas de build, caches, saves pessoais e arquivos só de desenvolvimento do pacote.
5. Inclua sua licença e avisos de terceiros; diferencie código próprio, código portado e créditos de pesquisa. O trabalho próprio do Link-Span usa CC0 1.0, mas código e assets de terceiros mantêm seus termos.
6. Não inclua ROMs, `oot.o2r`, `oot.otr`, `mm.o2r`, dumps ou assets protegidos extraídos do jogo. Referencie os archives gerados localmente pelo jogador. Publique só assets que possa redistribuir.
7. Dê uma versão à release, liste hosts e dependências compatíveis e forneça checksum do artefato final.

O validador de manifesto não executa Lua, não prova segurança nativa, não resolve todas as dependências instaladas e não comprova gameplay. Faça essas verificações separadamente.

Referências: [Manifesto](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest.pt-BR.md) · [CLI](https://github.com/BaiterYamato/link-span/blob/main/docs/shipmod-cli.pt-BR.md) · [Validador](https://github.com/BaiterYamato/link-span/blob/main/docs/manifest-validator.pt-BR.md) · [Mods nativos](https://github.com/BaiterYamato/link-span/blob/main/docs/native-mods.pt-BR.md) · [Licença](https://github.com/BaiterYamato/link-span/blob/main/LICENSE) · [Avisos](https://github.com/BaiterYamato/link-span/blob/main/NOTICE.md).
