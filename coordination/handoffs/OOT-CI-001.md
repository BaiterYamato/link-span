# Handoff — OOT-CI-001

## Estado

review

## Resultado

- O build raiz do Shipwright expõe os testes do host e do core ao CTest. Antes, `ctest --test-dir build/x64 -N` listava zero; agora lista 67.
- O workflow existente `generate-builds` executa essa suíte após o build de Windows e Linux, antes de publicar os artefatos.
- Os quatro testes do core que usam miniz diretamente recebem o mesmo prefixo de símbolos ZIP usado pelo host. O Python necessário ao teste de mod nativo independente fica disponível no escopo raiz do CMake.

## Commits

- 2026-10-02, por Claude com autorização do usuário ("Faça o que é necessário"). Locais, sem push.
  - host `26b8ca2a8` (CTest raiz e workflow).

## Arquivos alterados

- `OOT-UPSTREAM-78DC-001/CMakeLists.txt` e `OOT-UPSTREAM-78DC-001/.github/workflows/generate-builds.yml`.
- A correção encontrada no teste da policy do registro V2 está no claim SKIJER-001: `soh/soh/native/OotNativeEngine.cpp` e `soh/native-sdk/tests/EngineTests.cpp`.

## Validação executada

```text
Configuração CMake Windows/MSVC: passou.
Build Release completo: passou; o vinculador local precisou reiniciar em 64 bits após aviso de memória insuficiente na primeira tentativa de LTCG.
CTest raiz Release: 67/67 passou; inclui os testes do core, do host e o mod nativo independente.
YAML do workflow: parser local aceitou; git diff --check sem erro.
```

## Resultado da validação

- Gate local Windows passou. A execução remota do workflow modificado em Windows/Linux ainda não ocorreu.

## Pendências

- Revisar o diff e executar o workflow no GitHub quando a integração for autorizada. CI verde remoto não pode ser afirmado a partir do teste local.

## Riscos

- O runner Linux ainda não foi verificado com este diff. O build local usa Visual Studio; o workflow Windows usa Ninja/MSVC.
- Há alterações simultâneas no checkout; preservar ao integrar.

## Próxima ação recomendada

- Revisar o diff, integrar o recorte e observar os jobs Windows e Linux do workflow `generate-builds`.
