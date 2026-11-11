"""Liberty ROMpler v0.28 - Command-click existing ZERO buttons to focus."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopMarkerFocusV028(unittest.TestCase):
    def test_pure_cpp17_marker_focus(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v028-") as folder:
            executable = Path(folder) / "marker-focus"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_marker_focus_v028_test.cpp"),
                "-o", str(executable),
            ], check=True, cwd=ROOT, timeout=120)
            result = subprocess.run([str(executable)], check=False,
                                    capture_output=True, text=True,
                                    cwd=ROOT, timeout=20)
            self.assertEqual(result.returncode, 0,
                             result.stdout + result.stderr)
            self.assertIn("precise center, zoom lock, bounds OK", result.stdout)

    def test_existing_zero_buttons_keep_modifier_priority(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        required = (
            '#include "LoopMarkerFocusV028.h"',
            "snapStartButton.onClick = [this] {",
            "snapEndButton.onClick = [this] {",
            "if (modifiers.isAltDown() && modifiers.isShiftDown())",
            "optimizeLoopPair();",
            "else if (modifiers.isAltDown())",
            "optimizeLoopMarker(true);",
            "optimizeLoopMarker(false);",
            "else if (modifiers.isCommandDown() && !modifiers.isShiftDown())",
            "focusLoopMarker(true);",
            "focusLoopMarker(false);",
            "snapLoopMarker(true);",
            "snapLoopMarker(false);",
            "bool focusLoopMarker(bool startMarker)",
            "liberty::loop_v028::focusOnMarker(",
            "const int marker = (int)sliders[startMarker ? 4 : 5].getValue();",
            "viewStart = target.firstFrame;",
            "rememberViewport();",
            "syncPositionControl();",
        )
        for fragment in required:
            with self.subTest(fragment=fragment):
                self.assertIn(fragment, editor)
        for start_button, next_button in (
            ("snapStartButton.onClick = [this]", "snapEndButton.onClick = [this]"),
            ("snapEndButton.onClick = [this]", "cancelButton.onClick = [this]"),
        ):
            section = editor[editor.index(start_button):editor.index(next_button)]
            self.assertLess(section.index("modifiers.isAltDown() && modifiers.isShiftDown()"),
                            section.index("modifiers.isCommandDown()"))
            self.assertLess(section.index("modifiers.isAltDown())"),
                            section.index("modifiers.isCommandDown()"))
        focus = editor[editor.index("bool focusLoopMarker(bool startMarker)"):
                       editor.index("bool centerViewportOnClick(int localX)")]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "zoomSlider.setValue(", "setSampleZoneEdit(",
            "applyEdits(", "reconcile(", "reader->read(",
            "autoSnap.getToggleState()", "processBlock(",
        ):
            self.assertNotIn(forbidden, focus)
        self.assertEqual(editor.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::TextButton focusMarkerButton", editor)

    def test_accents_and_all_existing_navigation(self):
        for name in ("Source/SampleZoneView.h", "Source/LoopMarkerFocusV028.h"):
            raw = (ROOT / name).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        source = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for value in (
            r"Commande + clic : centrer D\u00c9BUT.",
            "Commande + clic : centrer FIN.",
            "void nudgeLoopPair(int delta)",
            "liberty::loop_v027::ViewHistory viewHistory;",
            "restorePreviousViewport();",
            "centerViewportOnClick(event.x - waveArea.getX())",
            "focusLoopViewport()",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
        ):
            self.assertIn(value, source)


if __name__ == "__main__":
    unittest.main()
