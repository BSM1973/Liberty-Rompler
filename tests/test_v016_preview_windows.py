"""Liberty ROMpler v0.16 - portable offline preview tests.

Run with:
python3 -m unittest discover -s tests -p 'test_*.py' -v

No GitHub macOS CI runs are started by this test.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class WindowedPreviewV016(unittest.TestCase):
    def test_cpp_preview_matches_v015_without_loading_whole_wav(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler, "C++17 compiler required")
        source = ROOT / "tests/loop_preview_windows_v016_test.cpp"
        with tempfile.TemporaryDirectory(prefix="liberty-preview-v016-") as folder:
            binary = Path(folder) / "test"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic", str(source), "-o", str(binary)
            ], cwd=ROOT, check=True, timeout=120)
            run = subprocess.run([str(binary)], check=True,
                                 capture_output=True, text=True, timeout=20)
            self.assertIn("bounded-window equivalence tests OK", run.stdout)

    def test_juce_ab_preview_and_metric_contract(self):
        view = (ROOT / "overlays/v015/SampleZoneView.h").read_text(encoding="ascii")
        processor = (ROOT / "overlays/v015/PluginProcessor.h").read_text(encoding="ascii")
        patch = (ROOT / "scripts/apply_v016_preview.sh").read_text(encoding="utf-8")
        for expected in (
            "previewSelectedLoop(bool withoutFade, bool extendedContext)",
            "keys.isShiftDown()",
            "withoutFade ? 0 : (int)sliders[6].getValue(), &rawSeamStepDb",
            "juce::String(rawSeamStepDb, 1)",
            'feedback.setTooltip(message)',
        ):
            self.assertIn(expected, view)
        self.assertIn("float* rawSeamStepDb = nullptr", processor)
        for expected in (
            'LoopSeamDifferenceV016.h',
            "measureRawSeamDifference(tail, head)",
            "if (rawSeamStepDb != nullptr)",
            "rawSeamStepDb = seamDifference.dbAmplitude",
            "juce::AudioBuffer<float> tailBuffer(channels, contextFrames)",
            "juce::AudioBuffer<float> headBuffer(channels, contextFrames)",
        ):
            self.assertIn(expected, patch)
        self.assertFalse(any(ord(char) > 127 for char in view))

    def test_only_two_wav_windows_remain_in_patch(self):
        patch = (ROOT / "scripts/apply_v016_preview.sh").read_text(encoding="utf-8")
        self.assertIn("tailBuffer(channels, contextFrames)", patch)
        self.assertIn("headBuffer(channels, contextFrames)", patch)
        self.assertIn("renderSeamFromWindows(", patch)
        self.assertIn("endFrame - contextFrames", patch)
        self.assertNotIn("previewVoice.process(", patch)
        self.assertIn("s.count(start_anchor) == 1", patch)


if __name__ == "__main__":
    unittest.main()
