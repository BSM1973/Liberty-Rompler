"""v0.41: 4 shared background workers, 64 zones; JUCE audio remains lock free."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class SharedPoolV041(unittest.TestCase):
    def test_shared_pool_compiles_and_runs(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v041-pool-") as tmp:
            binary = Path(tmp) / "sharedpool"
            subprocess.run([compiler,"-std=c++17","-O2","-Wall","-Wextra",
                            "-Werror","-pedantic","-pthread",
                            str(ROOT/"tests/streaming_pool_v041_test.cpp"),
                            "-o",str(binary)],cwd=ROOT,timeout=120,check=True)
            result=subprocess.run([str(binary)],capture_output=True,text=True,
                                  timeout=45,cwd=ROOT)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn("four-worker shared streaming pool 64 zones OK",result.stdout)

    def test_real_audio_uses_pool_not_per_zone_threads(self):
        zone=(ROOT/"Source/StreamedZoneV040.h").read_text("utf-8")
        cpp=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        for token in ("StreamingPoolV041.h","public liberty::stream_v041::ZoneService",
                      "registerZone(this)","unregisterZone(this)",
                      "serviceOneRequest() noexcept override",
                      "cache.tryReadFrame(frame, l, r)",
                      "cache.requestPage(page + lookahead)"):
            self.assertIn(token,zone)
        self.assertNotIn("ReadAheadWorker<",zone)
        self.assertNotIn("worker.start()",zone)
        self.assertIn("if (!streamed->start(attackFrames))",cpp)
        self.assertIn("renderStreamedNextBlock(out,start,count)",cpp)

    def test_cache_access_remains_nonblocking(self):
        zone=(ROOT/"Source/StreamedZoneV040.h").read_text("utf-8")
        rt=zone.split("// Audio callback operations:",1)[1].split(
            "// Called from the single assigned pool lane",1)[0]
        for forbidden in ("reader.readFrames(", "std::thread(", "join(",
                          "std::mutex", "std::lock_guard", "new ", "sleep_for"):
            self.assertNotIn(forbidden,rt)

    def test_bounded_pool_registration_and_lifetime(self):
        pool=(ROOT/"Source/StreamingPoolV041.h").read_text("utf-8")
        self.assertIn("lanesCount = 4",pool)
        self.assertIn("zonesPerLane = 16",pool)
        self.assertIn("if (best == lanesCount) return false",pool)
        self.assertIn("lane.zones[--lane.count] = nullptr;",pool)
        self.assertIn("lane.zones[slot]->serviceOneRequest()",pool)
        self.assertIn("const std::lock_guard<std::mutex> guard(lane.mutex)",pool)

    def test_old_presets_and_utf8_untouched(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        self.assertIn("reader.sampleRate * 20.0",engine)
        self.assertIn("reader.sampleRate * 0.75",engine)
        self.assertIn("std::numeric_limits<int>::max()-2",engine)
        self.assertIn("liberty::creative_v038::grainAt(",engine)
        for name in ("Source/StreamingPoolV041.h","Source/StreamedZoneV040.h",
                     "Source/SampleEngine.cpp"):
            data=(ROOT/name).read_bytes()
            self.assertNotIn(b"\xef\xbf\xbd",data)

if __name__ == "__main__":
    unittest.main()
