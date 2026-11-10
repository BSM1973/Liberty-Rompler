"""Liberty ROMpler v0.15: compile and execute isolated loop-editor DSP/UI math tests.

This is deliberately independent from JUCE and uses a temporary executable.
GitHub's existing 'python3 -m unittest discover' invocation executes this test
before the paid macOS AU/VST3/Standalone compilation.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LoopEditorV015Test(unittest.TestCase):
    def test_cpp_frame_preview_and_fade_geometry(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler, "A C++17 compiler is required")
        source = ROOT / "tests" / "loop_editor_v015_test.cpp"
        self.assertTrue(source.is_file(), "v0.15 C++ regression source missing")
        with tempfile.TemporaryDirectory(prefix="liberty-loop-v015-") as folder:
            executable = Path(folder) / "loop_editor_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c++17",
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    str(source),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=ROOT,
                timeout=120,
            )
            run = subprocess.run(
                [str(executable)],
                check=True,
                capture_output=True,
                text=True,
                timeout=20,
            )
            self.assertIn("Loop editor v0.15: C++ unit tests OK", run.stdout)


if __name__ == "__main__":
    unittest.main()
