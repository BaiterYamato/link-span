"""Valida ZIP no CLI canônico e executa testes Lua reais sem jogo/ROM."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import zipfile


def run(*command, expected=0, env=None):
    result = subprocess.run([str(part) for part in command], capture_output=True,
                            text=True, encoding="utf-8", errors="replace", timeout=30, env=env)
    if result.returncode != expected:
        raise AssertionError(f"{command}: {result.returncode}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--validator", required=True)
    parser.add_argument("--runner", required=True)
    parser.add_argument("--sdk", type=Path, required=True)
    args = parser.parse_args()
    fixture = args.sdk / "tests/fixtures/mod-runner/passing-mod"
    env = dict(os.environ, SHIPLUA_MANIFEST_VALIDATOR=args.validator,
               SHIPLUA_MOD_TEST_RUNNER=args.runner)
    with tempfile.TemporaryDirectory(prefix="linkspan-zip-tools-") as scratch:
        root = Path(scratch)
        for suffix in (".zip", ".ZIP", ".shipmod", ".SHIPMOD"):
            package = root / ("mod" + suffix)
            with zipfile.ZipFile(package, "w", zipfile.ZIP_DEFLATED) as archive:
                for path in sorted(fixture.rglob("*")):
                    if path.is_file():
                        archive.write(path, path.relative_to(fixture).as_posix())
            run(args.validator, package)
            for game in ("oot", "mm"):
                run(args.runner, package, "--game", game)
            for command in ("validate", "test"):
                run(sys.executable, args.sdk / "tools/shipmod.py", command, package, env=env)

        for entry in ("notes.txt", "mod/manifest.toml", "Manifest.toml"):
            package = root / "missing-root.zip"
            with zipfile.ZipFile(package, "w") as archive:
                archive.write(fixture / "manifest.toml", entry)
            run(args.validator, package, expected=1)
            run(args.runner, package, expected=1)

        package = root / "unsafe.zip"
        with zipfile.ZipFile(package, "w") as archive:
            archive.write(fixture / "manifest.toml", "manifest.toml")
            archive.writestr("../outside.lua", "error('escape')")
        run(args.validator, package, expected=1)
        run(args.runner, package, expected=1)
        package.write_bytes(b"PK\x03\x04truncated")
        run(args.validator, package, expected=1)
        run(args.runner, package, expected=1)
    print("ZIP/SHIPMOD: CLI, validador, testes Lua oot/mm e rejeição de pacotes inválidos aprovados.")


if __name__ == "__main__":
    main()
