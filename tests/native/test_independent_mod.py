"""Prova de extensão: host imutável, provider compilado e alterado separadamente."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import tempfile
import zipfile


def run(*args):
    result = subprocess.run([str(arg) for arg in args], capture_output=True, text=True,
                            encoding="utf-8", errors="replace", timeout=180)
    if result.returncode:
        raise RuntimeError(f"Comando falhou: {args}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", type=Path, required=True)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--generator", default="")
    parser.add_argument("--platform", default="")
    args = parser.parse_args()
    before = hashlib.sha256(args.host.read_bytes()).hexdigest()
    with tempfile.TemporaryDirectory(prefix="linkspan-independent-") as scratch:
        build = Path(scratch) / "provider-build"
        for velocity in (12, 21):
            configure = [args.cmake, "-S", args.sdk / "examples/native-dash", "-B", build,
                         f"-DDASH_VELOCITY={velocity}.0"]
            if args.generator:
                configure += ["-G", args.generator]
            if args.platform:
                configure += ["-A", args.platform]
            run(*configure)
            run(args.cmake, "--build", build, "--config", "Release")
            mod = build / "mod"
            package = Path(scratch) / "native-dash.zip"
            with zipfile.ZipFile(package, "w", zipfile.ZIP_DEFLATED) as archive:
                for path in sorted(mod.rglob("*")):
                    if path.is_file() and path.suffix in (".lua", ".toml", ".dll", ".so", ".dylib"):
                        archive.write(path, path.relative_to(mod).as_posix())
            print(run(args.host, mod, velocity, "oot").strip())
            print(run(args.host, package, velocity, "mm").strip())
            legacy = package.with_suffix(".shipmod")
            legacy.write_bytes(package.read_bytes())
            print(run(args.host, legacy, velocity, "oot").strip())
            after = hashlib.sha256(args.host.read_bytes()).hexdigest()
            if after != before:
                raise AssertionError("O executável do host mudou durante o teste")
    print(f"Host SHA-256 inalterado nas duas versões do mod: {before}")


if __name__ == "__main__":
    main()
