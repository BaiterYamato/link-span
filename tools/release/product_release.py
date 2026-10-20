#!/usr/bin/env python3
"""Monta e verifica a entrega central do Link-Span; não publica no GitHub."""
from __future__ import annotations

import argparse
import hashlib
import html
import io
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import tomllib
import zipfile

STAMP = (1980, 1, 1, 0, 0, 0)
ROLES = {"overlay", "sdk", "examples", "source"}
# validation.physical_playtest da ficha -> texto da página (pública, em inglês)
PLAYTEST = {"pending": "pending", "passed": "done", "failed": "failed"}
SHA = re.compile(r"^[0-9a-f]{64}$")
COMMIT = re.compile(r"^[0-9a-f]{40}$")
PROTECTED = {".z64", ".n64", ".v64", ".rom", ".sav", ".sra", ".fla", ".eep", ".dmp"}
ROM_MAGICS = {b"\x80\x37\x12\x40", b"\x37\x80\x40\x12", b"\x12\x40\x37\x80"}


class ReleaseError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise ReleaseError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def file_digest(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def safe_name(name):
    require(isinstance(name, str) and name and "\\" not in name and ":" not in name,
            f"Caminho inválido: {name!r}")
    require(not name.startswith("/") and all(p not in {"", ".", ".."} for p in name.rstrip("/").split("/")),
            f"Caminho inseguro: {name}")
    require(not any(ord(c) < 32 for c in name), f"Caractere inválido no caminho: {name!r}")
    return name


def safe_payload(name, data):
    safe_name(name)
    parts = PurePosixPath(name).parts
    base = parts[-1].lower()
    require(PurePosixPath(base).suffix not in PROTECTED and base not in {"oot.o2r", "mm.o2r"},
            f"Conteúdo protegido: {name}")
    require(not any(p.lower() in {".git", "dumps"} for p in parts), f"Diretório proibido: {name}")
    require(not base.startswith(".env") and base not in {"id_rsa", "id_ed25519"}, f"Arquivo privado: {name}")
    require(data[:4] not in ROM_MAGICS, f"Cabeçalho de ROM: {name}")


def json_bytes(value):
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def git(repo, *args):
    result = subprocess.run(["git", "-C", str(repo), *args], capture_output=True)
    require(result.returncode == 0, f"Git: {result.stderr.decode('utf-8', errors='replace').strip()}")
    return result.stdout


def read_definition(path):
    definition = json.loads(Path(path).read_text(encoding="utf-8"))
    require(definition.get("schema_version") == 1, "Schema da entrega não suportado")
    require(re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", definition["tag"]), "Tag inválida")
    require(definition["repository"] == "BaiterYamato/link-span", "Entrega pública deve ficar no link-span")
    require(set(definition["downloads"]) == ROLES, "A entrega exige exatamente os quatro downloads")
    names = [d["file"] for d in definition["downloads"].values()]
    require(len(set(names)) == 4, "Os quatro downloads precisam de arquivos distintos")
    for name in names:
        require("/" not in safe_name(name), "Download deve ter um nome simples")
    for source in definition["sources"].values():
        require(COMMIT.fullmatch(source["commit"]), "Fonte precisa de commit completo, não branch")
        require(re.fullmatch(r"[A-Za-z0-9_-]+/[A-Za-z0-9_.-]+", source["repository"]), "Repositório inválido")
    require(set(definition["sources"]) == {"core", "host", "examples"}, "Fixar os três repositórios")
    for value in (definition["host"]["layout"], definition["host"]["executable_sha256"], definition["evidence"]["sha256"]):
        require(SHA.fullmatch(value), "Fingerprint precisa de SHA-256 completo")
    seen = set()
    for artifact in definition["inputs"]:
        name = artifact["file"]
        require("/" not in safe_name(name) and name not in seen, "Entrada duplicada ou inválida")
        require(SHA.fullmatch(artifact["sha256"]), f"Hash inválido: {name}")
        seen.add(name)
    for role in ("overlay", "sdk"):
        require(any(i["file"] == definition["downloads"][role]["file"] and i["role"] == role
                    for i in definition["inputs"]), f"Entrada de {role} ausente")
    names = set()
    for native in definition["native_examples"]:
        require(re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", native["name"]), "Nome do demo inválido")
        require(native["name"] not in names, "Demo duplicado")
        names.add(native["name"])
        require(safe_name(native["project"]).startswith(("oot-native/", "lua/")), "Projeto fora dos exemplos")
    notes = definition.get("notes")
    if notes is not None:
        require("/" not in safe_name(notes["file"]) and notes["file"].endswith(".md"), "Notas precisam ser um .md ao lado da ficha")
        require(SHA.fullmatch(notes["sha256"]), "Notas precisam de SHA-256 completo")
    return definition


def read_notes(definition, definition_path):
    """Notas da release (em inglês) ao lado da ficha, conferidas pelo hash da ficha; vazio sem notas."""
    notes = definition.get("notes")
    if notes is None:
        return ""
    data = (Path(definition_path).parent / notes["file"]).read_bytes()
    # Mesmo critério da definição: a identidade não depende de CRLF/LF do checkout.
    data = data.replace(b"\r\n", b"\n")
    require(digest(data) == notes["sha256"], "Notas não correspondem à ficha")
    return data.decode("utf-8").strip() + "\n"


def committed_files(repo, commit):
    archive = git(repo, "archive", "--format=zip", commit)
    with zipfile.ZipFile(io.BytesIO(archive)) as z:
        files = {}
        for entry in z.infolist():
            safe_name(entry.filename)
            require(not stat.S_ISLNK(entry.external_attr >> 16), f"Symlink em fonte: {entry.filename}")
            if not entry.is_dir():
                data = z.read(entry)
                safe_payload(entry.filename, data)
                files[entry.filename] = data
        return files


def submodules(repo, commit):
    entries = git(repo, "ls-tree", "-r", commit).decode("utf-8").splitlines()
    return [{"path": line.split("\t", 1)[1], "commit": line.split()[2]}
            for line in entries if line.startswith("160000 ")]


def inspect_zip(data, name, depth=0):
    require(depth <= 8, f"ZIP aninhado demais: {name}")
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        seen = set()
        for entry in z.infolist():
            safe_name(entry.filename)
            require(entry.filename not in seen, f"Entrada ZIP duplicada: {name}!{entry.filename}")
            seen.add(entry.filename)
            require(not stat.S_ISLNK(entry.external_attr >> 16), f"Symlink no ZIP: {name}!{entry.filename}")
            require(entry.file_size <= 128 * 1024 * 1024, f"Entrada grande demais: {name}!{entry.filename}")
            if entry.is_dir():
                continue
            payload = z.read(entry)
            safe_payload(entry.filename, payload)
            if zipfile.is_zipfile(io.BytesIO(payload)):
                inspect_zip(payload, f"{name}!{entry.filename}", depth + 1)


def deterministic_zip(entries):
    output = io.BytesIO()
    seen = set()
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, data in sorted(entries.items()):
            safe_payload(name, data)
            require(name not in seen, f"Entrada duplicada: {name}")
            seen.add(name)
            info = zipfile.ZipInfo(name, STAMP)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            z.writestr(info, data)
    return output.getvalue()


def validate_package(data, definition):
    inspect_zip(data, "mod")
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        require("manifest.toml" in z.namelist(), "Mod sem manifesto na raiz")
        manifest = tomllib.loads(z.read("manifest.toml").decode("utf-8-sig"))
        require(manifest.get("id") and manifest.get("version"), "Mod sem id ou versão")
        entry = safe_name(manifest.get("entrypoint", "main.lua"))
        require(entry in z.namelist(), f"Entrypoint ausente: {entry}")
        provider = manifest.get("provider", {})
        if provider.get("win64"):
            require(safe_name(provider["win64"]) in z.namelist(), "DLL do provider ausente")
        layout = provider.get("host_layout_id") or provider.get("oot_layout_id") or provider.get("layout_id")
        if layout:
            require(str(layout).lower() == definition["host"]["layout"].lower(), "Layout do mod incompatível")
        fingerprints = provider.get("host_fingerprints", [])
        if fingerprints:
            require(definition["host"]["executable_sha256"] in fingerprints, "Mod exige outro executável")
        return manifest


def validate_sources(definition, repos):
    for role, source in definition["sources"].items():
        require(git(repos[role], "rev-parse", f"{source['commit']}^{{commit}}").decode().strip() == source["commit"],
                f"Commit indisponível: {role}")
    recorded = {s["path"]: s["commit"] for s in submodules(repos["host"], definition["sources"]["host"]["commit"])}
    require(recorded.get("extern/ship-lua") == definition["sources"]["core"]["commit"],
            "O host registra outro commit do núcleo")
    schema = json.loads(git(repos["core"], "show", f"{definition['sources']['core']['commit']}:schema/api.yml"))
    require(schema["api_version"] == definition["api_version"], "Versão da API diverge da ficha")


def read_inputs(definition, directory, evidence):
    inputs = {}
    for artifact in definition["inputs"]:
        path = directory / artifact["file"]
        require(path.is_file() and not path.is_symlink(), f"Pacote ausente: {path}")
        data = path.read_bytes()
        require(digest(data) == artifact["sha256"], f"Hash diverge: {artifact['file']}")
        safe_payload(artifact["file"], data)
        inspect_zip(data, artifact["file"])
        inputs[artifact["file"]] = data
    with zipfile.ZipFile(io.BytesIO(inputs[definition["downloads"]["overlay"]["file"]])) as z:
        require(digest(z.read("soh.exe")) == definition["host"]["executable_sha256"], "Executável não é o validado")
    summary = evidence / safe_name(definition["evidence"]["file"])
    require(summary.is_file() and file_digest(summary) == definition["evidence"]["sha256"], "Evidência não corresponde ao RC")
    return inputs, summary.read_bytes()


def examples_archive(definition, files, inputs, native_directory):
    entries, catalog = {}, []
    native_lock = json.loads((native_directory / "native-lock.json").read_text(encoding="utf-8"))
    require(native_lock["examples_commit"] == definition["sources"]["examples"]["commit"], "Demos usam outra revisão dos exemplos")
    require(native_lock["layout"] == definition["host"]["layout"], "Demos usam outro layout")
    require(set(native_lock["packages"]) == {n["name"] for n in definition["native_examples"]}, "Catálogo nativo incompleto")
    for artifact in definition["inputs"]:
        if artifact["role"] != "mod":
            continue
        entries[f"release-mods/{artifact['file']}"] = inputs[artifact["file"]]
        catalog.append({"name": artifact["file"], "kind": "release", "sha256": artifact["sha256"]})
    for name in sorted(files):
        if not re.fullmatch(r"lua/[^/]+/manifest.toml", name):
            continue
        prefix = str(PurePosixPath(name).parent) + "/"
        payload = {n[len(prefix):]: d for n, d in files.items() if n.startswith(prefix)}
        original = tomllib.loads(files[name].decode("utf-8-sig"))
        if original.get("provider", {}).get("win64"):
            require(any(n["id"] == original["id"] for n in definition["native_examples"]), "Provider Lua sem build declarado")
            continue
        packed = deterministic_zip(payload)
        manifest = validate_package(packed, definition)
        games = manifest.get("games", ["oot", "mm"])
        category = "lua-oot" if "oot" in games else "lua-mm"
        slug = PurePosixPath(name).parent.name
        archive_name = f"{category}/{slug}.shipmod"
        entries[archive_name] = packed
        catalog.append({"name": archive_name, "id": manifest["id"], "version": manifest["version"],
                        "games": games, "capabilities": manifest.get("capabilities", {}), "kind": "lua"})
    for native in definition["native_examples"]:
        path = native_directory / f"{native['name']}.zip"
        require(path.is_file() and not path.is_symlink(), f"Demo nativo não empacotado: {path}")
        data = path.read_bytes()
        require(digest(data) == native_lock["packages"][native["name"]], f"Demo alterado após o build: {native['name']}")
        manifest = validate_package(data, definition)
        require(manifest["id"] == native["id"], f"ID do demo não corresponde: {native['name']}")
        category = "diagnostics" if native.get("diagnostic") else "lua-oot" if native.get("lua_provider") else "native-oot"
        target = f"{category}/{native['name']}.zip"
        entries[target] = data
        catalog.append({"name": target, "id": manifest["id"], "kind": "native", "games": manifest.get("games", []),
                        "capabilities": manifest.get("capabilities", {}), "dependencies": manifest.get("dependencies", {}),
                        "diagnostic": native.get("diagnostic", False), "sha256": digest(data)})
    entries["CATALOG.json"] = json_bytes(catalog)
    entries["README.txt"] = (
        "Link-Span — mods and examples\n\n"
        "Install only the mods you want, as listed in CATALOG.json. Do not copy this whole ZIP into mods/.\n"
        "release-mods/: the release candidate's packages, including NEI, DMR, Mic Ocarina and Unbound.\n"
        "lua-oot/: Lua examples that declare OoT support; they need the capabilities listed in the catalog.\n"
        "lua-mm/: MM-only examples; outside the gameplay validation of the OoT candidate.\n"
        "native-oot/: demos built against this candidate's SDK; install them one at a time.\n"
        "diagnostics/: deliberate failure tests; crash-demo crashes the game on purpose.\n"
        "ZIP and .shipmod files go into mods/ without extracting them. The Actors Demo .o2r is optional.\n"
        "The original licenses and notices are kept inside each package and in the Source download.\n"
    ).encode("utf-8")
    for name in ("LICENSE", "NOTICE.md", "README.md", "README.pt-BR.md"):
        if name in files:
            entries[f"docs/{name}"] = files[name]
    return deterministic_zip(entries), catalog


def release_body(definition, notes=""):
    """Página pública da release, em inglês. Com notas, elas substituem a seção genérica de instalação."""
    base = f"https://github.com/{definition['repository']}/releases/download/{definition['tag']}"
    badges = "https://raw.githubusercontent.com/BaiterYamato/link-span/main/docs/assets"
    def button(role):
        download = definition["downloads"][role]
        return f"[![{download['label']}]({badges}/download-{role}.svg)]({base}/{download['file']})"
    sources = "\n".join(f"- {role}: [`{s['commit'][:12]}`](https://github.com/{s['repository']}/commit/{s['commit']})"
                        for role, s in definition["sources"].items())
    candidate = definition["title"].split()[-1]
    playtest = PLAYTEST.get(definition["validation"].get("physical_playtest"), "pending")
    origin = definition.get("historical_publication")
    history = ("\n\n## Earlier publication\n\nThis assembly reuses the original packages without changing their bytes. "
               f"[Original publication]({origin['url']}). The historical origin stays preserved.\n") if origin else "\n"
    install = notes or (
        "## Install\n\n"
        "Generate oot.o2r from your own ROM with the official Shipwright of the stated version. Make a backup and, with "
        "the game closed, extract the overlay into the game root. Pick the mods you want from the examples ZIP and put "
        "the packages into mods/ without extracting them. The SDK and the Source are for development. See README.txt "
        "and CATALOG.json inside the ZIP.\n")
    return (f"Release candidate of Link-Span for Shipwright {definition['host']['version']}.\n\n"
            f"## Download\n\n{button('overlay')} {button('sdk')}\n\n{button('examples')} {button('source')}\n\n"
            f"{install}\n"
            "## Verification\n\nHashes: SHA256SUMS.txt. Provenance: release-lock.json. Test summary: RESUMO.txt (the "
            "battery log, in Portuguese). The additional demos were validated by build and packaging only; that does not "
            f"prove gameplay for every example. Physical playtest of {candidate}: {playtest}.\n\n"
            f"## Delivery sources\n\n{sources}\n\n"
            f"API {definition['api_version']}; layout `{definition['host']['layout'][:8]}`. "
            "External dependencies and their revisions are in SOURCE-LOCK.json inside the Source download.\n\n"
            + history)


def preview(definition):
    buttons = []
    glyphs = {"overlay": "⊞", "sdk": "⚒", "examples": "▣", "source": "〈/〉"}
    for role, download in definition["downloads"].items():
        buttons.append(f'<a class="button" href="{html.escape(download["file"], quote=True)}">'
                       f'<span>{glyphs[role]}</span>{html.escape(download["label"])}</a>')
    return ("<!doctype html><html lang=en><meta charset=utf-8><title>Link-Span — downloads</title>"
            "<style>body{font:17px system-ui;background:#fff;color:#20252b;max-width:980px;margin:48px auto;padding:0 24px}"
            "h1{font-size:34px}h2{border-bottom:1px solid #ddd;padding-bottom:14px}"
            ".row{display:flex;gap:12px;flex-wrap:wrap;margin:20px 0}.button{display:flex;align-items:center;gap:12px;"
            "border:2px solid #30363d;border-radius:10px;padding:14px 20px;color:#20252b;background:#f4f5f6;"
            "text-decoration:none;font-weight:650}.button:hover{background:#e7eef6}.button span{font-size:27px}"
            ".badge{font-size:14px;color:#6b4b00;background:#fff0c4;padding:6px 10px;border-radius:20px}p{line-height:1.6}"
            "</style><h1>" + html.escape(definition["title"]) + "</h1><span class=badge>Local preview of the release assets</span>"
            "<h2>Download</h2><div class=row>" + "".join(buttons[:2]) + "</div><div class=row>" + "".join(buttons[2:]) + "</div>"
            "<h2>Install</h2><p>Install the overlay over Shipwright " + html.escape(definition["host"]["version"]) + ". "
            "Pick the mods from the examples ZIP. The SDK and the Source are for development.</p>"
            "<p>The four buttons open files in this folder. <a href=RELEASE.md>Release notes</a> · "
            "<a href=SHA256SUMS.txt>Hashes</a> · <a href=release-lock.json>Version record</a></p></html>")


def prepare(args):
    definition = read_definition(args.definition)
    repos = {"core": args.core_repo, "host": args.host_repo, "examples": args.examples_repo}
    require(not args.output.exists(), "Saída já existe; use uma pasta nova para preservar a montagem anterior")
    validate_sources(definition, repos)
    inputs, summary = read_inputs(definition, args.artifacts, args.evidence)
    example_files = committed_files(args.examples_repo, definition["sources"]["examples"]["commit"])
    examples, catalog = examples_archive(definition, example_files, inputs, args.native_packages)
    source_entries = {}
    source_lock = {}
    for role, source in definition["sources"].items():
        files = committed_files(repos[role], source["commit"])
        source_entries.update({f"{role}/{name}": data for name, data in files.items()})
        source_lock[role] = {**source, "submodules": submodules(repos[role], source["commit"])}
    source_entries["SOURCE-LOCK.json"] = json_bytes(source_lock)
    source_entries["README.txt"] = (
        "Pinned sources of the Link-Span release. core/, host/ and examples/ keep their own contracts and licenses.\n"
        "External Git dependencies are not vendored; fetch the submodules at the revisions in SOURCE-LOCK.json.\n"
        "The core in host/extern/ship-lua matches the contents of core/. No ROM, save or local file is included.\n"
    ).encode("utf-8")
    notes = read_notes(definition, args.definition)
    artifacts = {definition["downloads"][r]["file"]: inputs[definition["downloads"][r]["file"]] for r in ("overlay", "sdk")}
    artifacts[definition["downloads"]["examples"]["file"]] = examples
    artifacts[definition["downloads"]["source"]["file"]] = deterministic_zip(source_entries)
    for name, data in artifacts.items():
        inspect_zip(data, name)
    # A identidade do documento não depende de CRLF/LF do checkout.
    definition_data = json_bytes(definition)
    lock = {"schema_version": 1, "tag": definition["tag"], "repository": definition["repository"],
            "assembly_state": "verified", "definition_sha256": digest(definition_data), "sources": source_lock,
            "assembler_sha256": file_digest(Path(__file__)),
            "downloads": {r: {"file": d["file"], "sha256": digest(artifacts[d["file"]]),
                               "bytes": len(artifacts[d["file"]])} for r, d in definition["downloads"].items()},
            "validation": {"assembly": "passed", "examples": catalog, "historical": definition["validation"]},
            "documents": {"release-definition.json": digest(definition_data), "RESUMO.txt": digest(summary),
                          **({definition["notes"]["file"]: definition["notes"]["sha256"]} if notes else {})}}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    stage = Path(tempfile.mkdtemp(prefix=".linkspan-", dir=args.output.parent))
    try:
        for name, data in artifacts.items():
            (stage / name).write_bytes(data)
        (stage / "release-definition.json").write_bytes(definition_data)
        (stage / "RESUMO.txt").write_bytes(summary)
        (stage / "release-lock.json").write_bytes(json_bytes(lock))
        (stage / "RELEASE.md").write_text(release_body(definition, notes), encoding="utf-8", newline="\n")
        (stage / "preview.html").write_text(preview(definition), encoding="utf-8", newline="\n")
        (stage / "CATALOG.json").write_bytes(json_bytes(catalog))
        sums = "".join(f"{file_digest(p)}  {p.name}\n" for p in sorted(stage.iterdir()) if p.is_file())
        (stage / "SHA256SUMS.txt").write_text(sums, encoding="utf-8", newline="\n")
        verify_output(stage)
        stage.rename(args.output)
    finally:
        if stage.exists():
            require(stage.resolve().parent == args.output.parent.resolve(), "Pasta temporária fora da saída")
            shutil.rmtree(stage)
    print(json.dumps({"result": "ok", "output": str(args.output), "downloads": 4, "examples": len(catalog)}, ensure_ascii=False))


def verify_output(output):
    lock = json.loads((output / "release-lock.json").read_text(encoding="utf-8"))
    require(set(lock["downloads"]) == ROLES, "Registro precisa dos quatro downloads")
    records = {}
    for line in (output / "SHA256SUMS.txt").read_text(encoding="utf-8").splitlines():
        sha, name = line.split("  ", 1)
        require(SHA.fullmatch(sha) and "/" not in safe_name(name) and name not in records, "Lista de hashes inválida")
        records[name] = sha
    actual = {p.name for p in output.iterdir() if p.name != "SHA256SUMS.txt"}
    require(actual == set(records), "Lista de hashes não cobre exatamente a pasta de saída")
    for name, sha in records.items():
        path = output / name
        require(path.is_file() and not path.is_symlink() and file_digest(path) == sha, f"Arquivo alterado: {name}")
    definition = read_definition(output / "release-definition.json")
    require(file_digest(output / "release-definition.json") == lock["definition_sha256"], "Definição alterada")
    require(lock["tag"] == definition["tag"] and lock["repository"] == definition["repository"], "Identidade divergente")
    for role, artifact in lock["downloads"].items():
        require(artifact["file"] == definition["downloads"][role]["file"], "Download diverge da definição")
        path = output / safe_name(artifact["file"])
        require(file_digest(path) == artifact["sha256"] and path.stat().st_size == artifact["bytes"], "Download alterado")
        inspect_zip(path.read_bytes(), artifact["file"])
        if role in {"overlay", "sdk"}:
            original = next(i for i in definition["inputs"] if i["role"] == role)
            require(artifact["sha256"] == original["sha256"], "Pacote original alterado")
    return lock


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    build = commands.add_parser("prepare", help="Monta quatro downloads a partir de fontes e pacotes fixados")
    for name in ("definition", "core-repo", "host-repo", "examples-repo", "artifacts", "evidence", "native-packages", "output"):
        build.add_argument("--" + name, required=True, type=Path)
    verify = commands.add_parser("verify", help="Confere arquivos, hashes e conteúdo da montagem")
    verify.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    try:
        if args.command == "prepare":
            prepare(args)
        else:
            lock = verify_output(args.output)
            print(json.dumps({"result": "ok", "tag": lock["tag"], "downloads": 4}))
        return 0
    except (ReleaseError, OSError, KeyError, json.JSONDecodeError, zipfile.BadZipFile, tomllib.TOMLDecodeError) as exc:
        print(f"ERRO: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
