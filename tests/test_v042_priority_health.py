"""Liberty v0.42: critical-page priority, realtime isolation and health counters."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class StreamingPriorityV042(unittest.TestCase):
    def test_priority_pool_and_health_compile_cpp17(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v042-") as folder:
            executable = Path(folder) / "priority-test"
            subprocess.run([
                compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                "-pedantic", "-pthread",
                str(ROOT / "tests" / "streaming_priority_v042_test.cpp"),
                "-o", str(executable)
            ], cwd=ROOT, check=True, timeout=120)
            outcome = subprocess.run([str(executable)], cwd=ROOT, text=True,
                                     capture_output=True, timeout=25)
            self.assertEqual(outcome.returncode, 0, outcome.stdout+outcome.stderr)
            self.assertIn("critical first and health counters OK", outcome.stdout)

    def test_audio_requests_current_page_urgently(self):
        voice = (ROOT / "Source/SampleEngine.cpp").read_text("utf-8")
        sound = (ROOT / "Source/SampleEngine.h").read_text("utf-8")
        self.assertIn("requestStreamCritical(double pos)", sound)
        self.assertIn("s->requestStreamCritical(position)", voice)
        self.assertIn("s->requestStreamCritical(s->getLoopStart())", voice)
        self.assertIn("s->reportStreamUnderflowBlock();", voice)
        self.assertIn("liberty::stream_v042::HealthSnapshot streamingHealth()", sound)
        self.assertIn("bool underflowedThisBlock = false;", voice)

    def test_realtime_api_excludes_disk_and_locks(self):
        zone = (ROOT / "Source/StreamedZoneV040.h").read_text("utf-8")
        critical = zone.split("void requestCritical(",1)[1].split(
            "bool serviceUrgentRequest()",1)[0]
        self.assertIn("urgentPages", critical)
        self.assertIn("cache.isPageResident(page)", critical)
        self.assertIn("cache.requestPage(page)", critical)
        for forbidden in ("reader.readFrames(", "std::lock_guard", "std::mutex",
                          "std::this_thread::sleep_for(", "join(", "std::make_unique"):
            self.assertNotIn(forbidden, critical)
        cache = (ROOT/"Source/StreamingPageCacheV039.h").read_text("utf-8")
        self.assertIn("bool isPageResident(std::int64_t pageIndex) const noexcept", cache)

    def test_all_worker_lanes_consider_urgent_before_normal(self):
        pool=(ROOT/"Source/StreamingPoolV041.h").read_text("utf-8")
        self.assertIn("zone->serviceUrgentRequest()", pool)
        self.assertIn("zone->serviceOneRequest()", pool)
        self.assertIn("pass == 0", pool)
        self.assertIn("lanesCount = 4", pool)
        self.assertIn("zonesPerLane = 16", pool)

    def test_legacy_ram_path_and_french_ui_untouched(self):
        voice=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        self.assertIn("reader.sampleRate * 20.0", voice)
        self.assertIn("reader.sampleRate * 0.75", voice)
        self.assertIn("const auto& audio = s->getAudio();", voice)
        editor=(ROOT/"Source/PluginEditor.cpp").read_text("utf-8")
        self.assertIn("morphPad.setBounds", editor)
        self.assertIn("creativePanel.setBounds", editor)
        for p in ("Source/StreamingHealthV042.h", "Source/StreamedZoneV040.h",
                  "Source/StreamingPageCacheV039.h","Source/SampleEngine.cpp"):
            self.assertNotIn(b"\xef\xbf\xbd", (ROOT/p).read_bytes())

if __name__=="__main__":
    unittest.main()
