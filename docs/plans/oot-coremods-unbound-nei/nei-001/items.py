"""Catalogo de itens do NEI com checklist por item (NEI-001).

Fonte: sNeiItems[] de soh/mods/extended_player.c no commit congelado. Para cada linha, acha o
arquivo que define as funcoes de update/init e procura evidencias de input, animacao, modelo,
colisao, save, UI, randomizer e multiplayer.
"""
import collections, os, re, subprocess

REPO = r"D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001"
HEAD = "c29262b76ead6f00786885ae4be68916cbee77f5"
OUT = os.path.dirname(os.path.abspath(__file__))
GENERIC = {"func_8083485C", "Player_InitDefaultIA", "NULL"}


def git(*args):
    return subprocess.run(["git", "-C", REPO, *args], capture_output=True).stdout.decode("utf-8", "replace")


def show(path):
    return git("show", f"{HEAD}:{path}")


src = show("soh/mods/extended_player.c")
start = src.index("static const NeiItem sNeiItems[]")
body = src[start:src.index("};", start)]
# tira comentarios de linha para o regex nao pegar texto comentado
body = re.sub(r"//[^\n]*", "", body)
row_re = re.compile(
    r"\{\s*(ITEM_\w+|NEI_NO_ITEM)\s*,\s*(PLAYER_IA_\w+)\s*,\s*(PLAYER_MODELGROUP_\w+)\s*,\s*(\w+)\s*,\s*(AGE_REQ_\w+)\s*,"
    r"\s*([^,]+?)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*(NULL|\")",
    re.S)
rows = [m.groups()[:10] for m in row_re.finditer(body)]

# Todos os arquivos C/C++ do fork, para achar definicoes e referencias.
tree = [p for p in git("ls-tree", "-r", "--name-only", HEAD).splitlines()
        if p.endswith((".c", ".cpp", ".h", ".inc", ".hpp")) and p.startswith("soh/")]
mods_code = [p for p in tree if p.startswith(("soh/mods/", "soh/expansions/"))]


def grep_files(pattern, paths):
    if not paths:
        return []
    out = git("grep", "-l", "-E", pattern, HEAD, "--", *paths)
    return sorted({line.split(":", 1)[1] for line in out.splitlines() if ":" in line})


def definers(fn):
    if fn in GENERIC:
        return []
    return grep_files(rf"^[A-Za-z_][A-Za-z0-9_ \*]*\b{fn}\s*\([^;]*$", ["soh/mods", "soh/expansions", "soh/soh"])


cache = {}


def text(path):
    if path not in cache:
        cache[path] = show(path)
    return cache[path]


randomizer_files = [p for p in tree if "/randomizer/" in p]
network_files = [p for p in tree if p.startswith("soh/soh/Network/") or "FleetShipCombo" in p]
save_hdr = "soh/mods/nei_save.h" if "soh/mods/nei_save.h" in tree else None

ITEM_TOKENS = {}
table = []
for item, ia, group, slot, age, icon, update, init, draw, rg in rows:
    files = []
    for fn in (update, init):
        files += definers(fn)
    # heuristica extra: arquivo de logica com o nome do item
    token = item.replace("ITEM_", "").lower().replace("_skijer", "")
    compact = token.replace("_", "")
    for p in mods_code:
        base = os.path.basename(p).lower().replace("_", "")
        if p.endswith((".c", ".cpp")) and ("item" in base or "logic" in p) and compact in base:
            files.append(p)
    symbol = rg if item == "NEI_NO_ITEM" else item
    if not files and symbol != "NEI_NO_RG":
        skip = ("extended_player.c", "custom_items.h", "z64item.h", "item_list.cpp", "randomizerTypes.h", "randomizer.cpp")
        refs = [p for p in grep_files(rf"\b{symbol}\b", ["soh/mods", "soh/expansions"]) if not p.endswith(skip)]
        files += refs
    files = sorted(set(files))
    blob = "\n".join(text(p) for p in files)
    has = lambda pat: bool(re.search(pat, blob))
    anim = has(r"LinkAnimation_|gPlayerAnim|AnimationHeader|SkelAnime|PlayerAnimation_")
    model = draw != "NULL" or has(r"\bGfx\b|gSPDisplayList|object_nei_|Gfx_Draw|DisplayList")
    collider = has(r"Collider\w*_|ColliderCylinder|ColliderQuad|ColliderTris|ColliderJntSph")
    inputs = has(r"ItemInput_|CHECK_BTN|press\.button|cur\.button|rel\.button|BTN_")
    save = has(r"Nei_Save\(\)|gSaveContext\.|Flags_Set|NeiSaveData")
    ui = slot != "NEI_NO_SLOT" and icon.strip() != "NULL"
    rando = rg != "NEI_NO_RG" and bool(grep_files(rf"\b{rg}\b", randomizer_files)) if rg != "NEI_NO_RG" else False
    multi = bool(grep_files(rf"\b{item}\b", network_files))
    table.append({
        "item": symbol,
        "noitem": item == "NEI_NO_ITEM", "ia": ia, "slot": slot, "age": age.replace("AGE_REQ_", ""), "rg": rg,
        "files": files, "input": inputs, "anim": anim, "model": model, "collider": collider,
        "save": save, "ui": ui, "rando": rando, "multi": multi, "draw": draw,
    })


def mark(v):
    return "sim" if v else "—"


with open(os.path.join(OUT, "items-table.md"), "w", encoding="utf-8") as f:
    f.write("| # | Item | Slot | Idade | Lógica | Input | Anim | Modelo | Colisão | Save | UI | Rando | Multi |\n")
    f.write("|---:|---|---|---|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|\n")
    for i, r in enumerate(table, 1):
        logic = ", ".join(f"`{os.path.basename(p)}`" for p in r["files"][:3]) or "genérica"
        if len(r["files"]) > 3:
            logic += f" +{len(r['files']) - 3}"
        f.write(f"| {i} | `{r['item']}` | {r['slot'].replace('SLOT_', '')} | {r['age']} | {logic} | {mark(r['input'])} | "
                f"{mark(r['anim'])} | {mark(r['model'])} | {mark(r['collider'])} | {mark(r['save'])} | {mark(r['ui'])} | "
                f"{mark(r['rando'])} | {mark(r['multi'])} |\n")
print("linhas", len(table))
for r in table:
    print(("*" if r["noitem"] else " ") + r["item"], r["slot"], len(r["files"]), "rg" if r["rando"] else "", "multi" if r["multi"] else "")
