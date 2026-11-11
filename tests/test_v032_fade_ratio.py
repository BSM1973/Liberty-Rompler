"""v0.32: fade percentage in an existing tooltip, with v0.31 bundled."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class FadeRatioV032(unittest.TestCase):
    def test_standalone_cpp17_ratio(self):
        cc = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory(prefix="liberty-v032-") as folder:
            binary = Path(folder) / "ratio"
            subprocess.run(
                [cc, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                 "-pedantic", str(ROOT / "tests/loop_fade_ratio_v032_test.cpp"),
                 "-o", str(binary)],
                check=True, cwd=ROOT, timeout=120)
            result = subprocess.run(
                [str(binary)], capture_output=True, text=True,
                cwd=ROOT, timeout=20, check=False)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("exclusive end and bounds OK", result.stdout)

    def test_bundled_v031_marker_positions_and_v030_timing(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            '#include "LoopMarkerTimesV031.h"',
            '#include "LoopFadeRatioV032.h"',
            "liberty::loop_v032::FadeRatio fadeRatio() const noexcept",
            "liberty::loop_v032::describeFade(",
            "liberty::loop_v031::MarkerTimes markerTimes() const noexcept",
            "liberty::loop_v031::describePositions(",
            "liberty::loop_v030::describe(",
            "loadedSampleRate = reader->sampleRate;",
            "const auto timing = loopTiming();",
            "const auto ratio = fadeRatio();",
            "if (timing.valid && ratio.valid)",
            "juce::String(ratio.percentage, 2)",
            ' + " %");',
            "snapStartButton.setTooltip(markerStartHelp",
            "snapEndButton.setTooltip(markerEndHelp",
        ):
            with self.subTest(token=token):
                self.assertIn(token, view)
        ratio = view[view.index("liberty::loop_v032::FadeRatio fadeRatio()"):
                     view.index("liberty::loop_v031::MarkerTimes markerTimes()")]
        for mutation in ("setValue(", "setSampleZoneEdit(", "reconcile(",
                         "applyEdits(", "reader->read(", "processBlock("):
            self.assertNotIn(mutation, ratio)

    def test_no_extra_control_or_broken_french(self):
        for path in ("Source/LoopFadeRatioV032.h", "Source/SampleZoneView.h"):
            raw = (ROOT / path).read_bytes()
            raw.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", raw)
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)
        self.assertNotIn("juce::Label fadePercentageLabel", view)
        self.assertNotIn("juce::TextButton fadePercentageButton", view)
        for fragment in (
            r"NON APPLIQU\u00c9", r"Dur\u00e9e de boucle indisponible",
            r"Commande + Option : ZOOM D\u00c9TAIL.",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "liberty::loop_v027::ViewHistory viewHistory;",
        ):
            self.assertIn(fragment, view)


if __name__ == "__main__":
    unittest.main()
