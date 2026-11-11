"""v0.37 professional multisampling: real manifest import and engine regressions."""
import importlib.util
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "smart_import_v037", ROOT / "tools" / "smart_import_v037.py")
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)


def make_wav(path, frames=2205, rate=44100):
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(rate)
        wav.writeframes(b"\0\0" * frames)


class MultisampleV037(unittest.TestCase):
    def test_policy_cpp17_is_compilable(self):
        cc = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory(prefix="liberty-ms-v037-") as tmp:
            binary = Path(tmp) / "round-robin"
            subprocess.run(
                [cc, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                 "-pedantic", str(ROOT / "tests" / "multisample_v037_test.cpp"),
                 "-o", str(binary)],
                check=True, cwd=ROOT, timeout=90)
            result = subprocess.run([str(binary)], text=True, capture_output=True,
                                    cwd=ROOT, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("articulation and velocity transitions OK", result.stdout)

    def test_import_round_robin_velocity_and_articulations(self):
        with tempfile.TemporaryDirectory(prefix="liberty-import-") as tmp:
            folder = Path(tmp) / "MyBank"
            folder.mkdir()
            for band in ("001-063", "064-127"):
                for take in ("01", "02"):
                    make_wav(folder / (
                        f"Piano_C4_V{band}_RR{take}_ARTSUSTAIN_KS24.wav"))
            manifest = importer.create_manifest(folder, "mybank", "My Bank")
            self.assertEqual(manifest["format"], "liberty-rompler-bank-v1")
            self.assertEqual(len(manifest["presets"]), 1)
            zones = manifest["presets"][0]["zones"]
            self.assertEqual(len(zones), 4)
            self.assertEqual({z["root_note"] for z in zones}, {60})
            self.assertEqual({z["rr_index"] for z in zones}, {1, 2})
            self.assertEqual({z["rr_group"] for z in zones}, {1})
            self.assertEqual({z["articulation"] for z in zones}, {1})
            self.assertEqual({z["keyswitch_note"] for z in zones}, {24})
            self.assertEqual({z["velocity_fade"] for z in zones}, {8})
            self.assertEqual({(z["low_velocity"], z["high_velocity"])
                              for z in zones}, {(1, 63), (64, 127)})

            command = [sys.executable, str(ROOT / "tools" / "smart_import_v037.py"),
                       "--bank-folder", str(folder), "--bank-id", "mybank",
                       "--bank-name", "My Bank"]
            dry = subprocess.run(command, capture_output=True, text=True, cwd=ROOT)
            self.assertEqual(dry.returncode, 0, dry.stderr)
            self.assertIn("DRY RUN", dry.stdout)
            self.assertFalse((folder / "manifest.json").exists())
            first = subprocess.run(command + ["--write"],
                                   capture_output=True, text=True, cwd=ROOT)
            self.assertEqual(first.returncode, 0, first.stderr)
            self.assertEqual(json.loads((folder / "manifest.json").read_text(
                encoding="utf-8")), manifest)
            second = subprocess.run(command + ["--write"],
                                    capture_output=True, text=True, cwd=ROOT)
            self.assertEqual(second.returncode, 2)
            self.assertIn("--replace", second.stderr)

            from tools.validate_banks import check_root
            errors, count, presets = check_root(Path(tmp))
            self.assertEqual(errors, [])
            self.assertEqual((count, presets), (1, 1))

    def test_note_ranges_and_no_velocity_invention(self):
        with tempfile.TemporaryDirectory(prefix="liberty-range-") as tmp:
            root = Path(tmp)
            make_wav(root / "Strings_C3.wav")
            make_wav(root / "Strings_C4.wav")
            m = importer.create_manifest(root, "strings", "Strings")
            zones = m["presets"][0]["zones"]
            self.assertEqual([(z["root_note"], z["low_note"], z["high_note"])
                              for z in zones], [(48, 0, 54), (60, 55, 127)])
            self.assertTrue(all("rr_group" not in z for z in zones))
            self.assertTrue(all("velocity_fade" not in z for z in zones))

    def test_ambiguous_root_is_rejected_without_changes(self):
        with tempfile.TemporaryDirectory(prefix="liberty-unsafe-") as tmp:
            root = Path(tmp)
            make_wav(root / "Unknown.wav")
            with self.assertRaisesRegex(importer.ImportErrorWithContext,
                                        "ambiguous/missing root note"):
                importer.create_manifest(root, "bank", "Bank")
            self.assertFalse((root / "manifest.json").exists())

    def test_invalid_new_bank_metadata_rejected(self):
        with tempfile.TemporaryDirectory(prefix="liberty-invalid-") as tmp:
            root = Path(tmp)
            bank = root / "Test"
            bank.mkdir()
            make_wav(bank / "Piano_C4.wav")
            manifest = importer.create_manifest(bank, "test", "Test")
            zone = manifest["presets"][0]["zones"][0]
            zone["rr_group"] = 1
            zone["rr_index"] = 0
            (bank / "manifest.json").write_text(json.dumps(manifest),
                                                encoding="utf-8")
            from tools.validate_banks import check_root
            errors, _, _ = check_root(root)
            self.assertTrue(any("round robin" in s for s in errors), errors)

    def test_native_integration_preserves_legacy_samples_and_state(self):
        engine = (ROOT / "Source" / "SampleEngine.cpp").read_text("utf-8")
        h = (ROOT / "Source" / "SampleEngine.h").read_text("utf-8")
        bank = (ROOT / "Source" / "BankLibrary.cpp").read_text("utf-8")
        host = (ROOT / "Source" / "PluginProcessor.cpp").read_text("utf-8")
        for word in ("round_robin", "getRoundRobin", "rrGroup",
                     "getKeyswitchNote", "getActiveArticulation",
                     "velocityCrossfadeGain"):
            self.assertIn(word, engine + h + bank + host)
        self.assertIn("robin.select(", engine)
        self.assertIn("velocityCrossfadeGain(velocity)", engine)
        self.assertIn("midiVelocity >= controls.lowVelocity", engine)
        self.assertIn("loopCrossfadeFrames(z.loopCrossfadeFrames)", engine)
        self.assertIn("for (auto& synth : synths) synth.resetPerformanceSelectors();", host)
        self.assertIn('object->setProperty("layerState"', host)
        self.assertIn('object->setProperty("sampleZoneEdits"', host)
        self.assertIn('object->setProperty("morphActive"', host)
        self.assertNotIn("reader.read(", engine[engine.index(
            "void MappedSynthesiser::noteOn"):])

    def test_no_broken_french_or_binary_mutation(self):
        for file in ["Source/MultiSamplePolicyV037.h",
                     "Source/SampleEngine.cpp", "Source/SampleEngine.h",
                     "Source/BankLibrary.cpp"]:
            contents = (ROOT / file).read_bytes()
            self.assertNotIn(b"\xef\xbf\xbd", contents)
        self.assertIn("safe", (ROOT / "tools/smart_import_v037.py").read_text())


if __name__ == "__main__":
    unittest.main()
