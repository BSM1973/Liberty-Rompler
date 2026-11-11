"""Liberty ROMpler v0.34 - read-only WAV segment percentages and UI safety."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopSegmentsV034(unittest.TestCase):
    def test_cpp17_segments_and_invalid_wav_bounds(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v034-") as folder:
            executable = Path(folder) / "wav-segments"
            subprocess.run(
                [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                 "-Werror", "-pedantic",
                 str(ROOT / "tests/loop_segments_v034_test.cpp"),
                 "-o", str(executable)],
                check=True, cwd=ROOT, timeout=120)
            outcome = subprocess.run(
                [str(executable)], capture_output=True, text=True,
                check=False, cwd=ROOT, timeout=20)
            self.assertEqual(outcome.returncode, 0,
                             outcome.stdout + outcome.stderr)
            self.assertIn("v0.34 WAV segments, bounds and translation OK",
                          outcome.stdout)

    def test_existing_tooltip_and_legacy_controls_preserved(self):
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            '#include "LoopSegmentsV034.h"',
            "liberty::loop_v034::Segments loopSegments() const noexcept",
            "liberty::loop_v034::describeSegments(",
            "const auto segments = loopSegments();",
            "timing.valid && ratio.valid && coverage.valid && segments.valid",
            "PART DU WAV ",
            "juce::String(coverage.percentage, 2)",
            "AVANT LA BOUCLE ",
            "juce::String(segments.beforePercentage, 2)",
            r"APR\u00c8S LA BOUCLE ",
            "juce::String(segments.afterPercentage, 2)",
            "snapStartButton.setTooltip(markerStartHelp",
            "snapEndButton.setTooltip(markerEndHelp",
            "liberty::loop_v032::describeFade(",
            "liberty::loop_v031::describePositions(",
            "liberty::loop_v030::describe(",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "void nudgeLoopPair(int delta)",
        ):
            with self.subTest(token=token):
                self.assertIn(token, code)
        new_method = code[
            code.index("liberty::loop_v034::Segments loopSegments()"):
            code.index("liberty::loop_v033::Coverage loopCoverage()")
        ]
        for forbidden in ("setValue(", "setSampleZoneEdit(", "applyEdits(",
                          "reader->read(", "processBlock("):
            self.assertNotIn(forbidden, new_method)
        self.assertNotIn("juce::Label loopSegmentsLabel", code)
        self.assertNotIn("juce::TextButton loopSegmentsButton", code)
        self.assertEqual(code.count("std::array<juce::TextButton, 4> frameNudges;"), 1)

    def test_ascii_cpp_with_escaped_french_accents(self):
        for path in ("Source/LoopSegmentsV034.h", "Source/SampleZoneView.h"):
            raw = (ROOT / path).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"APR\u00c8S LA BOUCLE", code)
        self.assertIn(r"Dur\u00e9e de boucle indisponible", code)


if __name__ == "__main__":
    unittest.main()
