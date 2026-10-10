[English](FAQ) · **Português (Brasil)**

# Perguntas frequentes

**Preciso recompilar o jogo para cada mod?** Não. Instale mods Lua em `mods/` de um host integrado. Mods nativos compilam seu provider contra o SDK correspondente; um serviço ausente pode exigir atualizar o host.

**Um mod funciona nos dois jogos?** Um mod da API comum pode funcionar. Confira jogos declarados e capabilities obrigatórias; serviços nativos OoT não existem automaticamente no MM.

**`.shipmod` é diferente de ZIP?** Não. É um ZIP com outra extensão. Mantenha `manifest.toml` na raiz.

**Os exemplos acompanham o core?** Não. Baixe em [link-span-examples](https://github.com/BaiterYamato/link-span-examples). Uma release do host pode incluir demos selecionados.

**Posso desenvolver sem ROM?** Você pode validar manifestos e executar testes no mock. Gameplay real e recursos entre mundos exigem seus archives de jogo gerados localmente.

**Preciso ter os dois jogos?** Mods de um só jogo precisam apenas do archive do destino. Conteúdo entre mundos precisa do archive do jogo de origem; mods de viagem com `requires_both_games = true` exigem ambos.

**Providers nativos têm sandbox?** Não. Rodam no processo do host. Instale providers de fontes confiáveis e use build com SDK correspondente.

**Posso publicar assets extraídos do jogo?** Não inclua ROMs ou assets protegidos extraídos. Referencie os archives do jogador e siga as licenças do código e assets redistribuídos.

**Por onde começar?** [Primeiros passos](Getting-Started-pt-BR) para jogadores; [Seu primeiro mod](Writing-Your-First-Mod-pt-BR) para autores; [Mods nativos](Native-Mods-pt-BR) para providers.
