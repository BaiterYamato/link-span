[English](manifest-validator.md) · **Português (Brasil)**

# Validador de manifestos

Execute `shiplua_manifest_validator` antes de distribuir um mod Lua. Ele analisa manifestos e verifica a segurança da extração ZIP sem executar Lua. Veja campos em [manifest.pt-BR.md](manifest.pt-BR.md) e descoberta pelo CLI Python em [shipmod-cli.pt-BR.md](shipmod-cli.pt-BR.md).

## Valide pasta, arquivo ou pacote

Aceita arquivo direto de manifesto, pasta contendo `manifest.toml` ou pacote ZIP com extensão `.zip` ou `.shipmod`. O `manifest.toml` do pacote precisa ser um arquivo regular exatamente na raiz do archive. Múltiplas entradas podem ser verificadas numa execução:

```powershell
shiplua_manifest_validator mods/my-mod/manifest.toml
shiplua_manifest_validator mods/my-mod dist/my-mod.shipmod
python tools/shipmod.py validate mods/my-mod dist/my-mod.shipmod
```

Em um build multi-config do core já configurado, compile com `cmake --build build --config Release --target shiplua_manifest_validator` e execute da pasta de saída ou defina `SHIPLUA_MANIFEST_VALIDATOR` para o CLI.

## O que verifica

O executável usa `ManifestParser` do runtime: strings obrigatórias não vazias, SemVer do mod, sintaxe de faixas de API/host/dependência, combinações reconhecidas de papel/fase, formato de grants genéricos, limites de atores e restrições de manifesto de provider nativo. Compartilha a permissividade do parser com campos opcionais/desconhecidos; não é um motor completo de JSON Schema. O schema continua sendo a referência dos tipos pretendidos.

Pacotes são inspecionados quanto ao manifesto na raiz e extraídos em pasta temporária pelo extrator do runtime. O conteúdo temporário é removido ao finalizar aquela entrada. A extração rejeita caminhos inseguros (absolutos e travessia), riscos de links simbólicos/reparse points, entradas duplicadas/inseguras e violações de limites. Os limites padrão de pacote são:

| Limite | Padrão |
| --- | --- |
| Entradas | 1024 |
| Bytes descompactados por arquivo | 64 MiB |
| Total de bytes descompactados | 256 MiB |
| Razão de compressão | 200 |

A entrada precisa estar acessível e não pode ser link simbólico. A extração ZIP pode falhar quando a pasta temporária estiver indisponível.

## O que ainda precisa de teste

O validador não carrega o entrypoint, cria estado Lua, compara a versão da API/host em execução, verifica capabilities de host vivo ou resolve dependências instaladas. Não comprova disponibilidade de atores, renderização, comportamento de save ou viagem. Pode aceitar manifesto cujo arquivo de entrypoint não exista. Depois de validar, faça testes mock e as verificações reais de [writing-mods.pt-BR.md](writing-mods.pt-BR.md).

Sem o executável compilado, `shipmod validate` usa uma **verificação básica** Python identificada na saída. Esse fallback não faz extração segura canônica nem valida toda a semântica do manifesto. Não trate as duas verificações como equivalentes.

## Mensagens e códigos de saída

A saída usa `Válido:` ou `Inválido:`. Os diagnósticos são em português e podem abreviar mensagens do parser. Trechos típicos são `campo obrigatório ausente`, `campo inválido 'api'`, `campo inválido 'version'`, `range inválido para a dependência` e `erro de sintaxe TOML na linha ..., coluna ...`. Outros problemas semânticos podem aparecer como `estrutura ou conteúdo inválido`. A [referência do manifesto](manifest.pt-BR.md) relaciona mensagens e correções.

| Código de saída | Significado |
| --- | --- |
| `0` | Todas as entradas válidas |
| `1` | Pelo menos uma entrada inválida |
| `2` | Nenhum caminho de entrada fornecido |

A ferramenta standalone recebe caminhos, não flags do CLI como `--game`. Use o runner mock para selecionar jogo.
