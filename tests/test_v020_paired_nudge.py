"""Regression tests for Liberty ROMpler v0.20 manual paired loop movement."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class ManualPairedNudgeV020(unittest.TestCase):
    def test_paired_nudge_cpp17(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v020-") as tmp:
            binary = Path(tmp) / "nudge"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_paired_nudge_v020_test.cpp"),
                "-o", str(binary)
            ], cwd=ROOT, check=True, timeout=120)
            proc = subprocess.run([str(binary)], cwd=ROOT,
                                  text=True, capture_output=True,
                                  timeout=20)
            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
            self.assertIn("all duration and bounds tests OK", proc.stdout)

    def test_existing_four_buttons_handle_shift_and_alt(self):
        s = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for fragment in (
            '#include "LoopPairedNudgeV020.h"',
            "const int distance = modifiers.isAltDown() ? 10 : 1;",
            "if (modifiers.isShiftDown())",
            "nudgeLoopPair(delta);",
            "nudgeLoopMarker(i < 2, delta);",
            "void nudgeLoopPair(int delta)",
            "liberty::loop_v020::nudgePair(",
            "sliders[4].setValue((double)result.loop.begin,",
            "sliders[5].setValue((double)result.loop.end,",
            "reconcile(4);",
            "GLISSE : limite atteinte",
        ):
            with self.subTest(fragment=fragment):
                self.assertIn(fragment, s)
        body = s[s.index("void nudgeLoopPair(int delta)"):
                 s.index("void nudgeLoopMarker(bool startMarker, int delta)")]
        self.assertNotIn("applyEdits()", body)
        self.assertNotIn("processor.setSampleZoneEdit(", body)
        self.assertNotIn("snapLoopMarker(", body)
        self.assertNotIn("processBlock(", body)
        self.assertEqual(s.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::TextButton pairedNudgeButton;", s)
        self.assertIn("cancelButton.onClick =", s)

    def test_cplusplus_text_is_ascii_with_no_replacement_glyphs(self):
        for path in ("Source/LoopPairedNudgeV020.h", "Source/SampleZoneView.h"):
            content = (ROOT / path).read_bytes()
            content.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", content)


if __name__ == "__main__":
    unittest.main()
