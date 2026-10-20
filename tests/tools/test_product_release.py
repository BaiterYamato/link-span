"""Regressões da montagem pública: integridade, conteúdo e proveniência."""
import copy
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import stat
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("product_release", ROOT / "tools/release/product_release.py")
release = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(release)


def zip_bytes(entries):
    out = io.BytesIO()
    with zipfile.ZipFile(out, "w") as z:
        for name, data in entries.items():
            z.writestr(name, data)
    return out.getvalue()


def definition():
    d = json.loads((ROOT / "releases/oot-0.5.0-rc9/release.json").read_text(encoding="utf-8"))
    d["native_examples"] = []
    return d


class ProductReleaseTests(unittest.TestCase):
    def test_rejects_traversal_and_windows_paths(self):
        for name in ("../bad", "/bad", "C:/bad", "a\\b", "a/../b", "a//b"):
            with self.subTest(name=name), self.assertRaises(release.ReleaseError):
                release.safe_name(name)

    def test_rejects_nested_rom(self):
        inner = zip_bytes({"game.z64": b"payload"})
        with self.assertRaises(release.ReleaseError):
            release.inspect_zip(zip_bytes({"nested.zip": inner}), "outer")

    def test_rejects_rom_magic_under_other_extension(self):
        with self.assertRaises(release.ReleaseError):
            release.inspect_zip(zip_bytes({"data.bin": b"\x80\x37\x12\x40abcd"}), "outer")

    def test_rejects_symlink_in_archive(self):
        out = io.BytesIO()
        with zipfile.ZipFile(out, "w") as z:
            info = zipfile.ZipInfo("link")
            info.create_system = 3
            info.external_attr = (stat.S_IFLNK | 0o777) << 16
            z.writestr(info, "outside")
        with self.assertRaises(release.ReleaseError):
            release.inspect_zip(out.getvalue(), "outer")

    def test_allows_save_source_but_rejects_personal_save(self):
        release.safe_payload("src/save/PortableSave.cpp", b"source")
        with self.assertRaises(release.ReleaseError):
            release.safe_payload("Save/file1.sav", b"save")

    def test_determinism_is_independent_of_input_order(self):
        self.assertEqual(release.deterministic_zip({"b": b"2", "a": b"1"}),
                         release.deterministic_zip({"a": b"1", "b": b"2"}))

    def test_definition_refuses_moving_branch(self):
        d = definition()
        d["sources"]["core"]["commit"] = "main"
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp) / "definition.json"
            p.write_text(json.dumps(d), encoding="utf-8")
            with self.assertRaises(release.ReleaseError):
                release.read_definition(p)

    def test_definition_requires_exactly_four_downloads(self):
        d = definition()
        del d["downloads"]["source"]
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp) / "definition.json"
            p.write_text(json.dumps(d), encoding="utf-8")
            with self.assertRaises(release.ReleaseError):
                release.read_definition(p)

    def test_host_must_record_approved_core(self):
        d = definition()
        with patch.object(release, "git", side_effect=lambda repo, *args: d["sources"][repo]["commit"].encode()), \
             patch.object(release, "submodules", return_value=[{"path": "extern/ship-lua", "commit": "0" * 40}]):
            with self.assertRaisesRegex(release.ReleaseError, "outro commit"):
                release.validate_sources(d, {r: r for r in d["sources"]})

    def test_mod_requires_entrypoint_and_dll(self):
        manifest = b'id="example"\nversion="0.1.0"\nentrypoint="main.lua"\n[provider]\nwin64="provider/missing.dll"\n'
        with self.assertRaises(release.ReleaseError):
            release.validate_package(zip_bytes({"manifest.toml": manifest, "main.lua": b""}), definition())

    def test_mod_cannot_pin_other_host(self):
        manifest = b'id="example"\nversion="0.1.0"\n[provider]\nhost_fingerprints=["wrong"]\n'
        with self.assertRaisesRegex(release.ReleaseError, "outro executável"):
            release.validate_package(zip_bytes({"manifest.toml": manifest, "main.lua": b""}), definition())

    def test_mod_layout_must_match_release_host(self):
        d = definition()
        other = b'id="example"\nversion="0.1.0"\n[provider]\nhost_layout_id="' + b"0" * 64 + b'"\n'
        with self.assertRaisesRegex(release.ReleaseError, "Layout do mod"):
            release.validate_package(zip_bytes({"manifest.toml": other, "main.lua": b""}), d)
        same = ('id="example"\nversion="0.1.0"\n[provider]\nhost_layout_id="'
                + d["host"]["layout"].upper() + '"\n').encode()
        self.assertEqual(release.validate_package(zip_bytes({"manifest.toml": same, "main.lua": b""}), d)["id"],
                         "example")

    def test_examples_separate_mm_only_and_preserve_packages(self):
        d = definition()
        d["inputs"] = [{"file": "a.zip", "role": "mod", "sha256": "a" * 64}]
        files = {
            "lua/oot/manifest.toml": b'id="example.oot"\nversion="0.1.0"\ngames=["oot"]\n',
            "lua/oot/main.lua": b"return true",
            "lua/mm/manifest.toml": b'id="example.mm"\nversion="0.1.0"\ngames=["mm"]\n',
            "lua/mm/main.lua": b"return true"}
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp)
            (p / "native-lock.json").write_text(json.dumps({"examples_commit": d["sources"]["examples"]["commit"],
                "layout": d["host"]["layout"], "packages": {}}))
            packed, catalog = release.examples_archive(d, files, {"a.zip": b"original"}, p)
            with zipfile.ZipFile(io.BytesIO(packed)) as z:
                self.assertIn("lua-mm/mm.shipmod", z.namelist())
                self.assertIn("lua-oot/oot.shipmod", z.namelist())
                self.assertEqual(z.read("release-mods/a.zip"), b"original")
                self.assertIn("README.txt", z.namelist())
                self.assertIn("CATALOG.json", z.namelist())
            self.assertEqual(len(catalog), 3)

    def test_native_catalog_cannot_silently_skip_demo(self):
        d = definition()
        d["native_examples"] = [{"name": "missing", "id": "example.missing"}]
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp)
            (p / "native-lock.json").write_text(json.dumps({"examples_commit": d["sources"]["examples"]["commit"],
                "layout": d["host"]["layout"], "packages": {}}))
            with self.assertRaisesRegex(release.ReleaseError, "incompleto"):
                release.examples_archive(d, {}, {}, p)

    def test_public_links_stay_on_central_repository(self):
        d = definition()
        text = release.release_body(d)
        for item in d["downloads"].values():
            self.assertIn(f"https://github.com/BaiterYamato/link-span/releases/download/{d['tag']}/{item['file']}", text)
        self.assertNotIn("releases/download/oot-0.5.0-rc9/oot.o2r", text)

    def test_playtest_line_follows_definition(self):
        d = definition()
        self.assertIn("Physical playtest of RC9: pending.", release.release_body(d))
        d["title"] = "Link-Span OoT 0.5.0 RC10"
        d["validation"]["physical_playtest"] = "passed"
        text = release.release_body(d)
        self.assertIn("Physical playtest of RC10: done.", text)
        self.assertNotIn("RC9", text)

    def test_public_page_is_english(self):
        text = release.release_body(definition()) + release.preview(definition())
        for portuguese in ("Instalação", "Verificação", "Fontes da entrega", "LEIA-ME", "CATALOGO", "Prévia", "pendente"):
            self.assertNotIn(portuguese, text)
        self.assertIn("## Install", text)

    def test_notes_replace_generic_install_and_are_pinned(self):
        notes = b"## What changed since RC9\r\n\r\n- New layout.\r\n"
        d = definition()
        d["notes"] = {"file": "NOTES.md", "sha256": hashlib.sha256(notes.replace(b"\r\n", b"\n")).hexdigest()}
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp) / "release.json"
            p.write_text(json.dumps(d), encoding="utf-8")
            (Path(temp) / "NOTES.md").write_bytes(notes)
            text = release.release_body(release.read_definition(p), release.read_notes(d, p))
            self.assertIn("## What changed since RC9\n\n- New layout.\n", text)
            self.assertNotIn("## Install\n\nGenerate", text)
            (Path(temp) / "NOTES.md").write_bytes(notes + b"edited\n")
            with self.assertRaisesRegex(release.ReleaseError, "Notas"):
                release.read_notes(d, p)
            d["notes"]["file"] = "../NOTES.md"
            p.write_text(json.dumps(d), encoding="utf-8")
            with self.assertRaises(release.ReleaseError):
                release.read_definition(p)

    def test_download_labels_preserve_unicode(self):
        text = release.release_body(definition())
        self.assertIn("OoT Overlay — Windows", text)
        self.assertIn("Mod Examples — ZIP", text)
        self.assertNotIn("â€", text)

    def test_output_rejects_extra_file(self):
        with tempfile.TemporaryDirectory() as temp:
            p = Path(temp)
            (p / "release-lock.json").write_text(json.dumps({"downloads": {r: {} for r in release.ROLES}}))
            (p / "SHA256SUMS.txt").write_text(release.file_digest(p / "release-lock.json") + "  release-lock.json\n")
            (p / "unexpected.zip").write_bytes(b"unexpected")
            with self.assertRaisesRegex(release.ReleaseError, "exatamente"):
                release.verify_output(p)


if __name__ == "__main__":
    unittest.main()
