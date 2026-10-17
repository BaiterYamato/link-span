[English](Native-Mods) · **Português (Brasil)**

# Mods nativos

Um mod nativo empacota Lua com uma biblioteca provider em C/C++. O Lua chama funções privadas do mod por `ship.native.call`. Core extensions podem publicar serviços versionados para outros providers. Novas funções privadas não precisam de entrada no schema da API Lua nem de recompilação do host; um serviço ausente pode exigir atualizar o adaptador.

## Use o SDK OoT correspondente

Baixe o **`LinkSpan-OoT-SDK-oot-0.5.0-rc10-layout-ee6449e8-Win64.zip`** do RC10 junto com a release do host correspondente. Extraia fora da pasta do jogo. O SDK não é um mod: não o coloque em `mods/`.

Leia `AUTHOR-GUIDE.pt-BR.md`, `OOT-NATIVE-CONTRACT.md` e `linkspan-sdk.json` no SDK. Use a versão do MSVC registrada ali. Um build do host exporta `build/x64/native-sdk/Release/OotNativeSdk.cmake`; aponte `OOT_NATIVE_SDK` para o `OotNativeSdk.cmake` real do SDK extraído ou do build.

Os passos completos de autoria e empacotamento estão em [docs/native-mods.pt-BR.md](https://github.com/BaiterYamato/link-span/blob/main/docs/native-mods.pt-BR.md). O contrato genérico está em [Providers nativos](https://github.com/BaiterYamato/link-span/blob/main/docs/native-providers.pt-BR.md).

## Compatibilidade e instalação

- Compile para a plataforma declarada no manifesto e para o SDK/layout id correto do host.
- Valide com o validador do SDK/host correspondente. Um validador antigo do core pode recusar versões mais novas da ABI de provider.
- Um provider com escape hatch também depende do fingerprint exato do executável e dos símbolos correspondentes. A versão da API Lua sozinha não basta.
- Coloque o ZIP, `.shipmod` ou pasta validado em `mods/`; mantenha `manifest.toml` na raiz e a biblioteca no caminho declarado.
- Os serviços OoT não existem automaticamente no MM. Confira os serviços documentados do host antes de declarar suporte a MM.

Bibliotecas nativas executam no processo do jogo com suas permissões. Verificações de layout e ABI estabelecem compatibilidade, não uma sandbox. Um crash nativo pode encerrar o jogo; o boot guard serve para recuperação, não contenção.

Veja os demos nativos em [Mods de exemplo](Example-Mods-pt-BR). O `crash-demo` causa crash de propósito e só deve ser usado em teste isolado de recuperação. [Solução de problemas](Troubleshooting-pt-BR) explica `mods/.shiplua-disabled`.
