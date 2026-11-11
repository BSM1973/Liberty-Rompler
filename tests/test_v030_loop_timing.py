"""Liberty ROMpler v0.30 - non-destructive real WAV timing display."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopTimingV030(unittest.TestCase):
    def test_pure_cpp17_duration_calculator(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v030-") as folder:
            executable = Path(folder) / "loop-timing"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_timing_v030_test.cpp"),
                "-o", str(executable),
            ], cwd=ROOT, timeout=120, check=True)
            result = subprocess.run([str(executable)], cwd=ROOT, timeout=20,
                                    capture_output=True, text=True, check=False)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("WAV sample rate, frames, ms and fade OK", result.stdout)

    def test_precise_wav_sample_rate_is_used(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for fragment in (
            '#include "LoopTimingV030.h"',
            "double loadedSampleRate = 0.0;",
            "loadedSampleRate = 0.0;",
            "loadedSampleRate = reader->sampleRate;",
            "liberty::loop_v030::describe(",
            "timing.durationFrames",
            "timing.durationMs",
            "timing.fadeFrames",
            "timing.fadeMs",
            "BOUCLE ",
            " | FONDU ",
            "loopEnabled.setTooltip(",
            "feedback.setTooltip(message);",
        ):
            with self.subTest(fragment=fragment):
                self.assertIn(fragment, view)
        self.assertIn("if (!showLoops || !currentRow() || !loopEnabled.getToggleState())",
                      view)
        self.assertIn("const auto timing = loopTiming();", view)
        self.assertIn("waveArea.translated(0, waveArea.getHeight() + 2).withHeight(14)",
                      view)
        self.assertNotIn("juce::Label loopTimingLabel", view)
        self.assertNotIn("juce::TextButton loopTimeButton", view)

    def test_no_audio_or_loop_editing_from_timing(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        timing = view[view.index("liberty::loop_v030::Timing loopTiming()"):
                      view.index("liberty::loop_v027::View currentViewport()")]
        for forbidden in (
            "sliders[4].setValue(", "sliders[5].setValue(",
            "sliders[6].setValue(", "applyEdits(", "setSampleZoneEdit(",
            "processBlock(", "reader->read(", "reconcile(",
        ):
            self.assertNotIn(forbidden, timing)
        self.assertEqual(view.count("std::array<juce::TextButton, 4> frameNudges;"), 1)

    def test_ascii_french_and_existing_gestures(self):
        for name in ("Source/LoopTimingV030.h", "Source/SampleZoneView.h"):
            data = (ROOT / name).read_bytes()
            data.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd", data)
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for token in (
            r"Dur\u00e9e de boucle indisponible",
            r"Glisser les marqueurs ou r\u00e9gler le fondu",
            "liberty::loop_v029::planPrecisionZoom(",
            "liberty::loop_v028::focusOnMarker(",
            "liberty::loop_v027::ViewHistory viewHistory;",
            "liberty::loop_v025::panByWheel(",
            "liberty::loop_v024::panFromAnchor(",
            "liberty::loop_v023::zoomAtCursor(",
            "void nudgeLoopPair(int delta)",
        ):
            with self.subTest(token=token):
                self.assertIn(token, view)


if __name__ == "__main__":
    unittest.main()
