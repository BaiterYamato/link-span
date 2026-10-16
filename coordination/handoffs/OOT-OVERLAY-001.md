# Handoff — OOT-OVERLAY-001

## Estado

done (2026-10-16, CLAIMS-CLEANUP-001; ver «Encerramento 2026-10-16» no fim). Registro anterior abaixo.

review — overlay ZIP reproduzível gerado e validado sobre uma extração limpa da
release oficial Shipwright 9.2.3 para Windows x64.

## Resultado

- `package-linkspan-overlay.ps1` gera um ZIP para extração direta na raiz do
  Shipwright 9.2.3.
- O overlay substitui o par compatível `soh.exe` + `soh.o2r`, cria `mods` e pode
  incluir um pacote de exemplo.
- O pacote não contém `oot.o2r`, ROM, save nem configuração pessoal.
- A instalação real carregou `native-jump-example.zip`; a tecla J alterou a
  velocidade vertical do jogador de `0.00` para `18.00`.
- O ZIP final inclui metadados, instruções e hashes internos verificáveis.

## Commits

- Nenhum. Commit e push não foram autorizados.

## Arquivos alterados

- `OOT-NATIVE-001/tools/package-linkspan-overlay.ps1`
- `OOT-NATIVE-001/docs/install-linkspan-overlay.md`
- `coordination/claims/OOT-OVERLAY-001.md`
- `coordination/handoffs/OOT-OVERLAY-001.md`

Artefato ignorado pelo Git:

- `OOT-NATIVE-001/build/distribution/LinkSpan-Shipwright-9.2.3-Win64-overlay.zip`
- SHA-256: `5b0ba6c82d11674d855fbacc5bb9a7792688877ee889fe288e5017614ec1211c`

## Validação executada

```text
- parser PowerShell: 0 erros
- duas gerações do overlay: SHA-256 idêntico
- inspeção do ZIP: 6 entradas; nenhum arquivo de jogo protegido
- checksums internos: 5/5 válidos
- instalação sobre SoH-Ackbar-Delta-Win64.zip oficial
- runtime real DirectX 11 com oot.o2r legítimo fornecido apenas à fixture
- tecla J via sequência de input e captura de janela
- encerramento normal e gate RequireTrackedUnchanged -Strict
- git diff --check nos repositórios host e core
```

## Resultado da validação

- Overlay final reproduzível e íntegro.
- Shipwright iniciou como 9.2.3, Link-Span publicou `linkspan.oot.engine` v1,
  encontrou o pacote ZIP e carregou um mod nativo.
- Log funcional: `native-jump: velocidade Y 0.00 -> 18.00`.
- Executável, recursos, `oot.o2r`, mod, assets e checksums permaneceram
  inalterados durante o playtest.
- Evidências em
  `OOT-NATIVE-001/build/official-9.2.3/fresh-package-test-001/overlay-evidence`.

## Pendências

- Publicação/commit continuam pendentes de autorização explícita.
- O build ainda é uma prévia de desenvolvimento baseada no host 9.2.3; antes
  de uma release pública, remover sondagens experimentais de áudio MM do host OoT.

## Riscos

- Um provider nativo roda dentro do processo e pode causar crash ou acessar o
  mesmo estado que o jogo. Instalar somente mods nativos confiáveis.
- `soh.exe` e `soh.o2r` precisam ser instalados juntos; misturar um deles com o
  arquivo oficial causa falha no carregamento da interface.

## Próxima ação recomendada

- Separar uma release pública do overlay, com os experimentos não relacionados
  desativados, e publicar o ZIP e seu `.sha256` junto à versão correspondente.

## Encerramento 2026-10-16 (CLAIMS-CLEANUP-001)

- Estado final: done.
- Evidência de entrega: O overlay ZIP reproduzível veio com o host da OOT-NATIVE-001 (a19372c00, RC2+). A sonda de áudio MM foi retirada no fc27099 (RC2), e o overlay foi publicado no RC8 e no RC9.
- Core: commits na main que citam a tarefa: 6b8393b.
- As pendências registradas acima ficam como histórico; este encerramento atesta a entrega do recorte do claim, não a das pendências.
- Levantamento: `claims-review-2026-10-16.csv` em `logs/claims-cleanup-2026-10` do workspace, aprovado pelo usuário em 2026-10-16.
