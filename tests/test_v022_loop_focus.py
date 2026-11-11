"""v0.22: focus waveform viewport on selected loop without touching audio."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopViewportFocusV022(unittest.TestCase):
    def test_focus_calculator_cpp17(self):
        cc = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory(prefix="liberty-v022-") as temp:
            exe = Path(temp) / "focus"
            subprocess.run([
                cc, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_focus_v022_test.cpp"),
                "-o", str(exe),
            ], cwd=ROOT, check=True, timeout=120)
            outcome = subprocess.run([str(exe)], cwd=ROOT, capture_output=True,
                                     text=True, timeout=20, check=False)
            self.assertEqual(outcome.returncode, 0,
                             outcome.stdout + outcome.stderr)
            self.assertIn("frame bounds, loop padding, zoom caps OK", outcome.stdout)

    def test_option_double_click_and_plain_fit_all(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for expected in (
            '#include "LoopFocusV022.h"',
            "void mouseDoubleClick(const juce::MouseEvent& event) override",
            "event.mods.isAltDown() && focusLoopViewport()",
            "zoomSlider.setValue(1.0);",
            "bool focusLoopViewport()",
            "liberty::loop_v022::planFocus(",
            "focus.centerFrame, sampleFrames, viewSpan",
            "zoomSlider.setValue(focus.requestedZoom, juce::dontSendNotification)",
            "syncPositionControl();",
            "void nudgeLoopPair(int delta)",
            "void optimizeLoopPair()",
            "if (dragging == 4 && pairedDrag.active())",
        ):
            with self.subTest(fragment=expected):
                self.assertIn(expected, editor)

        body = editor[editor.index("bool focusLoopViewport()"):
                      editor.index("void updateZoom()", editor.index("bool focusLoopViewport()"))]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "setSampleZoneEdit(", "applyEdits(", "reader->read(",
            "processBlock(", "new juce::", "autoSnap.getToggleState()",
        ):
            with self.subTest(no=forbidden):
                self.assertNotIn(forbidden, body)
        self.assertEqual(editor.count("addAndMakeVisible(fitWaveButton);"), 0)
        self.assertNotIn("juce::TextButton focusLoopButton", editor)

    def test_ascii_cpp_maintains_french_unicode_escapes(self):
        source = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        (ROOT / "Source/LoopFocusV022.h").read_bytes().decode("ascii")
        self.assertIn(r"Option + double-clic : cadrer la boucle", source)
        self.assertNotIn("\ufffd", source)


if __name__ == "__main__":
    unittest.main()
