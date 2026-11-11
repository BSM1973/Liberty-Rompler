"""Liberty ROMpler v0.26: Command-double-click recenters WAV viewport."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class CenterViewV026(unittest.TestCase):
    def test_cpp17_center_view_math_and_edges(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v026-") as folder:
            binary = Path(folder) / "center"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_center_view_v026_test.cpp"),
                "-o", str(binary),
            ], check=True, cwd=ROOT, timeout=120)
            outcome = subprocess.run([
                str(binary)
            ], capture_output=True, text=True, check=False, cwd=ROOT, timeout=20)
            self.assertEqual(outcome.returncode, 0,
                             outcome.stdout + outcome.stderr)
            self.assertIn("pointer frame, zoom lock and edges OK", outcome.stdout)

    def test_modifiers_and_previous_doubleclick_commands(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for expected in (
            '#include "LoopCenterViewV026.h"',
            "void mouseDoubleClick(const juce::MouseEvent& event) override",
            "event.mods.isAltDown() && focusLoopViewport()",
            "event.mods.isCommandDown() && !event.mods.isAltDown()",
            "centerViewportOnClick(event.x - waveArea.getX())",
            "zoomSlider.setValue(1.0);",
            "bool centerViewportOnClick(int localX)",
            "liberty::loop_v026::centerOnPointer(",
            "viewStart = centered.firstFrame;",
            "syncPositionControl();",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "focusLoopViewport()",
            "void nudgeLoopPair(int delta)",
        ):
            with self.subTest(expected=expected):
                self.assertIn(expected, view)
        doubleclick = view[view.index("void mouseDoubleClick("):
                           view.index("void mouseWheelMove(")]
        self.assertLess(doubleclick.index("event.mods.isAltDown()"),
                        doubleclick.index("event.mods.isCommandDown()"))
        self.assertLess(doubleclick.index("event.mods.isCommandDown()"),
                        doubleclick.index("zoomSlider.setValue(1.0)"))
        center = view[view.index("bool centerViewportOnClick(int localX)"):
                      view.index("bool focusLoopViewport()")]
        for forbidden in ("zoomSlider.setValue(", "sliders[4].setValue(",
                          "sliders[5].setValue(", "setSampleZoneEdit(",
                          "reconcile(", "applyEdits(", "reader->read(",
                          "processBlock(", "autoSnap.getToggleState()"):
            self.assertNotIn(forbidden, center)
        self.assertNotIn("juce::TextButton centerViewButton", view)

    def test_french_escaping_and_compact_layout(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_bytes()
        helper = (ROOT / "Source/LoopCenterViewV026.h").read_bytes()
        editor.decode("ascii")
        helper.decode("ascii")
        self.assertNotIn(b"\xef\xbf\xbd", editor)
        self.assertIn(b"Commande + double-clic : centrer la vue.", editor)
        self.assertIn(rb"zone color\u00e9e", editor)
        self.assertEqual(editor.decode("ascii").count(
            "std::array<juce::TextButton, 4> frameNudges;"), 1)


if __name__ == "__main__":
    unittest.main()
