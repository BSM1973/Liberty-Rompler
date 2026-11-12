"""v0.43 real-source and generator checks (requires reconstructed JUCE overlay)."""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]

class DiagnosticsV043(unittest.TestCase):
    def test_aggregation_cpp17(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder) / "aggregation"
            subprocess.run([compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                            "-Werror", "-pedantic",
                            str(ROOT / "tests/streaming_diagnostics_v043_test.cpp"),
                            "-o", str(exe)], check=True, timeout=90)
            test = subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)
            self.assertEqual(test.returncode, 0, test.stderr)
            self.assertIn("aggregated diagnostics OK", test.stdout)

    def test_live_panel_connected_without_new_tab_or_audio_callback_modifications(self):
        processor = (ROOT / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
        sample = (ROOT / "Source/SampleEngine.h").read_text(encoding="utf-8")
        self.assertIn("const juce::ScopedLock guard(lock)", sample)
        self.assertIn("mapped->streamingHealth()", sample)
        editor = (ROOT / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
        header = (ROOT / "Source/PluginEditor.h").read_text(encoding="utf-8")
        panel = (ROOT / "Source/LibertyStreamingDiagnosticsV043.h").read_text(encoding="utf-8")
        self.assertIn("getStreamingDiagnostics()", processor)
        self.assertIn("synth.appendStreamingDiagnostics(result)", processor)
        self.assertIn("streamingDiagnostics.setVisible(next==Page::Settings)", editor)
        self.assertIn("streamingDiagnostics.setBounds", editor)
        self.assertIn("LibertyStreamingDiagnosticsV043 streamingDiagnostics;", header)
        self.assertIn("startTimerHz(4)", panel)
        self.assertIn("SystemClipboard::copyTextToClipboard", panel)
        self.assertEqual(header.count("enum class Page {"), 1)
        self.assertIn("Creative, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings", header)
        self.assertNotIn("getStreamingDiagnostics()", processor.split(
            "void LibertyRomplerAudioProcessor::processBlock(", 1)[-1].split("\n}", 1)[0])

    def test_bank_generator_uses_current_v1_format_and_bounded_files(self):
        from importlib.util import spec_from_file_location, module_from_spec
        spec = spec_from_file_location("stress", ROOT / "tools/generate_streaming_stress_v043.py")
        module = module_from_spec(spec)
        spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as folder:
            location = Path(folder) / "Test-Bank"
            manifest = module.generate(location, seconds=0.12, rate=8000, zones_per_layer=2)
            contents = json.loads(manifest.read_text(encoding="utf-8"))
            self.assertEqual(contents["format"], "liberty-rompler-bank-v1")
            self.assertEqual(len(contents["presets"]), 5)
            self.assertEqual(len(contents["presets"][4]["layers"]), 4)
            self.assertEqual(len(list((location / "Samples").glob("*.wav"))), 8)
            with wave.open(str(location / "Samples/layer1_n48.wav"), "rb") as wav:
                self.assertEqual(wav.getnchannels(), 2)
                self.assertEqual(wav.getframerate(), 8000)
                self.assertEqual(wav.getnframes(), 960)

if __name__ == "__main__":
    unittest.main()
