[English](Getting-Started) · **Português (Brasil)**

# Primeiros passos

## Instalar no OoT: overlay e mods independentes

Use os arquivos **oot-0.5.0 RC9** e suas notas de release no [fork Shipwright com Link-Span](https://github.com/BaiterYamato/Shipwright/releases/tag/oot-0.5.0-rc9). Escolha o overlay somente para **Shipwright 9.3.0**, não o ZIP do SDK de autor. Confira os arquivos baixados com o `SHA256SUMS.txt` da release.

1. Extraia o Shipwright 9.3.0 oficial (`SoH-Dewey-Alpha-Win64.zip`) numa pasta nova com caminho curto, como `C:\Jogos\SoH`. Use o fluxo normal de extração do Shipwright para gerar `oot.o2r` da sua própria ROM do jogo. Abra o jogo uma vez e feche.
2. Faça backup da pasta inteira. Guarde ao menos `Save/`, `shipofharkinian.json`, `mods/`, `soh.exe`, `soh.o2r` e `soh.symbols`.
3. Extraia `LinkSpan-Shipwright-9.3.0-Upstream-ecd889c20-layout-7376240d-Win64-overlay.zip` na raiz do jogo. Troque **`soh.exe`, `soh.o2r` e `soh.symbols` juntos**.
4. Coloque os ZIPs dos mods escolhidos em `mods/`, sem extrair. Abra `soh.exe` e confira os mods carregados ou recusados em `logs/Ship of Harkinian.log`.

```text
SoH/
├── soh.exe
├── soh.o2r
├── soh.symbols
├── oot.o2r              # gerado localmente pelo jogador
├── Save/
├── logs/
└── mods/               # pacotes escolhidos ou pastas de desenvolvimento
```

O RC9 oferece pacotes separados de Not Enough Items 0.3.24, Unbound Framework 0.6.14, Dynamic Movement Remake 0.2.23, Mic Ocarina 0.2.9, Wind Waker Style 0.1.5 e Shovel Demo 0.1.5, além do archive de dados Unbound Actors Demo e do SDK de autor. Instale o que quiser; o overlay é o host, e esses pacotes são mods.

Com o Unbound Framework instalado, a primeira execução gera `oot-unbound.o2r` e `oot-unbound.o2r.source.json` ao lado de `oot.o2r`. O log detalhado é `logs/linkspan-unbound.log`. Archives de dados Unbound precisam do framework; o RC9 ignora essas camadas quando ele não está presente.

A partir do NEI 0.3.24, os assets do NEI (modelos, ícones, texturas e nomes dos itens) vão dentro do pacote `LinkSpan-NEI-*.shipmod`, na pasta `assets/`. Não há nada a gerar nem a copiar: coloque o pacote em `mods/` e o coremod NEI monta os assets de dentro dele. Algumas texturas que são cópias exatas das do próprio jogo não estão no pacote; os modelos as leem do seu `oot.o2r` / `mm.o2r`. A pasta `nei-assets/` ao lado do jogo é só uma alternativa, usada quando o pacote não traz o núcleo. Pacotes NEI antigos (anteriores à 0.3.24, como o 0.3.23 do RC8) ainda dependem de um `nei-assets/nei-assets-core.o2r` gerado localmente; sem ele, só Roc's Feather e Roc's Cape ficam disponíveis. Sem o núcleo, a 0.3.24 se comporta do mesmo jeito.

## MM e launcher

MM precisa de um **host 2Ship2Harkinian com Link-Span integrado**. Um executável upstream comum não ganha o loader ao receber um ZIP de mod. Use a [integração do host](https://github.com/BaiterYamato/link-span/blob/main/docs/host-integration.pt-BR.md) documentada ou o [fork MM](https://github.com/BaiterYamato/2ship2harkinian). Gere `mm.o2r` pelo fluxo normal de extração do 2Ship, coloque os mods em `mods/` ao lado de `2ship.exe` e confira `logs/2 Ship 2 Harkinian.log`.

O overlay OoT RC9 não é um overlay MM. Para instalar os dois hosts, siga o [guia do launcher](https://github.com/BaiterYamato/link-span/blob/main/docs/launcher.pt-BR.md). Um launcher empacotado usa `link-span.exe` e as pastas `hosts/oot/` e `hosts/mm/`; escolhe o jogo disponível ou oferece um seletor quando ambos estão presentes. Um mod com `requires_both_games = true` exige os dois jogos. Veja [Assets entre mundos](Cross-World-Assets-pt-BR) para os archives vizinhos.

## Atualizar ou voltar atrás

Feche o jogo antes. Mova o pacote antigo para fora de `mods/` e instale o novo; não deixe versões duplicadas. Ao trocar o overlay RC9, substitua NEI, DMR e Mic Ocarina por builds correspondentes, pois dependem do executável exato. Shovel Demo e Wind Waker Style precisam do layout id correspondente; o Unbound usa serviços públicos do host.

Para voltar atrás, restaure o backup: os três arquivos do host juntos, os mods anteriores, saves e configuração. Um save mais novo ou um `shipofharkinian.json` migrado pode não funcionar num host anterior ou oficial.

Nenhuma ROM ou `oot.o2r`, `oot.otr` ou `mm.o2r` gerado pelo jogador é distribuído. [Solução de problemas](Troubleshooting-pt-BR) cobre a recuperação após crash.
