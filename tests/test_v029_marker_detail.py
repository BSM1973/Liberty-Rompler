"""Liberty ROMpler v0.29 - Command+Option click for loop marker detail."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class PrecisionLoopDetailV029(unittest.TestCase):
    def test_pure_cpp17_detail_planner(self):
        cc = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory(prefix="liberty-v029-") as temp:
            exe = Path(temp) / "marker-detail"
            subprocess.run(
                [cc, "-std=c++17", "-O2", "-Wall", "-Wextra",
                 "-Werror", "-pedantic",
                 str(ROOT / "tests/loop_marker_detail_v029_test.cpp"),
                 "-o", str(exe)],
                cwd=ROOT, timeout=120, check=True)
            result = subprocess.run(
                [str(exe)], cwd=ROOT, timeout=20,
                capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0,
                             result.stdout + result.stderr)
            self.assertIn("512-frame target, zoom cap, no zoom-out OK", result.stdout)

    def test_existing_zero_commands_and_modifier_priority(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for expected in (
            '#include "LoopMarkerDetailV029.h"',
            "snapStartButton.onClick = [this] {",
            "snapEndButton.onClick = [this] {",
            "if (modifiers.isAltDown() && modifiers.isShiftDown())",
            "else if (modifiers.isAltDown()) {",
            "if (modifiers.isCommandDown())",
            "focusLoopMarkerDetail(true);",
            "focusLoopMarkerDetail(false);",
            "optimizeLoopMarker(true);",
            "optimizeLoopMarker(false);",
            "focusLoopMarker(true);",
            "focusLoopMarker(false);",
            "snapLoopMarker(true);",
            "snapLoopMarker(false);",
            "bool focusLoopMarkerDetail(bool startMarker)",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "zoomSlider.setValue(detail.requestedZoom, juce::dontSendNotification)",
            "lastAppliedZoom = detail.requestedZoom;",
            "rememberViewport();",
            "syncPositionControl();",
            "restorePreviousViewport();",
        ):
            with self.subTest(expected=expected):
                self.assertIn(expected, view)

        for name, end in (
            ("snapStartButton.onClick = [this]", "snapEndButton.onClick = [this]"),
            ("snapEndButton.onClick = [this]", "cancelButton.onClick = [this]"),
        ):
            code = view[view.index(name):view.index(end)]
            self.assertLess(code.index("modifiers.isAltDown() && modifiers.isShiftDown()"),
                            code.index("else if (modifiers.isAltDown()) {"))
            self.assertLess(code.index("focusLoopMarkerDetail("),
                            code.index("optimizeLoopMarker("))
            self.assertLess(code.index("else if (modifiers.isAltDown()) {"),
                            code.index("else if (modifiers.isCommandDown() && !modifiers.isShiftDown())"))
        detail = view[view.index("bool focusLoopMarkerDetail(bool startMarker)"):
                      view.index("bool focusLoopMarker(bool startMarker)")]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "setSampleZoneEdit(", "applyEdits(", "reader->read(",
            "reconcile(", "autoSnap.getToggleState()", "processBlock(",
        ):
            with self.subTest(forbidden=forbidden):
                self.assertNotIn(forbidden, detail)
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::TextButton precisionZoomButton", view)

    def test_french_accents_and_legacy_gestures(self):
        for filename in ("Source/LoopMarkerDetailV029.h", "Source/SampleZoneView.h"):
            raw = (ROOT / filename).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        source = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for fragment in (
            r"Commande + Option : ZOOM D\u00c9TAIL.",
            r"Commande + clic : centrer D\u00c9BUT.",
            "liberty::loop_v027::ViewHistory viewHistory;",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "if (dragging == 4 && pairedDrag.active())",
            "bool centerViewportOnClick(int localX)",
            "bool focusLoopViewport()",
        ):
            self.assertIn(fragment, source)


if __name__ == "__main__":
    unittest.main()
