"""Liberty ROMpler v0.18: raw seam search regression and source contracts."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class SmartSeamV018(unittest.TestCase):
    def test_stereo_raw_step_search_cpp17(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler, "A C++17 compiler is required.")
        with tempfile.TemporaryDirectory(prefix="liberty-v018-") as folder:
            executable = Path(folder) / "seam-test"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                "-Werror", "-pedantic",
                str(ROOT / "tests/loop_seam_search_v018_test.cpp"),
                "-o", str(executable),
            ], cwd=ROOT, check=True, timeout=120)
            proc = subprocess.run([str(executable)], cwd=ROOT,
                                  capture_output=True, text=True,
                                  check=True, timeout=30)
            self.assertIn("conservative mono/stereo tests OK", proc.stdout)

    def test_zero_buttons_reuse_existing_geometry(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        for required in (
            '#include "LoopSeamSearchV018.h"',
            "void optimizeLoopMarker(bool startMarker)",
            "modifiers.isAltDown()",
            "optimizeLoopMarker(true);",
            "optimizeLoopMarker(false);",
            "liberty::loop_v018::findLowestRawStep(",
            "sliders[(size_t)(startMarker ? 4 : 5)].setValue(best.marker)",
            "SAUT MIN : aucun meilleur point",
            "Compar er A/B".replace(" ", ""),
        ):
            if required == "ComparerA/B":
                # Tooltip may have a space; the contract is a visible reminder.
                self.assertIn("Comparer A/B avant d'appliquer.", editor)
            else:
                with self.subTest(phrase=required):
                    self.assertIn(required, editor)
        # The existing component count does not change.
        self.assertEqual(editor.count("addAndMakeVisible(snapStartButton);"), 0)
        self.assertNotIn("juce::TextButton smartSnapButton;", editor)
        self.assertNotIn("setSampleZoneEdit(", editor[
            editor.index("void optimizeLoopMarker("):
            editor.index("void snapLoopMarker(")
        ])
        self.assertNotIn("processor.", editor[
            editor.index("void optimizeLoopMarker("):
            editor.index("void snapLoopMarker(")
        ])

    def test_only_bounded_wav_reads_and_no_audio_thread_changes(self):
        editor = (ROOT / "Source/SampleZoneView.h").read_text(encoding="ascii")
        search = editor[editor.index("void optimizeLoopMarker("):
                        editor.index("void snapLoopMarker(")]
        self.assertIn("constexpr int radius = 512", search)
        self.assertIn("juce::AudioBuffer<float> movingBuffer(channels, count)", search)
        self.assertIn("juce::AudioBuffer<float> referenceBuffer(channels, 2)", search)
        self.assertIn("referenceFirst, true, true", search)
        self.assertNotIn("processBlock(", search)
        self.assertNotIn("applyEdits()", search)
        helper = (ROOT / "Source/LoopSeamSearchV018.h").read_bytes()
        helper.decode("ascii")
        self.assertIn(b"raw", helper)
        self.assertNotIn(b"\xef\xbf\xbd", helper)


if __name__ == "__main__":
    unittest.main()
