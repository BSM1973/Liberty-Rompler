"""Liberty ROMpler v0.31: source-relative DÉBUT/FIN marker positions."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class MarkerTimesV031(unittest.TestCase):
    def test_cpp17_marker_times(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v031-") as folder:
            exe = Path(folder) / "marker-times"
            subprocess.run(
                [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                 "-Werror", "-pedantic",
                 str(ROOT / "tests/loop_marker_times_v031_test.cpp"),
                 "-o", str(exe)],
                check=True, cwd=ROOT, timeout=120)
            result = subprocess.run(
                [str(exe)], check=False, capture_output=True,
                text=True, cwd=ROOT, timeout=20)
            self.assertEqual(result.returncode, 0,
                             result.stdout + result.stderr)
            self.assertIn("absolute ms, exclusive end OK", result.stdout)

    def test_existing_zero_tooltips_keep_all_shortcuts(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for text in (
            '#include "LoopMarkerTimesV031.h"',
            "liberty::loop_v031::MarkerTimes markerTimes() const noexcept",
            "liberty::loop_v031::describePositions(",
            "loadedSampleRate = reader->sampleRate;",
            "const auto positions = markerTimes();",
            "snapStartButton.setTooltip(markerStartHelp",
            "snapEndButton.setTooltip(markerEndHelp",
            "positions.beginFrame",
            "positions.endFrame",
            "positions.beginMs",
            "positions.endMs",
            "juce::String(positions.beginMs, 3)",
            "juce::String(positions.endMs, 3)",
            "if (positions.valid) {",
            "snapStartButton.setTooltip(markerStartHelp);",
            "snapEndButton.setTooltip(markerEndHelp);",
            "Clic : z\\u00e9ro. Option : SAUT MIN.",
            "Maj + Option : PAIRE",
            "Commande + Option : ZOOM D\\u00c9TAIL.",
        ):
            with self.subTest(text=text):
                self.assertIn(text, view)
        helper = view[view.index("liberty::loop_v031::MarkerTimes markerTimes()"):
                      view.index("liberty::loop_v030::Timing loopTiming()")]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "sliders[6].setValue(", "setSampleZoneEdit(",
            "applyEdits(", "processBlock(", "reader->read(",
            "reconcile(", "zoomSlider.setValue(",
        ):
            self.assertNotIn(forbidden, helper)

    def test_previous_versions_and_layout_rules(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for text in (
            "liberty::loop_v030::describe(",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "liberty::loop_v027::ViewHistory viewHistory;",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "restorePreviousViewport();",
            "void nudgeLoopPair(int delta)",
            "snapLoopMarker(true);",
            "snapLoopMarker(false);",
        ):
            self.assertIn(text, view)
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::Label markerTimeLabel", view)
        self.assertNotIn("juce::TextButton markerTimeButton", view)

    def test_correct_french_accents_and_fallback(self):
        for path in ("Source/LoopMarkerTimesV031.h", "Source/SampleZoneView.h"):
            data = (ROOT / path).read_bytes()
            data.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", data)
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"D\u00c9BUT ", view)
        self.assertIn(r"dur\u00e9e fixe", view)
        self.assertIn(r"Dur\u00e9e de boucle indisponible", view)


if __name__ == "__main__":
    unittest.main()
