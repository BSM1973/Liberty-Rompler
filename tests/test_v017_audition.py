"""Liberty ROMpler v0.17: offline precise sample-loop audition tests."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class AuditionV017(unittest.TestCase):
    def test_context_is_bounded_at_all_sample_rates(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v017-") as folder:
            exe = Path(folder) / "audition"
            subprocess.run([
                compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                "-pedantic", "-O2",
                str(ROOT / "tests/loop_audition_v017_test.cpp"),
                "-o", str(exe),
            ], check=True, timeout=120, cwd=ROOT)
            output = subprocess.run(
                [str(exe)], check=True, capture_output=True,
                text=True, timeout=20).stdout
            self.assertIn("v0.17 audition context plans: tests OK", output)

    def test_existing_controls_handle_option_and_shift(self):
        view = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        header = (ROOT / "Source/PluginProcessor.h").read_text(encoding="ascii")
        required = (
            "isShiftDown()", "isAltDown()",
            "previewSelectedLoop(bool withoutFade, bool extendedContext)",
            "extendedContext ? 250 : 60",
            "Option + clic = 10 frames",
            "nudgeLoopMarker(i < 2, delta)",
            "extendedContext ? \" 250 ms\" : \" 60 ms\"",
        )
        for part in required:
            with self.subTest(fragment=part):
                self.assertIn(part, view)
        self.assertIn("int contextMilliseconds = 60", header)
        self.assertEqual(view.count("addAndMakeVisible(previewButton);"), 1)
        self.assertNotIn("\ufffd", view)

    def test_bounded_preview_and_audio_callback_order(self):
        processor = (ROOT / "Source/PluginProcessor.cpp").read_text(encoding="ascii")
        self.assertIn('"LoopAuditionPlanV017.h"', processor)
        self.assertIn("liberty::loop_v017::planAudition(", processor)
        self.assertIn("const int contextFrames = plan.contextFrames;", processor)
        self.assertIn("renderSeamFromWindows(", processor)
        self.assertNotIn("AudioBuffer<float> decoded(channels, total)", processor)
        start = processor.index("void LibertyRomplerAudioProcessor::processBlock(")
        end = processor.index("juce::Result LibertyRomplerAudioProcessor::previewSampleZoneLoop(", start)
        block = processor[start:end]
        self.assertLess(block.index("audio.applyGain(level);"),
                        block.index("previewVoice.process("))
        self.assertIn("juce::ScopedTryLock previewTryLock(previewLock)", block)
        self.assertNotIn("reader->read(", block)
        self.assertNotIn("std::vector<", block)

    def test_ascii_sources_preserve_unicode_escapes(self):
        for relative in ("Source/LoopAuditionPlanV017.h",
                         "Source/PluginProcessor.h",
                         "Source/PluginProcessor.cpp",
                         "Source/SampleZoneView.h"):
            with self.subTest(path=relative):
                (ROOT / relative).read_bytes().decode("ascii")


if __name__ == "__main__":
    unittest.main()
