"""Liberty ROMpler v0.19: paired loop marker duration-lock regression tests.

Tests operate on the real JUCE SampleZoneView.h that has been restored through
v0.14 source patches and overlaid with v0.15-v0.19. No DAW is touched.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class PairedLoopV019(unittest.TestCase):
    def test_cpp17_paired_seam_preserves_exact_duration_and_stereo_quality(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-paired-v019-") as tmp:
            binary = Path(tmp) / "paired"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_paired_seam_v019_test.cpp"),
                "-o", str(binary)
            ], check=True, timeout=120, cwd=ROOT)
            result = subprocess.run([str(binary)], check=True, timeout=30,
                                    text=True, capture_output=True)
            self.assertIn("stereo and duration-lock tests OK", result.stdout)

    def test_juce_uses_existing_two_zero_buttons_and_existing_apply_cancel(self):
        source = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for expected in (
            '#include "LoopPairedSeamV019.h"',
            "void optimizeLoopPair()",
            "modifiers.isAltDown() && modifiers.isShiftDown()",
            "optimizeLoopPair();",
            "findBestPairedShift(",
            "startCount, startFirst, true, true",
            "endCount, endFirst, true, true",
            "sliders[4].setValue(suggestion.startFrame, juce::dontSendNotification)",
            "sliders[5].setValue(suggestion.endFrame, juce::dontSendNotification)",
            "reconcile(4);",
            '"PAIRE : aucun gain"',
        ):
            with self.subTest(expected=expected):
                self.assertIn(expected, source)
        pair = source[source.index("void optimizeLoopPair()"):
                      source.index("void optimizeLoopMarker(bool startMarker)")]
        self.assertNotIn("setSampleZoneEdit(", pair)
        self.assertNotIn("applyEdits()", pair)
        self.assertNotIn("processBlock(", pair)
        self.assertNotIn("getWritePointer(", pair)
        # No extra control/layout item is allowed.
        self.assertEqual(source.count("juce::TextButton snapStartButton"), 1)
        self.assertNotIn("juce::TextButton pairShiftButton", source)
        self.assertIn("void applyEdits()", source)
        self.assertIn("cancelButton.onClick =", source)

    def test_changed_sources_obey_liberty_french_encoding_rule(self):
        for name in ("Source/LoopPairedSeamV019.h",
                     "Source/SampleZoneView.h"):
            content = (ROOT / name).read_bytes()
            content.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", content)


if __name__ == "__main__":
    unittest.main()
