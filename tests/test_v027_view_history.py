"""Liberty ROMpler v0.27: reversible one-step waveform view history."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class WaveformViewHistoryV027(unittest.TestCase):
    def test_pure_cpp17_toggle_and_guards(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v027-") as folder:
            binary = Path(folder) / "view-history"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_view_history_v027_test.cpp"),
                "-o", str(binary),
            ], check=True, cwd=ROOT, timeout=120)
            process = subprocess.run([
                str(binary)], check=False, capture_output=True,
                text=True, timeout=20, cwd=ROOT)
            self.assertEqual(process.returncode, 0,
                             process.stdout + process.stderr)
            self.assertIn("undo, redo, WAV boundaries OK", process.stdout)

    def test_new_shortcut_and_previous_shortcuts(self):
        s = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for name in (
            '#include "LoopViewHistoryV027.h"',
            "liberty::loop_v027::ViewHistory viewHistory;",
            "void rememberViewport()",
            "bool restorePreviousViewport()",
            "viewHistory.toggle(currentViewport(), restored)",
            "zoomSlider.setValue(restored.zoom, juce::dontSendNotification)",
            "lastAppliedZoom = restored.zoom;",
            "event.mods.isShiftDown() && !event.mods.isAltDown()",
            "restorePreviousViewport();",
            "event.mods.isAltDown() && focusLoopViewport()",
            "event.mods.isCommandDown() && !event.mods.isAltDown()",
            "zoomSlider.setValue(1.0);",
            "liberty::loop_v026::centerOnPointer(",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "if (dragging == 4 && pairedDrag.active())",
        ):
            with self.subTest(token=name):
                self.assertIn(name, s)
        dbl = s[s.index("void mouseDoubleClick("):
                s.index("void mouseWheelMove(")]
        self.assertLess(dbl.index("restorePreviousViewport();"),
                        dbl.index("focusLoopViewport()"))
        self.assertLess(dbl.index("focusLoopViewport()"),
                        dbl.index("centerViewportOnClick("))
        self.assertLess(dbl.index("centerViewportOnClick("),
                        dbl.index("zoomSlider.setValue(1.0);"))

    def test_navigation_paths_record_history_before_change(self):
        s = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        body = s[s.index("bool restorePreviousViewport()"):
                 s.index("void syncPositionControl()")]
        self.assertNotIn("sliders[4].setValue(", body)
        self.assertNotIn("sliders[5].setValue(", body)
        self.assertNotIn("reconcile(", body)
        for token in (
            "if (panHistoryPending) {",
            "if (nextView.firstFrame != viewStart)",
            "if (viewport.spanFrames != viewSpan",
            "if (centered.firstFrame != viewStart)",
            "if (targetSpan != viewSpan || targetStart != viewStart)",
            "if (nextSpan != viewSpan || nextStart != viewStart)",
            "if (nextStart != viewStart)",
            "viewHistory.clear();",
        ):
            self.assertIn(token, s)
        # No extra controls; no source-WAV modifications.
        self.assertNotIn("juce::TextButton viewBackButton", s)
        self.assertEqual(s.count("std::array<juce::TextButton, 4> frameNudges;"), 1)

    def test_french_and_ascii_source(self):
        for name in ("Source/SampleZoneView.h", "Source/LoopViewHistoryV027.h"):
            data = (ROOT / name).read_bytes()
            data.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", data)
        s = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"cadrage pr\u00e9c\u00e9dent.", s)
        self.assertIn(r"zone color\u00e9e", s)


if __name__ == "__main__":
    unittest.main()
