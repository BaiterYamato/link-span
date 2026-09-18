"""Filtra a tabela de itens (so linhas ITEM_) e acrescenta a coluna Estado."""
import os

OUT = os.path.dirname(os.path.abspath(__file__))
lines = open(os.path.join(OUT, "items-table.md"), encoding="utf-8").read().splitlines()
head, sep, rows = lines[0], lines[1], [l for l in lines[2:] if "| `ITEM_" in l]
STATE = {
    "ITEM_DESIRE_SENSOR": "retirado (virou runa da Sheikah Slate)",
    "ITEM_HYLIAS_GRACE": "retirado da grade (célula 41 é da Phantom Hourglass)",
    "ITEM_NET": "adiado (ícone provisório)",
    "ITEM_BOTTOMLESS_BOTTLE": "adiado (ícone provisório)",
    "ITEM_POKEBALL": "expansão Pikachu (SSBB)",
    "ITEM_MARIO_MASK": "expansão SM64 (libsm64 + ROM do usuário)",
    "ITEM_BOTTLE_WITH_MAGIC_MUSHROOM": "garrafa (Mask of Scents)",
}
out = [head.replace("| Multi |", "| Multi | Estado |"), sep + ":-:|---|".replace(":-:|", "", 1)]
out[1] = sep + "---|"
for i, row in enumerate(rows, 1):
    cells = row.split("|")
    item = cells[2].strip().strip("`")
    cells[1] = f" {i} "
    if item in STATE:
        state = STATE[item]
    elif item.startswith("ITEM_MM_MASK_"):
        state = "máscara MM (página 3)"
    elif item in ("ITEM_GOLD_DUST", "ITEM_HOT_SPRING_WATER", "ITEM_DEKU_PRINCESS", "ITEM_SEAHORSE",
                  "ITEM_SPRING_WATER", "ITEM_ZORA_EGG", "ITEM_HYLIAN_LOACH", "ITEM_OBABA_DRINK"):
        state = "conteúdo de garrafa MM"
    else:
        state = "ativo"
    out.append("|".join(cells[:-1]) + f"| {state} |")
open(os.path.join(OUT, "items-final.md"), "w", encoding="utf-8").write("\n".join(out) + "\n")
print(len(rows))
print("\n".join(out[:6]))
