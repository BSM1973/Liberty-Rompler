"""Liberty ROMpler v0.23 cursor-anchored Option-wheel zoom regression."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class CursorZoomV023(unittest.TestCase):
    def test_cpp17_helper_and_bounds(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v023-") as folder:
            exe = Path(folder) / "cursor-zoom"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_cursor_zoom_v023_test.cpp"),
                "-o", str(exe),
            ], check=True, timeout=120, cwd=ROOT)
            result = subprocess.run([str(exe)], check=False, timeout=20,
                                    capture_output=True, text=True, cwd=ROOT)
            self.assertEqual(result.returncode, 0,
                             result.stdout + result.stderr)
            self.assertIn("stable anchor and clipped bounds OK", result.stdout)

    def test_option_wheel_reuses_existing_zoom_controls(self):
        source = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        required = (
            '#include "LoopCursorZoomV023.h"',
            "event.mods.isAltDown()",
            "liberty::loop_v023::zoomAtCursor(",
            "waveArea.getWidth(), event.x - waveArea.getX()",
            "zoomSlider.setValue(next, juce::dontSendNotification)",
            "viewSpan = viewport.spanFrames;",
            "viewStart = viewport.firstFrame;",
            "syncPositionControl();",
            "zoomSlider.setValue(next);",
            "focusLoopViewport()",
            "if (dragging == 4 && pairedDrag.active())",
            "void nudgeLoopPair(int delta)",
        )
        for fragment in required:
            with self.subTest(fragment=fragment):
                self.assertIn(fragment, source)
        start = source.index("void mouseWheelMove(")
        end = source.index("\nprivate:", start)
        wheel = source[start:end]
        assert wheel.index("event.mods.isAltDown()") < wheel.index("zoomSlider.setValue(next);")
        self.assertIn("wheel.deltaX != 0.0f && viewSpan < sampleFrames", wheel)
        for forbidden in ("sliders[4].setValue(", "sliders[5].setValue(",
                          "setSampleZoneEdit(", "applyEdits(", "reader->read("):
            self.assertNotIn(forbidden, wheel)
        self.assertNotIn("juce::TextButton cursorZoomButton", source)

    def test_ascii_french_accents_and_no_replacement_glyphs(self):
        for file in ("Source/LoopCursorZoomV023.h", "Source/SampleZoneView.h"):
            content = (ROOT / file).read_bytes()
            content.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", content)
        editor = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"vis\u00e9", editor)


if __name__ == "__main__":
    unittest.main()
