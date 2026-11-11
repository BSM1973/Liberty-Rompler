"""Liberty ROMpler v0.35: read-only before/after timing in existing tooltip."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopSegmentTimesV035(unittest.TestCase):
    def test_cpp17_source_rate_and_bounds(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v035-") as directory:
            executable = Path(directory) / "wav-segment-times"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                "-pedantic",
                str(ROOT / "tests/loop_segment_times_v035_test.cpp"),
                "-o", str(executable)],
                check=True, cwd=ROOT, timeout=120)
            result = subprocess.run(
                [str(executable)], capture_output=True, text=True,
                check=False, cwd=ROOT, timeout=20)
            self.assertEqual(result.returncode, 0,
                             result.stdout + result.stderr)
            self.assertIn("v0.35 segment times: source rate and bounds OK",
                          result.stdout)

    def test_existing_tooltip_unchanged_and_new_timing_appended(self):
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            '#include "LoopSegmentTimesV035.h"',
            "liberty::loop_v035::SegmentTimes loopSegmentTimes() const noexcept",
            "liberty::loop_v035::describeSegmentTimes(",
            "loadedSampleRate);",
            "const auto segmentTimes = loopSegmentTimes();",
            "loopEnabled.getTooltip()",
            "juce::String(segmentTimes.beforeFrames)",
            "juce::String(segmentTimes.beforeMs, 3)",
            "juce::String(segmentTimes.afterFrames)",
            "juce::String(segmentTimes.afterMs, 3)",
            r"APR\u00c8S : ",
            "const auto segments = loopSegments();",
            "if (timing.valid && ratio.valid && coverage.valid && segments.valid)",
            "AVANT LA BOUCLE ",
            "PART DU WAV ",
            "juce::String(segments.beforePercentage, 2)",
            "juce::String(segments.afterPercentage, 2)",
            "snapStartButton.setTooltip(markerStartHelp",
            "snapEndButton.setTooltip(markerEndHelp",
            "liberty::loop_v030::describe(",
            "liberty::loop_v031::describePositions(",
            "liberty::loop_v032::describeFade(",
        ):
            with self.subTest(token=token):
                self.assertIn(token, code)
        new_method = code[code.index(
            "liberty::loop_v035::SegmentTimes loopSegmentTimes()"):
            code.index("liberty::loop_v034::Segments loopSegments()")]
        for forbidden in ("setValue(", "setSampleZoneEdit(", "applyEdits(",
                          "reader->read(", "processBlock(", "reconcile("):
            self.assertNotIn(forbidden, new_method)
        self.assertNotIn("juce::Label segmentTimesLabel", code)
        self.assertNotIn("juce::TextButton segmentTimesButton", code)

    def test_ascii_cplusplus_and_french(self):
        for path in ("Source/LoopSegmentTimesV035.h", "Source/SampleZoneView.h"):
            data = (ROOT / path).read_bytes()
            data.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", data)
        code = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertIn(r"APR\u00c8S LA BOUCLE", code)
        self.assertIn(r"APR\u00c8S : ", code)
        self.assertIn(r"Dur\u00e9e de boucle indisponible", code)
        self.assertEqual(code.count(
            "std::array<juce::TextButton, 4> frameNudges;"), 1)


if __name__ == "__main__":
    unittest.main()
