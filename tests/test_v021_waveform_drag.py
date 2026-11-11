"""v0.21: shift-drag inside loop translates both waveform markers safely."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class WaveformDragV021(unittest.TestCase):
    def test_pure_cpp_gesture_and_bounds(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler, "C++17 compiler required")
        with tempfile.TemporaryDirectory(prefix="liberty-v021-") as temp:
            exe = Path(temp) / "drag"
            subprocess.run([
                compiler, "-std=c++17", "-Wall", "-Wextra",
                "-Werror", "-pedantic", "-O2",
                str(ROOT / "tests/loop_drag_v021_test.cpp"),
                "-o", str(exe),
            ], check=True, timeout=120, cwd=ROOT)
            result = subprocess.run(
                [str(exe)], capture_output=True, text=True,
                check=False, timeout=20, cwd=ROOT)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("stable anchor, clipping and duration OK", result.stdout)

    def test_existing_waveform_editing_preserved(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        expected = (
            '#include "LoopDragV021.h"',
            "liberty::loop_v021::PairedLoopDrag pairedDrag;",
            "e.mods.isShiftDown() && e.x >= startX && e.x < endX",
            "pairedDrag.begin({loopStart, loopEnd, sampleFrames}, anchored)",
            "dragging = 4;",
            "if (dragging == 4 && pairedDrag.active())",
            "const auto candidate = pairedDrag.move(pointer);",
            "sliders[4].setValue((double)candidate.loop.begin,",
            "sliders[5].setValue((double)candidate.loop.end,",
            "reconcile(4);",
            "pairedDrag.cancel();",
            "const int moved = dragging;",
            "moved == 2 || moved == 3",
            "void nudgeLoopPair(int delta)",
            "void optimizeLoopPair()",
            "void optimizeLoopMarker(bool startMarker)",
            "void snapLoopMarker(bool startMarker)",
        )
        for value in expected:
            with self.subTest(fragment=value):
                self.assertIn(value, view)
        drag = view[view.index("if (dragging == 4 && pairedDrag.active())"):
                    view.index("if (dragging >= 2)")]
        self.assertNotIn("snapLoopMarker(", drag)
        self.assertNotIn("processor.setSampleZoneEdit(", drag)
        self.assertNotIn("applyEdits(", drag)
        self.assertNotIn("reader->read(", drag)
        self.assertNotIn("new juce::", drag)
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::TextButton dragLoopButton", view)

    def test_liberty_orthography_and_no_replacement_glyphs(self):
        for file in ("Source/LoopDragV021.h", "Source/SampleZoneView.h"):
            b = (ROOT / file).read_bytes()
            b.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", b)
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"color\u00e9e", view)
        self.assertIn(r"d\u00e9placer", view)


if __name__ == "__main__":
    unittest.main()
