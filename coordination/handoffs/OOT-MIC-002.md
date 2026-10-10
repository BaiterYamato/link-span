# Handoff — OOT-MIC-002

## Estado

review — feedback visual implementado inteiramente no mod. O pacote 0.1.1 foi validado pelo Codex no host
MovementV2-OcarinaV1 e reempacotado e validado pelo Claude no host do OOT-UNBOUND-003B.

## Resultado

- O estado `audio-input-on` agora mantém uma tarja no alto do HUD com bloco
  laranja, glifo de microfone pulsante e os textos `MICROFONE ATIVO` e `B: SAIR`.
- O aviso desaparece quando B encerra a captura, a ocarina é guardada ou uma
  música é reconhecida, porque esses caminhos deixam de retornar
  `audio-input-on` no mesmo update do mod.
- O manifesto exige a capability existente `hud.draw`; nenhuma alteração ou
  recompilação do host foi necessária.
- ZIP do Codex (host MovementV2-OcarinaV1): `OOT-NATIVE-001/build/distribution/Mic-Ocarina-0.1.1.zip`.
- ZIP para o host do OOT-UNBOUND-003B (overlay MovementV2-D-pad), gerado pelo Claude a pedido do usuário a partir
  da mesma fonte: `OOT-NATIVE-001/build/distribution/Mic-Ocarina-0.1.1-layout-bceacd08.zip`. A DLL do Mic não
  embute layout id, porque `ShipOotOcarinaV1` não tem `layout_id`; o sufixo só identifica o SDK usado.

## Commits

- OOT-NATIVE-001: `8ef12c061`, commit local feito pelo Claude a pedido do usuário, depois do reempacotamento e do
  smoke. Sem push.

## Arquivos alterados

- `OOT-NATIVE-001/soh/native-sdk/mic-ocarina/main.lua`
- `OOT-NATIVE-001/soh/native-sdk/mic-ocarina/manifest.toml`
- `OOT-NATIVE-001/soh/native-sdk/mic-ocarina/README.md`
- `link-span/coordination/claims/OOT-MIC-001.md` (liberado após a entrega anterior)
- `link-span/coordination/claims/OOT-MIC-002.md`
- `link-span/coordination/handoffs/OOT-MIC-002.md`

## Validação executada

```powershell
# Codex, host MovementV2-OcarinaV1
cmake -S soh/native-sdk/mic-ocarina -B build/mic-ocarina ...
cmake --build build/mic-ocarina --config Release
ctest --test-dir build/mic-ocarina -C Release --output-on-failure
shiplua_manifest_validator.exe build/distribution/Mic-Ocarina-0.1.1.zip
prepare-runtime-session.ps1 ...
# soh.exe iniciado na fixture limpa com DM 0.2.7 e Mic Ocarina 0.1.1
finish-runtime-session.ps1 ... -RequireTrackedUnchanged -Strict
git diff --check

# Claude, host do OOT-UNBOUND-003B
cmake -S soh/native-sdk/mic-ocarina -B build/mic-ocarina-0.1.1-unbound-003b -DOOT_NATIVE_SDK=<sdk do host 003B> -DSDL2_DIR=<sdl2 estático>
cmake --build build/mic-ocarina-0.1.1-unbound-003b --config Release
ctest --test-dir build/mic-ocarina-0.1.1-unbound-003b -C Release --output-on-failure
shiplua_manifest_validator.exe build/distribution/Mic-Ocarina-0.1.1-layout-bceacd08.zip
# smoke: overlay MovementV2-D-pad com DM 0.2.8 e o ZIP acima; prepare/finish-runtime-session -RequireTrackedUnchanged -Strict
```

## Resultado da validação

- Build do provider passou e `mic_ocarina_matcher` passou 1/1.
- Manifesto aprovado: id `baiteryamato.mic_ocarina`, versão 0.1.1 e
  `hud.draw` obrigatório.
- ZIP com 795.661 bytes e SHA-256
  `2A668B23B4ABFBBDC66AA50AD1F5A4F969EF30526A453D907FB2645215DC687F`.
- `main.lua`, manifesto e README do ZIP são byte a byte iguais às fontes.
- Smoke real válido na fixture `fresh-mic-002-20260913-1307`: os mods 0.2.7 e
  0.1.1 carregaram, `mic-ocarina: pronto` apareceu e executável, O2Rs e ZIPs
  acompanhados permaneceram inalterados.
- O aviso consome 46 unidades do orçamento máximo de 400 desenhos por frame.
- Reempacotamento para o host do OOT-UNBOUND-003B:
  - `ship.hud.draw_rect`, `draw_ring` e `draw_text` existem no host com as assinaturas usadas pelo `main.lua`
    (`ShipLuaBootstrap.cpp`);
  - ctest 1/1, validador aprovado e `main.lua`, manifesto e README do ZIP byte a byte iguais às fontes;
  - ZIP com 795.114 bytes e SHA-256 `BC32102BAF4691B8452F80A65C5B3FB9CDABC6834899C70110DD662CE66B40CA`;
  - smoke válido na instalação limpa `fresh-unbound-003b-20260913-1204`, junto do Dynamic Movement 0.2.8:
    `mic-ocarina: pronto; ocarina + Start ativa microfone; B encerra` e `mic-ocarina: aguardando ocarina` no log,
    171 s vivo e ZIPs inalterados.

## Pendências

- Entrar numa partida com ocarina e microfone reais para capturar a aparência
  final do aviso e confirmar a saída visual com B.

## Riscos

- O smoke valida carregamento e sintaxe, mas não ativou o microfone; posição,
  escala e legibilidade ainda dependem de julgamento visual no jogo.
- A tarja ocupa o canto superior esquerdo (x 17 a 209, y 15 a 49 no HUD 320x240), onde ficam corações e magia;
  pode cobri-los enquanto a captura estiver ativa.
- Hosts anteriores sem `hud.draw` recusarão corretamente o mod 0.1.1 pelo
  manifesto, em vez de carregá-lo sem feedback.

## Próxima ação recomendada

- Instalar o overlay MovementV2-D-pad com `Mic-Ocarina-0.1.1-layout-bceacd08.zip`, abrir a ocarina, apertar
  Start/+ e registrar uma captura com a tarja visível; ajustar somente o Lua se o usuário preferir outra posição,
  cor ou tamanho.
