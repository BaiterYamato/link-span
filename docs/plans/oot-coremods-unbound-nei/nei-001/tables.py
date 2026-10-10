"""Tabelas do inventario NEI-001: arquivos do host tocados pelo fork e resumo por area."""
import collections, os

OUT = os.path.dirname(os.path.abspath(__file__))
rows = [l.rstrip("\n").split("\t") for l in open(os.path.join(OUT, "files.tsv"), encoding="utf-8")]
conflicts = {l.strip() for l in open(os.path.join(OUT, "conflicts.txt"), encoding="utf-8-sig") if l.strip()}


def area(p):
    if p.startswith("soh/assets/custom/"):
        return "assets"
    if p.startswith("soh/mods/"):
        return "mods"
    if p.startswith("soh/expansions/"):
        return "expansions"
    if p.startswith(("soh/soh/FleetShipCombo/", "soh/soh/Network/")):
        return "network"
    return "host"


def role(p):
    if "/randomizer/" in p:
        return "randomizer"
    if "/Network/" in p or "FleetShipCombo" in p:
        return "multiplayer"
    if p.startswith("soh/src/overlays/"):
        return "ator"
    if p.startswith(("soh/src/code/", "soh/include/")):
        return "motor"
    if "SohGui" in p or "debugger" in p:
        return "UI/menu"
    if "custom-message" in p or "Items/" in p:
        return "mensagens/itens"
    if "game-interactor" in p:
        return "hooks"
    return "outro"


counts = collections.Counter(area(r[0]) for r in rows)
status = collections.Counter((area(r[0]), r[1]) for r in rows)
print("areas", dict(counts))
print("status", dict(status))
host = [r for r in rows if area(r[0]) in ("host", "network")]
with open(os.path.join(OUT, "host-table.md"), "w", encoding="utf-8") as f:
    f.write("| Arquivo | St | + | − | Papel | Merge no HEAD |\n|---|:-:|--:|--:|---|---|\n")
    for p, st, a, d in host:
        m = "conflito" if p in conflicts else ("novo" if st == "A" else "limpo")
        f.write(f"| `{p}` | {st} | {a} | {d} | {role(p)} | {m} |\n")
roles = collections.Counter((role(r[0]), "conflito" if r[0] in conflicts else ("novo" if r[1] == "A" else "limpo")) for r in host)
for k, v in sorted(roles.items()):
    print(k, v)
print("host rows", len(host), "conflitos no host", sum(1 for r in host if r[0] in conflicts))
print("conflitos fora da tabela", sorted(c for c in conflicts if c not in {r[0] for r in host}))
