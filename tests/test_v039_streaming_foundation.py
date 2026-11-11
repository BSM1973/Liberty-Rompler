"""Liberty v0.39: streaming cache core, long offsets and legacy callback safety."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent


class StreamingV039(unittest.TestCase):
    def compile_and_run(self, filename, marker):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v039-") as folder:
            binary = Path(folder) / "test-cache"
            subprocess.run([compiler, "-std=c++17", "-O2", "-Wall", "-Wextra",
                            "-Werror", "-pedantic", "-pthread",
                            str(ROOT / "tests" / filename), "-o", str(binary)],
                           check=True, cwd=ROOT, timeout=120)
            result = subprocess.run([str(binary)], capture_output=True,
                                    text=True, cwd=ROOT, timeout=45)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn(marker, result.stdout)

    def test_page_cache_64_bit_and_stereo_races(self):
        self.compile_and_run("streaming_page_cache_v039_test.cpp", "nonblocking read")

    def test_read_ahead_worker_off_audio_thread(self):
        self.compile_and_run("streaming_worker_v039_test.cpp", "worker-only I/O")

    def test_bounded_mailbox_multi_producer(self):
        self.compile_and_run("streaming_mailbox_v039_test.cpp", "concurrent request safety")

    def test_legacy_audio_remains_unchanged(self):
        source = (ROOT / "Source/SampleEngine.cpp").read_text(encoding="utf-8")
        render = source.split("void SampleVoice::renderNextBlock(", 1)[1]
        self.assertNotIn("AudioFormatReader", render)
        self.assertNotIn("reader.read(", render)
        self.assertNotIn("StreamingReadAheadV039", render)
        self.assertNotIn("StreamingPageCacheV039", source)
        self.assertIn("reader.sampleRate * 20.0", source)

    def test_offline_contract_and_french_text(self):
        cache = (ROOT / "Source/StreamingPageCacheV039.h").read_text("utf-8")
        worker = (ROOT / "Source/StreamingReadAheadV039.h").read_text("utf-8")
        adapter = (ROOT / "Source/JuceStreamingReaderV039.h").read_text("utf-8")
        self.assertIn("std::make_unique<Slot[]>(PageSlots)", cache)
        self.assertIn("std::memory_order_seq_cst", cache)
        self.assertIn("std::this_thread::yield()", cache)
        self.assertIn("std::this_thread::sleep_for(", worker)
        self.assertIn("readFrames(first, count, left.data(), right.data())", worker)
        self.assertIn("reader->read(&buffer, 0,", adapter)
        for path in ("StreamingPageCacheV039.h", "StreamingReadAheadV039.h",
                     "JuceStreamingReaderV039.h"):
            data = (ROOT / "Source" / path).read_bytes()
            self.assertNotIn(b"\xef\xbf\xbd", data)


if __name__ == "__main__":
    unittest.main()
