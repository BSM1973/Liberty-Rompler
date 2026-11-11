"""Liberty ROMpler v0.33 - loop coverage of source WAV and UI safety."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopCoverageV033(unittest.TestCase):
    def test_pure_cpp17_coverage_calculation(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v033-") as folder:
            executable = Path(folder) / "wav-coverage"
            subprocess.run(
                [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                 "-Werror", "-pedantic",
                 str(ROOT / "tests/loop_coverage_v033_test.cpp"),
                 "-o", str(executable)],
                check=True, cwd=ROOT, timeout=120)
            outcome = subprocess.run(
                [str(executable)], capture_output=True, text=True,
                check=False, cwd=ROOT, timeout=20)
            self.assertEqual(outcome.returncode, 0,
                             outcome.stdout + outcome.stderr)
            self.assertIn("translation and bounds OK", outcome.stdout)

    def test_view_is_readonly_and_reuses_tooltip(self):
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            '#include "LoopCoverageV033.h"',
            "liberty::loop_v033::Coverage loopCoverage() const noexcept",
            "liberty::loop_v033::describeCoverage(",
            "const auto timing = loopTiming();",
            "const auto ratio = fadeRatio();",
            "const auto coverage = loopCoverage();",
            "if (timing.valid && ratio.valid && coverage.valid && segments.valid)",
            "PART DU WAV ",
            "juce::String(coverage.percentage, 2)",
            ' + " %");',
            "snapStartButton.setTooltip(markerStartHelp",
            "snapEndButton.setTooltip(markerEndHelp",
            "liberty::loop_v032::describeFade(",
            "liberty::loop_v031::describePositions(",
            "liberty::loop_v030::describe(",
        ):
            with self.subTest(token=token):
                self.assertIn(token, code)
        new_method = code[
            code.index("liberty::loop_v033::Coverage loopCoverage()"):
            code.index("liberty::loop_v032::FadeRatio fadeRatio()")
        ]
        for forbidden in (
            "setValue(", "setSampleZoneEdit(", "applyEdits(",
            "reconcile(", "reader->read(", "processBlock(",
        ):
            self.assertNotIn(forbidden, new_method)
        self.assertNotIn("juce::Label loopCoverageLabel", code)
        self.assertNotIn("juce::TextButton loopCoverageButton", code)

    def test_previous_gestures_and_french_accents(self):
        for path in ("Source/LoopCoverageV033.h", "Source/SampleZoneView.h"):
            raw = (ROOT / path).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            r"Dur\u00e9e de boucle indisponible",
            r"NON APPLIQU\u00c9",
            r"Commande + Option : ZOOM D\u00c9TAIL.",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "liberty::loop_v027::ViewHistory viewHistory;",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "void nudgeLoopPair(int delta)",
        ):
            self.assertIn(token, code)
        self.assertEqual(code.count("std::array<juce::TextButton, 4> frameNudges;"), 1)


if __name__ == "__main__":
    unittest.main()
