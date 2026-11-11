"""Liberty ROMpler v0.24 - Option+drag waveform viewport regression."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class WaveformPanV024(unittest.TestCase):
    def test_pure_cpp17_pan_helper(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v024-") as folder:
            binary = Path(folder) / "pan"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_pan_v024_test.cpp"),
                "-o", str(binary),
            ], check=True, cwd=ROOT, timeout=120)
            outcome = subprocess.run(
                [str(binary)], check=False, capture_output=True,
                text=True, timeout=20, cwd=ROOT)
            self.assertEqual(outcome.returncode, 0, outcome.stdout + outcome.stderr)
            self.assertIn("bounded viewport and no marker edits OK", outcome.stdout)

    def test_modifier_priority_and_no_loop_mutation(self):
        s = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for required in (
            '#include "LoopPanV024.h"',
            "int panFirstAtDown = 0, panAnchorX = 0;",
            "e.mods.isAltDown() && !e.mods.isShiftDown()",
            "panFirstAtDown = viewStart;",
            "panAnchorX = e.x;",
            "dragging = 5;",
            "if (dragging == 5) {",
            "liberty::loop_v024::panFromAnchor(",
            "viewStart = panned.firstFrame;",
            "syncPositionControl();",
            "pairedDrag.begin({loopStart, loopEnd, sampleFrames}, anchored)",
            "if (dragging == 4 && pairedDrag.active())",
            "if (showLoops && (moved == 2 || moved == 3) && autoSnap.getToggleState())",
            "if (event.mods.isAltDown()",
        ):
            with self.subTest(fragment=required):
                self.assertIn(required, s)

        down = s[s.index("void mouseDown("):s.index("void mouseDrag(")]
        self.assertLess(down.index("e.mods.isAltDown() && !e.mods.isShiftDown()"),
                        down.index("const int loopStart ="))
        drag = s[s.index("if (dragging == 5) {"):
                 s.index("if (dragging == 4 && pairedDrag.active())")]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "applyEdits(", "setSampleZoneEdit(", "reader->read(",
            "status =", "autoSnap", "reconcile(",
        ):
            with self.subTest(forbidden=forbidden):
                self.assertNotIn(forbidden, drag)
        self.assertIn("if (!loopEnabled.getToggleState()) return;", down)
        self.assertNotIn("juce::TextButton panButton", s)

    def test_unicode_and_visual_constraints(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_bytes()
        helper = (ROOT / "Source/LoopPanV024.h").read_bytes()
        editor.decode("ascii")
        helper.decode("ascii")
        self.assertNotIn(b"\xef\xbf\xbd", editor)
        self.assertIn(rb"d\u00e9placer la vue sans toucher aux marqueurs", editor)
        view = editor.decode("ascii")
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertIn("focusLoopViewport()", view)
        self.assertIn("void nudgeLoopPair(int delta)", view)


if __name__ == "__main__":
    unittest.main()
