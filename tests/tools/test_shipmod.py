"""Testes do CLI de desenvolvimento `tools/shipmod.py` (MODSDK-006).

Segue a convenção do repositório: unittest executado via ctest com o
interpretador Python encontrado pelo CMake.
"""

from __future__ import annotations

import contextlib
import io
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))

import shipmod  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parents[2]


def run_cli(argv: list[str]) -> tuple[int, str, str]:
    """Executa o CLI capturando stdout/stderr, sem encerrar o processo."""
    stdout, stderr = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
        try:
            code = shipmod.main(argv)
        except SystemExit as exit_:  # erros de argparse saem com código 2
            code = exit_.code if isinstance(exit_.code, int) else 2
    return code, stdout.getvalue(), stderr.getvalue()


class NewCommandTests(unittest.TestCase):
    def test_scaffold_creates_expected_structure(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            code, out, _ = run_cli(["new", "kafei-puppet", "--dir", temporary])
            self.assertEqual(code, shipmod.EXIT_OK)

            mod_dir = Path(temporary) / "kafei-puppet"
            manifest = (mod_dir / "manifest.toml").read_text(encoding="utf-8")
            self.assertIn('id = "community.kafei_puppet"', manifest)
            self.assertIn('api = ">=0.1 <0.6"', manifest)
            self.assertIn('entrypoint = "main.lua"', manifest)
            self.assertTrue((mod_dir / "main.lua").is_file())
            self.assertTrue((mod_dir / "README.md").is_file())
            test_file = mod_dir / "tests" / "kafei_puppet_test.lua"
            self.assertTrue(test_file.is_file())
            self.assertIn("describe(", test_file.read_text(encoding="utf-8"))
            self.assertIn("community.kafei_puppet", out)

    @unittest.skipIf(shipmod.tomllib is None, "requer Python 3.11+ (tomllib)")
    def test_scaffold_manifest_passes_basic_validation(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            code, _, _ = run_cli(["new", "puppet-show", "--dir", temporary])
            self.assertEqual(code, shipmod.EXIT_OK)
            ok, message = shipmod._validate_basic(Path(temporary) / "puppet-show")
            self.assertTrue(ok, message)

    def test_scaffold_honors_custom_id_author_and_description(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            code, _, _ = run_cli([
                "new", "meu-mod", "--dir", temporary,
                "--id", "baiteryamato.meu_mod",
                "--autor", "Kimi",
                "--desc", "Descrição personalizada.",
            ])
            self.assertEqual(code, shipmod.EXIT_OK)
            manifest = (Path(temporary) / "meu-mod" / "manifest.toml").read_text(encoding="utf-8")
            self.assertIn('id = "baiteryamato.meu_mod"', manifest)
            self.assertIn('authors = ["Kimi"]', manifest)
            self.assertIn('description = "Descrição personalizada."', manifest)

    def test_scaffold_escapes_toml_and_lua_strings(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            code, _, _ = run_cli([
                "new", "meu-mod", "--dir", temporary,
                "--id", 'community.meu_mod"quoted',
                "--autor", 'Autor "Teste"',
                "--desc", "Linha 1\nLinha 2",
            ])
            self.assertEqual(code, shipmod.EXIT_OK)
            mod_dir = Path(temporary) / "meu-mod"
            ok, message = shipmod._validate_basic(mod_dir)
            self.assertTrue(ok, message)
            main_lua = (mod_dir / "main.lua").read_text(encoding="utf-8")
            self.assertIn(r'community.meu_mod\"quoted', main_lua)

    def test_scaffold_refuses_invalid_name(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            for invalid in ("MeuMod", "mod_name", "1mod"):
                code, _, err = run_cli(["new", invalid, "--dir", temporary])
                self.assertEqual(code, shipmod.EXIT_USAGE, invalid)
                self.assertIn("inválido", err)
            # "-mod" é interceptado pelo argparse como opção desconhecida (uso inválido)
            code, _, _ = run_cli(["new", "--", "-mod", "--dir", temporary])
            self.assertEqual(code, shipmod.EXIT_USAGE)

    def test_scaffold_refuses_non_empty_directory(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            target = Path(temporary) / "kafei-puppet"
            target.mkdir()
            (target / "ocupado.txt").write_text("x", encoding="utf-8")
            code, _, err = run_cli(["new", "kafei-puppet", "--dir", temporary])
            self.assertEqual(code, shipmod.EXIT_FAILURE)
            self.assertIn("não está vazio", err)


class ValidateFallbackTests(unittest.TestCase):
    """Fallback estrutural em Python, sem o validador canônico compilado."""

    def setUp(self) -> None:
        if shipmod.tomllib is None:
            self.skipTest("requer Python 3.11+ (tomllib)")
        patcher = mock.patch.object(shipmod, "find_repo_tool", return_value=None)
        patcher.start()
        self.addCleanup(patcher.stop)

    def test_valid_example_directory(self) -> None:
        code, out, _ = run_cli(["validate", str(REPO_ROOT / "tests" / "fixtures" / "mods" / "hello-world")])
        self.assertEqual(code, shipmod.EXIT_OK)
        self.assertIn("example.hello_world", out)

    def test_valid_manifest_file(self) -> None:
        fixture = REPO_ROOT / "tests" / "fixtures" / "manifest" / "valid-minimal.toml"
        code, _, _ = run_cli(["validate", str(fixture)])
        self.assertEqual(code, shipmod.EXIT_OK)

    def test_invalid_version_fixture(self) -> None:
        fixture = REPO_ROOT / "tests" / "fixtures" / "manifest" / "invalid-version.toml"
        code, _, err = run_cli(["validate", str(fixture)])
        self.assertEqual(code, shipmod.EXIT_FAILURE)
        self.assertIn("Inválido", err)

    def test_missing_path(self) -> None:
        code, _, err = run_cli(["validate", "caminho/inexistente"])
        self.assertEqual(code, shipmod.EXIT_FAILURE)
        self.assertIn("Inválido", err)

    def test_mod_packages(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            package = Path(temporary) / "pacote.shipmod"
            with zipfile.ZipFile(package, "w") as archive:
                archive.writestr(
                    "manifest.toml",
                    'id = "community.teste"\nname = "Teste"\nversion = "1.0.0"\n'
                    'api = ">=0.1"\nentrypoint = "main.lua"\n',
                )
                archive.writestr("main.lua", 'local ship = require("ship")\n')
            code, out, _ = run_cli(["validate", str(package)])
            self.assertEqual(code, shipmod.EXIT_OK)
            self.assertIn("community.teste", out)
            for extension in (".zip", ".ZIP", ".SHIPMOD"):
                with self.subTest(extension=extension):
                    renamed = package.with_suffix(extension)
                    renamed.write_bytes(package.read_bytes())
                    code, out, _ = run_cli(["validate", str(renamed)])
                    self.assertEqual(code, shipmod.EXIT_OK)
                    self.assertIn("community.teste", out)

    def test_zip_requires_root_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            package = Path(temporary) / "ordinary.zip"
            for entry in ("notes.txt", "mod/manifest.toml", "Manifest.toml"):
                with self.subTest(entry=entry):
                    with zipfile.ZipFile(package, "w") as archive:
                        archive.writestr(entry, "some content")
                    code, _, _ = run_cli(["validate", str(package)])
                    self.assertEqual(code, shipmod.EXIT_FAILURE)


class ValidateDelegationTests(unittest.TestCase):
    """Com o validador canônico presente, o CLI apenas delega e propaga o código."""

    def test_delegates_to_canonical_validator(self) -> None:
        fake = Path("C:/fake/shiplua_manifest_validator.exe")
        completed = mock.Mock(returncode=0)
        with mock.patch.object(shipmod, "find_repo_tool", return_value=fake), \
             mock.patch.object(shipmod.subprocess, "run", return_value=completed) as run:
            code, _, _ = run_cli(["validate", "a", "b"])
        self.assertEqual(code, 0)
        run.assert_called_once_with([str(fake), "a", "b"])

    def test_propagates_validator_failure(self) -> None:
        fake = Path("C:/fake/shiplua_manifest_validator.exe")
        completed = mock.Mock(returncode=1)
        with mock.patch.object(shipmod, "find_repo_tool", return_value=fake), \
             mock.patch.object(shipmod.subprocess, "run", return_value=completed):
            code, _, _ = run_cli(["validate", "mod-invalido"])
        self.assertEqual(code, 1)

    def test_unexpected_validator_exit_is_diagnosed(self) -> None:
        fake = Path("C:/fake/shiplua_manifest_validator.exe")
        completed = mock.Mock(returncode=127)
        with mock.patch.object(shipmod, "find_repo_tool", return_value=fake), \
             mock.patch.object(shipmod.subprocess, "run", return_value=completed):
            code, _, err = run_cli(["validate", "mod"])
        self.assertEqual(code, 127)
        self.assertIn("127", err)
        self.assertIn("PATH", err)


class TestCommandTests(unittest.TestCase):
    def test_missing_runner_fails_with_clear_message(self) -> None:
        with mock.patch.object(shipmod, "find_repo_tool", return_value=None):
            code, _, err = run_cli(["test", "tests/fixtures/mods/hello-runtime"])
        self.assertEqual(code, shipmod.EXIT_MISSING_TOOL)
        self.assertIn("shiplua_mod_test_runner", err)
        self.assertIn("MODSDK-004", err)

    def test_delegates_with_game_and_capabilities(self) -> None:
        fake = Path("C:/fake/shiplua_mod_test_runner.exe")
        completed = mock.Mock(returncode=0)
        with mock.patch.object(shipmod, "find_repo_tool", return_value=fake), \
             mock.patch.object(shipmod.subprocess, "run", return_value=completed) as run:
            code, _, _ = run_cli([
                "test", "meu-mod", "--game", "mm",
                "--capability", "core.input", "--capability", "mm.cycle",
            ])
        self.assertEqual(code, 0)
        run.assert_called_once_with([
            str(fake), "meu-mod", "--game", "mm",
            "--capability", "core.input", "--capability", "mm.cycle",
        ])


class DoctorTests(unittest.TestCase):
    def test_python_check_ok_on_supported_version(self) -> None:
        status, name, _ = shipmod._check_python()
        self.assertEqual(name, "Python")
        if sys.version_info >= (3, 11):
            self.assertEqual(status, "ok")
        else:
            self.assertIn(status, {"aviso", "falha"})

    def test_parse_version(self) -> None:
        self.assertEqual(shipmod._parse_version("cmake version 3.28.1"), (3, 28, 1))
        self.assertIsNone(shipmod._parse_version("sem versão"))

    def test_doctor_runs_all_checks_and_summarizes(self) -> None:
        code, out, _ = run_cli(["doctor"])
        self.assertIn(code, (shipmod.EXIT_OK, shipmod.EXIT_FAILURE))
        for expected in ("Python", "git", "CMake", "Schemas", "Resumo"):
            self.assertIn(expected, out)


def build_pe(exports: list[str], imports: list[str], machine: int = 0x8664, dll: bool = True) -> bytes:
    """PE32+ mínimo com uma seção contendo os diretórios de export e import."""
    section_rva, section_raw = 0x1000, 0x200
    body = bytearray()

    def add_string(text: str) -> int:
        rva = section_rva + len(body)
        body.extend(text.encode("ascii") + b"\0")
        return rva

    name_rvas = [add_string(name) for name in exports]
    import_name_rvas = [add_string(name) for name in imports]
    while len(body) % 4:
        body.append(0)
    names_table = section_rva + len(body)
    for rva in name_rvas:
        body.extend(rva.to_bytes(4, "little"))
    export_dir = section_rva + len(body)
    export = bytearray(40)
    export[24:28] = len(exports).to_bytes(4, "little")
    export[32:36] = names_table.to_bytes(4, "little")
    body.extend(export)
    import_dir = section_rva + len(body)
    for rva in import_name_rvas:
        descriptor = bytearray(20)
        descriptor[12:16] = rva.to_bytes(4, "little")
        body.extend(descriptor)
    body.extend(bytes(20))

    pe = 0x40
    optional_size = 112 + 16 * 8
    header = bytearray(section_raw)
    header[0:2] = b"MZ"
    header[0x3C:0x40] = pe.to_bytes(4, "little")
    header[pe:pe + 4] = b"PE\0\0"
    header[pe + 4:pe + 6] = machine.to_bytes(2, "little")
    header[pe + 6:pe + 8] = (1).to_bytes(2, "little")
    header[pe + 20:pe + 22] = optional_size.to_bytes(2, "little")
    header[pe + 22:pe + 24] = (0x2022 if dll else 0x0022).to_bytes(2, "little")
    optional = pe + 24
    header[optional:optional + 2] = (0x20B).to_bytes(2, "little")
    header[optional + 108:optional + 112] = (16).to_bytes(4, "little")
    dirs = optional + 112
    header[dirs:dirs + 4] = export_dir.to_bytes(4, "little")
    header[dirs + 8:dirs + 12] = import_dir.to_bytes(4, "little")
    section = optional + optional_size
    header[section:section + 6] = b".rdata"
    header[section + 8:section + 12] = len(body).to_bytes(4, "little")
    header[section + 12:section + 16] = section_rva.to_bytes(4, "little")
    header[section + 16:section + 20] = len(body).to_bytes(4, "little")
    header[section + 20:section + 24] = section_raw.to_bytes(4, "little")
    return bytes(header) + bytes(body)


NATIVE_MANIFEST = """\
id = "teste.nativo"
name = "Teste nativo"
version = "1.0.0"
api = ">=0.5 <1.0"
entrypoint = "main.lua"
{extra}
[provider]
abi_version = "{abi}"
win64 = "provider/teste.dll"
{provider_extra}"""


@unittest.skipIf(shipmod.tomllib is None, "requer Python 3.11+ (tomllib)")
class NativeDoctorTests(unittest.TestCase):
    def make_mod(self, temporary: str, dll: bytes | None, abi: str = "1.0", extra: str = "",
                 provider_extra: str = "", as_zip: bool = False) -> Path:
        manifest = NATIVE_MANIFEST.format(abi=abi, extra=extra, provider_extra=provider_extra)
        if as_zip:
            package = Path(temporary) / "teste.shipmod"
            with zipfile.ZipFile(package, "w") as archive:
                archive.writestr("manifest.toml", manifest)
                archive.writestr("main.lua", "")
                if dll is not None:
                    archive.writestr("provider/teste.dll", dll)
            return package
        mod = Path(temporary) / "mod"
        (mod / "provider").mkdir(parents=True)
        (mod / "manifest.toml").write_text(manifest, encoding="utf-8")
        if dll is not None:
            (mod / "provider" / "teste.dll").write_bytes(dll)
        return mod

    @staticmethod
    def statuses(checks: list[tuple[str, str, str]]) -> dict[str, str]:
        return {name: status for status, name, _ in checks}

    def test_read_pe_returns_exports_and_imports(self) -> None:
        pe = shipmod.read_pe(build_pe(["ShipNative_Query", "Outro"], ["KERNEL32.dll"]))
        self.assertEqual(pe["machine"], 0x8664)
        self.assertTrue(pe["dll"])
        self.assertEqual(pe["exports"], ["ShipNative_Query", "Outro"])
        self.assertEqual(pe["imports"], ["KERNEL32.dll"])

    def test_read_pe_rejects_non_pe(self) -> None:
        with self.assertRaises(ValueError):
            shipmod.read_pe(b"nao e um pe")

    def test_valid_package_passes(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            package = self.make_mod(temporary, build_pe(["ShipNative_Query"], ["KERNEL32.dll"]), as_zip=True)
            code, out, _ = run_cli(["doctor", str(package)])
        self.assertEqual(code, shipmod.EXIT_OK, out)
        self.assertIn("0 falha(s)", out)

    def test_missing_query_export_fails(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, build_pe(["Outro"], ["KERNEL32.dll"]))
            checks = shipmod.check_native_mod(mod)
        self.assertEqual(self.statuses(checks)["Exports"], "falha")

    def test_wrong_machine_and_exe_import_fail(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, build_pe(["ShipNative_Query"], ["soh.exe"], machine=0x14C))
            checks = self.statuses(shipmod.check_native_mod(mod))
        self.assertEqual(checks["Biblioteca win64"], "falha")
        self.assertEqual(checks["Imports"], "falha")

    def test_debug_crt_is_a_warning(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, build_pe(["ShipNative_Query"], ["VCRUNTIME140D.dll"]))
            checks = self.statuses(shipmod.check_native_mod(mod))
        self.assertEqual(checks["Imports"], "aviso")

    def test_missing_library_and_traversal_fail(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, None)
            self.assertEqual(self.statuses(shipmod.check_native_mod(mod))["Biblioteca win64"], "falha")
        self.assertFalse(shipmod._safe_relative("../fora.dll"))
        self.assertFalse(shipmod._safe_relative("C:/fora.dll"))
        self.assertTrue(shipmod._safe_relative("provider/ok.dll"))

    def test_core_extension_rules(self) -> None:
        dll = build_pe(["ShipNative_Query"], ["KERNEL32.dll"])
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, dll, abi="1.0", extra='kind = "core_extension"\nload_phase = "pre_game"')
            self.assertEqual(self.statuses(shipmod.check_native_mod(mod))["Tipo"], "falha")
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, dll, abi="9.9")
            self.assertEqual(self.statuses(shipmod.check_native_mod(mod))["ABI"], "falha")

    def test_fingerprints_checked_against_host(self) -> None:
        dll = build_pe(["ShipNative_Query"], ["KERNEL32.dll"])
        extra = 'kind = "core_extension"\nload_phase = "pre_game"'
        with tempfile.TemporaryDirectory() as temporary:
            mod = self.make_mod(temporary, dll, abi="1.3", extra=extra,
                                provider_extra=f'host_fingerprints = ["{"a" * 64}"]')
            listed = self.statuses(shipmod.check_native_mod(mod, "A" * 64))
            other = self.statuses(shipmod.check_native_mod(mod, "b" * 64))
        self.assertEqual(listed["Fingerprints"], "ok")
        self.assertEqual(other["Fingerprints"], "aviso")

    def test_lua_mod_has_nothing_to_validate(self) -> None:
        code, out, _ = run_cli(["doctor", str(REPO_ROOT / "tests" / "fixtures" / "mods" / "hello-runtime")])
        self.assertEqual(code, shipmod.EXIT_OK, out)
        self.assertIn("sem [provider]", out)


if __name__ == "__main__":
    unittest.main()
