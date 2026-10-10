#!/usr/bin/env python3
"""CLI de desenvolvimento ShipLua — scaffold, validação, testes e diagnóstico de mods.

Subcomandos (plan-sdk.md §16, release v0.1.0-alpha.1 §29):

- ``new``      — cria o scaffold de um mod (manifest.toml, main.lua, README.md, tests/).
- ``validate`` — valida manifestos (diretório, manifest.toml ou pacote .zip/.shipmod).
- ``test``     — executa os testes do mod no mock runtime via shiplua_mod_test_runner.
- ``doctor``   — diagnostica o ambiente de desenvolvimento e a saúde do repositório;
  com caminhos, valida ABI, exports e imports de mods nativos.

Códigos de saída: 0 = sucesso; 1 = falha (inválido/testes falharam/checks falharam);
2 = uso inválido; 3 = ferramenta necessária ausente.
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

try:
    import tomllib
except ModuleNotFoundError:  # Python < 3.11: fallback de validação indisponível
    tomllib = None  # type: ignore[assignment]

REPO_ROOT = Path(__file__).resolve().parents[1]

EXIT_OK = 0
EXIT_FAILURE = 1
EXIT_USAGE = 2
EXIT_MISSING_TOOL = 3

SEMVER = re.compile(
    r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)"
    r"(-[0-9A-Za-z.-]+)?(\+[0-9A-Za-z.-]+)?$"
)
SLUG = re.compile(r"^[a-z][a-z0-9]*(?:-[a-z0-9]+)*$")
GAMES = {"oot", "mm"}
MANIFEST_REQUIRED_FIELDS = ("id", "name", "version", "api", "entrypoint")

# ---------------------------------------------------------------------------
# Descoberta de ferramentas compiladas do repositório
# ---------------------------------------------------------------------------


def find_repo_tool(executable: str, env_var: str) -> Path | None:
    """Localiza um executável produzido pelo build do repositório.

    Ordem de busca: variável de ambiente explícita, diretórios ``build*`` na
    raiz do repositório (raiz e um nível, cobrindo geradores single-config e
    multi-config) e, por fim, o PATH do sistema.
    """
    override = os.environ.get(env_var)
    if override:
        candidate = Path(override)
        return candidate if candidate.is_file() else None

    names = (executable, executable + ".exe")
    for build_dir in sorted(REPO_ROOT.glob("build*")):
        if not build_dir.is_dir():
            continue
        for name in names:
            for candidate in (build_dir / name, *build_dir.glob(f"*/{name}")):
                if candidate.is_file():
                    return candidate

    on_path = shutil.which(executable)
    return Path(on_path) if on_path else None


# ---------------------------------------------------------------------------
# shipmod new
# ---------------------------------------------------------------------------

MANIFEST_TEMPLATE = """\
id = {mod_id}
name = {display_name}
version = "0.1.0"
api = ">=0.1 <0.6"
entrypoint = "main.lua"
description = {description}
authors = [{author}]
games = ["oot", "mm"]
"""

MAIN_LUA_TEMPLATE = """\
local ship = require("ship")

ship.events.on("game.ready", function()
    ship.log.info({mod_id} .. " carregado em " .. ship.game.id())
end)
"""

README_TEMPLATE = """\
# {display_name}

{description}

## Validar

```powershell
python tools/shipmod.py validate mods/{name}
```

## Testar (sem jogo, via mock runtime)

```powershell
python tools/shipmod.py test mods/{name}
```

O teste em `tests/` usa a DSL `describe`/`it`/`assert` do
`shiplua_mod_test_runner` (MODSDK-004) e roda sem jogo ou ROM.
"""

TEST_LUA_TEMPLATE = """\
-- Teste gerado por `shipmod new`: roda no mock runtime (MODSDK-004), sem jogo/ROM.
-- O runner executa o entrypoint e emite game.ready antes deste arquivo.
local ship = require("ship")

describe("{mod_id}", function()
    it("registra o carregamento no game.ready", function()
        local encontrado = false
        for _, entry in ipairs(mock.log.entries()) do
            if string.find(entry.message, "{mod_id} carregado", 1, true) ~= nil then
                encontrado = true
                break
            end
        end
        assert.is_true(encontrado, "esperava log de carregamento do mod")
    end)
end)
"""


def _git_author() -> str | None:
    try:
        result = subprocess.run(
            ["git", "config", "user.name"],
            capture_output=True,
            text=True,
            timeout=10,
        )
    except OSError:
        return None
    name = result.stdout.strip()
    return name or None


def cmd_new(args: argparse.Namespace) -> int:
    name: str = args.nome
    if not SLUG.fullmatch(name):
        print(
            f"Erro: nome de mod inválido '{name}' — use letras minúsculas, "
            "dígitos e hífens (ex.: kafei-puppet).",
            file=sys.stderr,
        )
        return EXIT_USAGE

    target = Path(args.dir) / name
    if target.exists() and any(target.iterdir()):
        print(f"Erro: o diretório {target} já existe e não está vazio.", file=sys.stderr)
        return EXIT_FAILURE

    mod_id = args.id or "community." + name.replace("-", "_")
    display_name = name.replace("-", " ").title()
    description = args.desc or f"Mod {display_name} gerado por shipmod new."
    author = args.autor or _git_author() or "desconhecido"
    test_slug = name.replace("-", "_")

    files = {
        target / "manifest.toml": MANIFEST_TEMPLATE.format(
            mod_id=json.dumps(mod_id, ensure_ascii=False),
            display_name=json.dumps(display_name, ensure_ascii=False),
            description=json.dumps(description, ensure_ascii=False),
            author=json.dumps(author, ensure_ascii=False),
        ),
        target / "main.lua": MAIN_LUA_TEMPLATE.format(
            mod_id=json.dumps(mod_id, ensure_ascii=False)
        ),
        target / "README.md": README_TEMPLATE.format(
            display_name=display_name, description=description, name=name
        ),
        target / "tests" / f"{test_slug}_test.lua": TEST_LUA_TEMPLATE.format(mod_id=mod_id),
    }
    for path, content in files.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"Criado: {path}")

    print()
    print(f"Mod '{name}' criado em {target} (id={mod_id}).")
    print("Próximos passos:")
    print(f"  python tools/shipmod.py validate {target}")
    print(f"  python tools/shipmod.py test {target}")
    return EXIT_OK


# ---------------------------------------------------------------------------
# shipmod validate
# ---------------------------------------------------------------------------


def _validate_manifest_data(data: object, origem: str) -> list[str]:
    """Verificação estrutural básica do manifesto (fallback sem o validador C++)."""
    errors: list[str] = []
    if not isinstance(data, dict):
        return [f"{origem}: documento raiz deve ser uma tabela TOML"]
    for field in MANIFEST_REQUIRED_FIELDS:
        value = data.get(field)
        if not isinstance(value, str) or not value.strip():
            errors.append(f"{origem}: campo obrigatório ausente ou inválido '{field}'")
    version = data.get("version")
    if isinstance(version, str) and version and not SEMVER.fullmatch(version):
        errors.append(f"{origem}: campo 'version' não é SemVer ({version!r})")
    games = data.get("games")
    if games is not None:
        if not isinstance(games, list) or any(g not in GAMES for g in games):
            errors.append(f"{origem}: campo 'games' deve lista apenas 'oot' e/ou 'mm'")
    return errors


def _validate_basic(path: Path) -> tuple[bool, str]:
    """Valida um manifesto/diretório/pacote sem o validador canônico."""
    if tomllib is None:
        return False, "o fallback de validação requer Python 3.11+ (tomllib)"

    origem = str(path)
    if path.is_dir():
        manifest = path / "manifest.toml"
        if not manifest.is_file():
            return False, f"{origem}: diretório sem manifest.toml"
        raw = manifest.read_bytes()
    elif path.is_file() and path.suffix.lower() in (".zip", ".shipmod"):
        try:
            with zipfile.ZipFile(path) as archive:
                raw = archive.read("manifest.toml")
        except (OSError, KeyError, zipfile.BadZipFile) as exc:
            return False, f"{origem}: pacote inválido ou sem manifest.toml ({exc})"
    elif path.is_file():
        raw = path.read_bytes()
    else:
        return False, f"{origem}: entrada ausente ou inacessível"

    try:
        data = tomllib.loads(raw.decode("utf-8"))
    except (tomllib.TOMLDecodeError, UnicodeDecodeError) as exc:
        return False, f"{origem}: erro de sintaxe TOML ({exc})"

    errors = _validate_manifest_data(data, origem)
    if errors:
        return False, "; ".join(errors)
    assert isinstance(data, dict)
    return True, f"id={data['id']}, versão={data['version']}"


def _run_delegated(command: list[str]) -> int:
    """Executa uma ferramenta delegada, diagnosticando falha de carga do executável."""
    try:
        result = subprocess.run(command)
    except OSError as exc:
        print(f"Erro ao executar {Path(command[0]).name}: {exc}", file=sys.stderr)
        return EXIT_MISSING_TOOL
    if result.returncode not in (EXIT_OK, EXIT_FAILURE, EXIT_USAGE):
        print(
            f"Aviso: {Path(command[0]).name} saiu com código {result.returncode} — "
            "verifique se o runtime do toolchain (ex.: DLLs do MinGW) está no PATH.",
            file=sys.stderr,
        )
    return result.returncode


def cmd_validate(args: argparse.Namespace) -> int:
    paths: list[str] = args.caminhos
    validator = find_repo_tool("shiplua_manifest_validator", "SHIPLUA_MANIFEST_VALIDATOR")
    if validator is not None:
        return _run_delegated([str(validator), *paths])

    print(
        "Aviso: shiplua_manifest_validator não encontrado — usando verificação "
        "estrutural básica.",
        file=sys.stderr,
    )
    print(
        "Para a validação canônica, compile: "
        "cmake --build build --target shiplua_manifest_validator",
        file=sys.stderr,
    )
    if tomllib is None:
        print("Erro: o fallback de validação requer Python 3.11+ (tomllib).", file=sys.stderr)
        return EXIT_MISSING_TOOL

    all_valid = True
    for raw_path in paths:
        ok, message = _validate_basic(Path(raw_path))
        if ok:
            print(f"Válido (verificação básica): {raw_path} — {message}")
        else:
            all_valid = False
            print(f"Inválido: {message}", file=sys.stderr)
    return EXIT_OK if all_valid else EXIT_FAILURE


# ---------------------------------------------------------------------------
# shipmod test
# ---------------------------------------------------------------------------


def cmd_test(args: argparse.Namespace) -> int:
    runner = find_repo_tool("shiplua_mod_test_runner", "SHIPLUA_MOD_TEST_RUNNER")
    if runner is None:
        print("Erro: shiplua_mod_test_runner não encontrado.", file=sys.stderr)
        print(
            "O executor de testes de mods (mock runtime) faz parte do MODSDK-004 "
            "e precisa ser compilado antes do uso.",
            file=sys.stderr,
        )
        print("Compile o runner com:", file=sys.stderr)
        print("  cmake --build build --target shiplua_mod_test_runner", file=sys.stderr)
        print(
            "Ou aponte um executável existente com a variável de ambiente "
            "SHIPLUA_MOD_TEST_RUNNER.",
            file=sys.stderr,
        )
        return EXIT_MISSING_TOOL

    command = [str(runner), args.caminho]
    if args.game:
        command += ["--game", args.game]
    for capability in args.capability:
        command += ["--capability", capability]
    return _run_delegated(command)


# ---------------------------------------------------------------------------
# shipmod doctor
# ---------------------------------------------------------------------------

STATUS_ICONS = {"ok": "[ok]  ", "aviso": "[aviso]", "falha": "[falha]"}


def _run_first_line(command: list[str]) -> str | None:
    try:
        result = subprocess.run(
            command, capture_output=True, text=True, timeout=30
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    if result.returncode != 0:
        return None
    lines = result.stdout.strip().splitlines()
    return lines[0] if lines else None


def _parse_version(text: str) -> tuple[int, ...] | None:
    match = re.search(r"(\d+(?:\.\d+)+)", text)
    if not match:
        return None
    return tuple(int(part) for part in match.group(1).split("."))


def _check_python() -> tuple[str, str, str]:
    version = platform.python_version()
    parsed = _parse_version(version) or ()
    if parsed >= (3, 11):
        return "ok", "Python", f"{version} (tomllib disponível para o fallback de validação)"
    if parsed >= (3, 10):
        return (
            "aviso",
            "Python",
            f"{version} — o CLI funciona, mas o fallback de validação requer 3.11+",
        )
    return "falha", "Python", f"{version} — o CLI requer Python 3.10+"


def _check_git() -> tuple[str, str, str]:
    line = _run_first_line(["git", "--version"])
    if line is None:
        return "falha", "git", "não encontrado no PATH"
    return "ok", "git", line


def _check_cmake() -> tuple[str, str, str]:
    line = _run_first_line(["cmake", "--version"])
    if line is None:
        return "falha", "CMake", "não encontrado no PATH (requerido >= 3.20)"
    parsed = _parse_version(line)
    if parsed is not None and parsed < (3, 20):
        return "falha", "CMake", f"{line} — o projeto requer >= 3.20"
    return "ok", "CMake", line


def _check_ninja() -> tuple[str, str, str]:
    line = _run_first_line(["ninja", "--version"])
    if line is None:
        return "aviso", "Ninja", "não encontrado (recomendado; alternativa: gerador Visual Studio)"
    return "ok", "Ninja", f"versão {line}"


def _check_compiler() -> tuple[str, str, str]:
    gpp = _run_first_line(["g++", "--version"])
    if gpp is not None:
        return "ok", "Compilador C++", gpp
    cl = _run_first_line(["cl"])
    if cl is not None:
        return "ok", "Compilador C++", cl
    return "aviso", "Compilador C++", "nenhum g++/cl encontrado no PATH"


def _check_repo_python_tools() -> tuple[str, str, str]:
    expected = [
        "generate_api_docs.py",
        "generate_cpp_api.py",
        "generate_api_contracts.py",
        "validate_api_schemas.py",
    ]
    missing = [name for name in expected if not (REPO_ROOT / "tools" / name).is_file()]
    if missing:
        return "falha", "Ferramentas Python do repo", "ausentes: " + ", ".join(missing)
    return "ok", "Ferramentas Python do repo", ", ".join(expected)


def _check_cpp_tool(executable: str, env_var: str, ausente: str) -> tuple[str, str, str]:
    found = find_repo_tool(executable, env_var)
    if found is not None:
        return "ok", executable, str(found)
    return "aviso", executable, ausente


def _run_repo_script(script: str, *extra: str) -> tuple[bool, str]:
    try:
        result = subprocess.run(
            [sys.executable, str(REPO_ROOT / "tools" / script), *extra],
            capture_output=True,
            text=True,
            cwd=REPO_ROOT,
            timeout=120,
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        return False, f"falha ao executar {script}: {exc}"
    if result.returncode == 0:
        return True, ""
    detail = (result.stderr or result.stdout).strip().splitlines()
    return False, detail[0] if detail else f"{script} saiu com código {result.returncode}"


def _check_schemas() -> tuple[str, str, str]:
    ok, detail = _run_repo_script("validate_api_schemas.py")
    if ok:
        return "ok", "Schemas da API", "schema/*.yml válidos"
    return "falha", "Schemas da API", detail


def _check_codegen() -> tuple[str, str, str]:
    problems: list[str] = []
    for script in (
        "generate_cpp_api.py",
        "generate_api_docs.py",
        "generate_api_contracts.py",
    ):
        ok, detail = _run_repo_script(script, "--check")
        if not ok:
            problems.append(f"{script}: {detail}")
    if problems:
        return "falha", "Codegen sem drift", "; ".join(problems)
    return "ok", "Codegen sem drift", "generated/ sincronizado com schema/"


def _check_examples() -> tuple[str, str, str]:
    examples_dir = REPO_ROOT / "tests" / "fixtures" / "mods"
    manifests = sorted(
        path.parent for path in examples_dir.glob("*/manifest.toml")
    )
    if not manifests:
        return "aviso", "Fixtures de mods", "nenhuma fixture encontrada em tests/fixtures/mods/"

    validator = find_repo_tool("shiplua_manifest_validator", "SHIPLUA_MANIFEST_VALIDATOR")
    if validator is not None:
        invalid: list[str] = []
        for mod_dir in manifests:
            try:
                result = subprocess.run(
                    [str(validator), str(mod_dir)], capture_output=True, text=True
                )
            except OSError:
                result = None
            if result is None or result.returncode not in (0, 1):
                return (
                    "aviso",
                    "Fixtures de mods",
                    "validador encontrado, mas não executa (runtime do toolchain, "
                    "ex.: DLLs do MinGW, está no PATH?)",
                )
            if result.returncode != 0:
                invalid.append(mod_dir.name)
        if invalid:
            return "falha", "Fixtures de mods", "inválidos: " + ", ".join(invalid)
        return "ok", "Fixtures de mods", f"{len(manifests)} fixture(s) válidas (validador canônico)"

    if tomllib is None:
        return "aviso", "Fixtures de mods", "sem validador compilado e sem Python 3.11+ para o fallback"
    invalid = [mod_dir.name for mod_dir in manifests if not _validate_basic(mod_dir)[0]]
    if invalid:
        return "falha", "Fixtures de mods", "inválidos (verificação básica): " + ", ".join(invalid)
    return "ok", "Fixtures de mods", f"{len(manifests)} fixture(s) válidas (verificação básica)"


# --- validador de ABI e exports de mods nativos (COREEXT-009, plan §9.9) ---

NATIVE_ABI_VERSIONS = ("1.0", "1.1", "1.2", "1.3")
NATIVE_QUERY_EXPORT = "ShipNative_Query"
PE_MACHINE_AMD64 = 0x8664
PE_FILE_DLL = 0x2000
DEBUG_CRT_IMPORTS = {"msvcp140d.dll", "vcruntime140d.dll", "vcruntime140_1d.dll", "ucrtbased.dll"}
SHA256_HEX = re.compile(r"^[0-9a-fA-F]{64}$")


def _host_abi_minor() -> int | None:
    """Minor da ABI declarada no cabeçalho do repositório (o que os hosts deste core aceitam)."""
    header = REPO_ROOT / "include" / "shiplua" / "native" / "ship_native_abi.h"
    try:
        match = re.search(r"#define\s+SHIP_NATIVE_ABI_MINOR\s+(\d+)u?", header.read_text(encoding="utf-8"))
    except OSError:
        return None
    return int(match.group(1)) if match else None


def read_pe(data: bytes) -> dict[str, object]:
    """Lê machine, flag de DLL, nomes exportados e DLLs importadas de um PE (PE32/PE32+).

    Lança ValueError quando o arquivo não é PE ou tem diretórios fora do arquivo.
    """
    def u16(offset: int) -> int:
        if offset + 2 > len(data):
            raise ValueError("PE truncado")
        return int.from_bytes(data[offset:offset + 2], "little")

    def u32(offset: int) -> int:
        if offset + 4 > len(data):
            raise ValueError("PE truncado")
        return int.from_bytes(data[offset:offset + 4], "little")

    if data[:2] != b"MZ":
        raise ValueError("sem cabeçalho MZ")
    pe = u32(0x3C)
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("sem assinatura PE")
    machine = u16(pe + 4)
    section_count = u16(pe + 6)
    optional_size = u16(pe + 20)
    characteristics = u16(pe + 22)
    optional = pe + 24
    magic = u16(optional)
    if magic == 0x20B:
        dir_count, dirs = u32(optional + 108), optional + 112
    elif magic == 0x10B:
        dir_count, dirs = u32(optional + 92), optional + 96
    else:
        raise ValueError(f"optional header desconhecido (0x{magic:x})")
    sections = []
    table = optional + optional_size
    for index in range(section_count):
        entry = table + index * 40
        sections.append((u32(entry + 12), max(u32(entry + 8), u32(entry + 16)), u32(entry + 20)))

    def offset_of(rva: int) -> int:
        for virtual, size, raw in sections:
            if virtual <= rva < virtual + size:
                return raw + (rva - virtual)
        raise ValueError(f"RVA 0x{rva:x} fora das seções")

    def c_string(rva: int) -> str:
        start = offset_of(rva)
        end = data.find(b"\0", start)
        if end < 0:
            raise ValueError("string sem terminador")
        return data[start:end].decode("ascii", errors="replace")

    exports: list[str] = []
    if dir_count > 0 and u32(dirs) != 0:
        directory = offset_of(u32(dirs))
        names = offset_of(u32(directory + 32)) if u32(directory + 24) else 0
        exports = [c_string(u32(names + 4 * i)) for i in range(u32(directory + 24))]
    imports: list[str] = []
    if dir_count > 1 and u32(dirs + 8) != 0:
        descriptor = offset_of(u32(dirs + 8))
        while u32(descriptor + 12) != 0 and len(imports) < 4096:
            imports.append(c_string(u32(descriptor + 12)))
            descriptor += 20
    return {
        "machine": machine,
        "dll": bool(characteristics & PE_FILE_DLL),
        "exports": exports,
        "imports": imports,
    }


def _open_mod(path: Path) -> tuple[dict | None, object, str]:
    """Abre diretório/pacote e devolve (manifesto, leitor de arquivo relativo, erro)."""
    if path.is_dir():
        def read_dir(relative: str) -> bytes | None:
            candidate = path / relative
            return candidate.read_bytes() if candidate.is_file() else None
        reader = read_dir
    elif path.is_file() and path.suffix.lower() in (".zip", ".shipmod"):
        try:
            with zipfile.ZipFile(path) as archive:
                entries = {
                    info.filename.replace("\\", "/"): archive.read(info)
                    for info in archive.infolist() if not info.is_dir()
                }
        except (OSError, zipfile.BadZipFile) as exc:
            return None, None, f"pacote inválido ({exc})"
        reader = entries.get
    else:
        return None, None, "esperado diretório, .zip ou .shipmod"
    raw = reader("manifest.toml")
    if raw is None:
        return None, None, "sem manifest.toml na raiz"
    if tomllib is None:
        return None, None, "a leitura do manifesto requer Python 3.11+ (tomllib)"
    try:
        data = tomllib.loads(raw.decode("utf-8"))
    except (tomllib.TOMLDecodeError, UnicodeDecodeError) as exc:
        return None, None, f"erro de sintaxe TOML ({exc})"
    return data, reader, ""


def _safe_relative(value: str) -> bool:
    normalized = value.replace("\\", "/")
    return bool(normalized) and not normalized.startswith("/") and ":" not in normalized and \
        ".." not in normalized.split("/")


def check_native_mod(path: Path, host_sha256: str | None = None) -> list[tuple[str, str, str]]:
    """Checks de ABI, exports e imports de um mod nativo (diretório, .zip ou .shipmod)."""
    data, reader, error = _open_mod(path)
    if data is None:
        return [("falha", "Manifesto", error)]
    mod_id = data.get("id", "?")
    provider = data.get("provider")
    if provider is None:
        return [("ok", "Provider", f"{mod_id}: sem [provider] (mod Lua, nada a validar)")]
    if not isinstance(provider, dict):
        return [("falha", "Provider", "[provider] precisa ser uma tabela")]

    checks: list[tuple[str, str, str]] = []
    kind = data.get("kind", "mod")
    phase = data.get("load_phase", "runtime")
    abi = provider.get("abi_version")
    if abi not in NATIVE_ABI_VERSIONS:
        checks.append(("falha", "ABI", f"abi_version {abi!r} fora de {', '.join(NATIVE_ABI_VERSIONS)}"))
    else:
        host_minor = _host_abi_minor()
        minor = int(abi.split(".")[1])
        if host_minor is not None and minor > host_minor:
            checks.append(("falha", "ABI", f"{abi} exige host com ABI 1.{minor}; este core vai até 1.{host_minor}"))
        else:
            checks.append(("ok", "ABI", f"{abi} (core aceita até 1.{host_minor if host_minor is not None else '?'})"))
    if kind == "core_extension":
        if phase != "pre_game":
            checks.append(("falha", "Tipo", "core_extension exige load_phase = \"pre_game\""))
        elif abi in ("1.0",):
            checks.append(("falha", "Tipo", "core_extension exige ABI 1.1 ou maior"))
        else:
            checks.append(("ok", "Tipo", "core_extension, pre_game"))
    elif phase == "pre_game":
        checks.append(("falha", "Tipo", "load_phase = \"pre_game\" é reservado a core_extension"))
    fingerprints = provider.get("host_fingerprints")
    if fingerprints is not None:
        valid = isinstance(fingerprints, list) and 0 < len(fingerprints) <= 32 and \
            all(isinstance(item, str) and SHA256_HEX.match(item) for item in fingerprints)
        if not valid:
            checks.append(("falha", "Fingerprints", "host_fingerprints precisa de 1-32 SHA-256 hex"))
        elif kind != "core_extension" or abi != "1.3":
            checks.append(("falha", "Fingerprints", "host_fingerprints exige core_extension com ABI 1.3"))
        elif host_sha256 is not None:
            listed = host_sha256.lower() in (item.lower() for item in fingerprints)
            checks.append(("ok" if listed else "aviso", "Fingerprints",
                           "host listado" if listed else "host fora da lista: escape hatch fica UNSUPPORTED"))
        else:
            checks.append(("ok", "Fingerprints", f"{len(fingerprints)} host(s); use --host para conferir"))
    elif abi == "1.3" and kind == "core_extension":
        checks.append(("aviso", "Fingerprints", "ABI 1.3 sem host_fingerprints: resolve/detour ficam UNSUPPORTED"))

    library = provider.get("win64")
    if not isinstance(library, str) or not library:
        checks.append(("aviso", "Biblioteca win64", "ausente: o mod não carrega no Windows"))
        return checks
    if not _safe_relative(library):
        checks.append(("falha", "Biblioteca win64", f"{library!r} precisa ser relativo e dentro do mod"))
        return checks
    binary = reader(library.replace("\\", "/"))
    if binary is None:
        checks.append(("falha", "Biblioteca win64", f"{library} não existe no mod"))
        return checks
    try:
        pe = read_pe(binary)
    except ValueError as exc:
        checks.append(("falha", "Biblioteca win64", f"{library}: {exc}"))
        return checks
    if pe["machine"] != PE_MACHINE_AMD64 or not pe["dll"]:
        checks.append(("falha", "Biblioteca win64",
                       f"{library}: precisa ser DLL x86-64 (machine 0x{pe['machine']:x}, dll={pe['dll']})"))
    else:
        checks.append(("ok", "Biblioteca win64", f"{library}: DLL x86-64, {len(binary)} bytes"))
    exports = pe["exports"]
    if NATIVE_QUERY_EXPORT in exports:
        checks.append(("ok", "Exports", f"{NATIVE_QUERY_EXPORT} ({len(exports)} export(s))"))
    else:
        checks.append(("falha", "Exports", f"{NATIVE_QUERY_EXPORT} não exportado (extern \"C\" + SHIP_NATIVE_EXPORT)"))
    imports = [name.lower() for name in pe["imports"]]
    executables = [name for name in imports if name.endswith(".exe")]
    debug_crt = sorted(set(imports) & DEBUG_CRT_IMPORTS)
    linked_core = [name for name in imports if name.startswith(("lua", "shiplua"))]
    if executables:
        checks.append(("falha", "Imports", "linka o executável (" + ", ".join(executables) +
                       "): use serviços e resolve_symbol, não import"))
    elif linked_core:
        checks.append(("falha", "Imports", "linka Lua/shiplua (" + ", ".join(linked_core) +
                       "): o provider usa só ship_native_abi.h"))
    elif debug_crt:
        checks.append(("aviso", "Imports", "CRT de debug (" + ", ".join(debug_crt) +
                       "): não existe em máquinas de jogador; compile em Release"))
    else:
        checks.append(("ok", "Imports", ", ".join(pe["imports"]) or "nenhum"))
    for field in ("linux64", "macos"):
        other = provider.get(field)
        if isinstance(other, str) and other and (not _safe_relative(other) or reader(other.replace("\\", "/")) is None):
            checks.append(("aviso", f"Biblioteca {field}", f"{other} ausente no mod"))
    return checks


def _print_checks(checks: list[tuple[str, str, str]]) -> tuple[int, int]:
    failures = warnings = 0
    for status, name, detail in checks:
        print(f"{STATUS_ICONS[status]} {name}: {detail}")
        failures += status == "falha"
        warnings += status == "aviso"
    return failures, warnings


def _doctor_mods(paths: list[str], host: str | None) -> int:
    host_sha256 = None
    if host:
        import hashlib
        try:
            host_sha256 = hashlib.sha256(Path(host).read_bytes()).hexdigest()
        except OSError as exc:
            print(f"Erro: não foi possível ler o host {host} ({exc})", file=sys.stderr)
            return EXIT_USAGE
        print(f"Host: {host} (SHA-256 {host_sha256})")
    total_failures = 0
    for raw_path in paths:
        print(f"shipmod doctor — {raw_path}")
        failures, warnings = _print_checks(check_native_mod(Path(raw_path), host_sha256))
        print(f"Resumo: {failures} falha(s), {warnings} aviso(s)")
        print()
        total_failures += failures
    return EXIT_FAILURE if total_failures else EXIT_OK


def cmd_doctor(args: argparse.Namespace) -> int:
    if getattr(args, "caminhos", None):
        return _doctor_mods(args.caminhos, getattr(args, "host", None))
    checks = [
        _check_python(),
        _check_git(),
        _check_cmake(),
        _check_ninja(),
        _check_compiler(),
        _check_repo_python_tools(),
        _check_cpp_tool(
            "shiplua_manifest_validator",
            "SHIPLUA_MANIFEST_VALIDATOR",
            "não compilado — cmake --build build --target shiplua_manifest_validator",
        ),
        _check_cpp_tool(
            "shiplua_mod_test_runner",
            "SHIPLUA_MOD_TEST_RUNNER",
            "não compilado — cmake --build build --target shiplua_mod_test_runner",
        ),
        _check_schemas(),
        _check_codegen(),
        _check_examples(),
    ]

    print("shipmod doctor — diagnóstico do ambiente de desenvolvimento ShipLua")
    print(f"Repositório: {REPO_ROOT}")
    print()
    failures = 0
    warnings = 0
    for status, name, detail in checks:
        print(f"{STATUS_ICONS[status]} {name}: {detail}")
        failures += status == "falha"
        warnings += status == "aviso"
    print()
    print(f"Resumo: {len(checks) - failures - warnings} ok, {warnings} aviso(s), {failures} falha(s)")
    return EXIT_FAILURE if failures else EXIT_OK


# ---------------------------------------------------------------------------
# Parser e entrada
# ---------------------------------------------------------------------------


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="shipmod",
        description="CLI de desenvolvimento de mods ShipLua (plan-sdk.md §16).",
    )
    parser.add_argument("--version", action="version", version="%(prog)s 0.1.0-alpha.1")
    subparsers = parser.add_subparsers(dest="comando", required=True, metavar="subcomando")

    p_new = subparsers.add_parser("new", help="cria o scaffold de um mod novo")
    p_new.add_argument("nome", help="nome do diretório do mod (slug, ex.: kafei-puppet)")
    p_new.add_argument("--dir", default=".", help="diretório pai (padrão: diretório atual)")
    p_new.add_argument("--id", help="id do mod (padrão: community.<nome_com_sublinhados>)")
    p_new.add_argument("--autor", help="autor do mod (padrão: user.name do git)")
    p_new.add_argument("--desc", help="descrição curta do mod")
    p_new.set_defaults(func=cmd_new)

    p_validate = subparsers.add_parser(
        "validate", help="valida manifesto de mod (diretório, manifest.toml, .zip ou .shipmod)"
    )
    p_validate.add_argument("caminhos", nargs="+", help="caminhos a validar")
    p_validate.set_defaults(func=cmd_validate)

    p_test = subparsers.add_parser(
        "test", help="executa os testes do mod no mock runtime (MODSDK-004)"
    )
    p_test.add_argument("caminho", help="diretório do mod ou pacote .zip/.shipmod")
    p_test.add_argument("--game", choices=sorted(GAMES), help="jogo simulado pelo mock")
    p_test.add_argument(
        "--capability",
        action="append",
        default=[],
        help="capability extra concedida ao mock (repetível)",
    )
    p_test.set_defaults(func=cmd_test)

    p_doctor = subparsers.add_parser(
        "doctor",
        help="diagnostica o ambiente; com caminhos, valida ABI, exports e imports de mods nativos",
    )
    p_doctor.add_argument(
        "caminhos", nargs="*", help="mods nativos a validar (diretório, .zip ou .shipmod)"
    )
    p_doctor.add_argument(
        "--host", help="executável do jogo, para conferir host_fingerprints da ABI 1.3"
    )
    p_doctor.set_defaults(func=cmd_doctor)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
