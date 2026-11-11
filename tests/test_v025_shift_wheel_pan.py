"""Liberty ROMpler v0.25 Shift + wheel horizontal navigation tests."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class ShiftWheelNavigationV025(unittest.TestCase):
    def test_cpp17_wheel_pan_math(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v025-") as folder:
            binary = Path(folder) / "wheelpan"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_wheel_pan_v025_test.cpp"),
                "-o", str(binary)
            ], check=True, cwd=ROOT, timeout=120)
            process = subprocess.run(
                [str(binary)], check=False, capture_output=True,
                text=True, cwd=ROOT, timeout=20)
            self.assertEqual(process.returncode, 0,
                             process.stdout + process.stderr)
            self.assertIn("stable direction, steps and bounds OK", process.stdout)

    def test_ui_priority_and_previous_mouse_actions(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            '#include "LoopWheelPanV025.h"',
            "event.mods.isShiftDown() && !event.mods.isAltDown()",
            "wheel.deltaY != 0.0f || wheel.deltaX != 0.0f",
            "wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX",
            "liberty::loop_v025::panByWheel(",
            "viewStart = nextView.firstFrame;",
            "syncPositionControl();",
            "wheel.deltaX != 0.0f && viewSpan < sampleFrames",
            "liberty::loop_v023::zoomAtCursor(",
            "zoomSlider.setValue(next);",
            "liberty::loop_v024::panFromAnchor(",
            "if (dragging == 4 && pairedDrag.active())",
            "focusLoopViewport()",
        ):
            with self.subTest(token=token):
                self.assertIn(token, view)
        wheel = view[view.index("void mouseWheelMove("):
                     view.index("\nprivate:", view.index("void mouseWheelMove("))]
        self.assertLess(
            wheel.index("event.mods.isShiftDown() && !event.mods.isAltDown()"),
            wheel.index("if (wheel.deltaY != 0.0f) {"))
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "setSampleZoneEdit(", "applyEdits(", "reader->read(",
            "reconcile(", "autoSnap.getToggleState()",
        ):
            self.assertNotIn(forbidden, wheel)
        self.assertNotIn("juce::TextButton wheelPanButton", view)

    def test_french_accents_and_no_added_controls(self):
        for path in ("Source/LoopWheelPanV025.h", "Source/SampleZoneView.h"):
            raw = (ROOT / path).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"d\u00e9placer la vue sans toucher aux marqueurs", view)
        self.assertIn("Maj + molette : navigation horizontale rapide.", view)
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)


if __name__ == "__main__":
    unittest.main()
