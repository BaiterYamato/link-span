"""Coleta o diff proprio do NEI (merge-base com o develop do upstream .. head congelado)."""
import collections, json, os, subprocess, sys

REPO = r"D:\Desenvolvimento\ship-lua-worktrees\OOT-UPSTREAM-78DC-001"
HEAD = "c29262b76ead6f00786885ae4be68916cbee77f5"
BASE = "783139310ae89427ce90b0476be86b08e4261c79"
OUT = os.path.dirname(os.path.abspath(__file__))


def git(*args):
    return subprocess.run(["git", "-C", REPO, *args], capture_output=True, check=True).stdout.decode("utf-8", "replace")


status = {}
for line in git("diff", "--name-status", "-M", BASE, HEAD).splitlines():
    parts = line.split("\t")
    status[parts[-1]] = parts[0][0]
num = {}
for line in git("diff", "--numstat", "-M", BASE, HEAD).splitlines():
    a, d, path = line.split("\t", 2)
    if " => " in path:
        path = path.split(" => ")[-1].replace("}", "").replace("{", "")
    num[path] = (a, d)

rows = []
for path, st in sorted(status.items()):
    a, d = num.get(path, ("-", "-"))
    rows.append((path, st, a, d))
with open(os.path.join(OUT, "files.tsv"), "w", encoding="utf-8") as f:
    for r in rows:
        f.write("\t".join(r) + "\n")


def bucket(path):
    p = path.split("/")
    if p[0] != "soh":
        return "/".join(p[:1])
    if len(p) > 2 and p[1] == "soh":
        if p[2] == "mods" and len(p) > 3:
            return "soh/soh/mods/" + p[3]
        return "soh/soh/" + (p[2] if len(p) > 3 else "(raiz)")
    if len(p) > 2 and p[1] == "assets":
        return "soh/assets/" + (p[2] if len(p) > 3 else "(raiz)")
    if len(p) > 2 and p[1] == "src":
        return "soh/src/" + (p[2] if len(p) > 3 else "(raiz)")
    return "/".join(p[:2])


agg = collections.OrderedDict()
for path, st, a, d in rows:
    b = bucket(path)
    e = agg.setdefault(b, {"files": 0, "A": 0, "M": 0, "D": 0, "R": 0, "add": 0, "del": 0, "bin": 0, "ext": collections.Counter()})
    e["files"] += 1
    e[st if st in "AMDR" else "M"] += 1
    if a == "-":
        e["bin"] += 1
    else:
        e["add"] += int(a); e["del"] += int(d)
    e["ext"][os.path.splitext(path)[1].lower() or "(sem)"] += 1
for b, e in sorted(agg.items(), key=lambda kv: -kv[1]["files"]):
    ext = ", ".join(f"{k}:{v}" for k, v in e["ext"].most_common(4))
    print(f"{e['files']:5} A{e['A']:<5} M{e['M']:<4} D{e['D']:<3} R{e['R']:<3} +{e['add']:<7} -{e['del']:<5} bin{e['bin']:<4} {b}  [{ext}]")
