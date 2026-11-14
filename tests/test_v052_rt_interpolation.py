"""v0.52: one-reader-pin adjacent sample path and CPU/wall callback QA."""
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class RealtimeInterpolationV052(unittest.TestCase):
    def test_pair_cache_publication_cpp(self):
        c=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(c)
        with tempfile.TemporaryDirectory() as tmp:
            exe=Path(tmp)/'pair'
            subprocess.run([c,'-std=c++17','-O2','-pthread','-Wall','-Wextra',
                            '-Werror','-pedantic',str(ROOT/'tests/streaming_pair_v052_test.cpp'),
                            '-o',str(exe)],check=True,timeout=40)
            res=subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)
            self.assertEqual(res.returncode,0,res.stderr)
            self.assertIn('interpolation PASS',res.stdout)
    def test_cache_hard_safety_guards(self):
        c=(ROOT/'Source/StreamingPageCacheV039.h').read_text('utf-8')
        body=c.split('bool tryReadFramePair(',1)[1].split('bool isPageResident(',1)[0]
        self.assertIn('slot.readers.fetch_add',body)
        self.assertIn('slot.readers.fetch_sub',body)
        self.assertIn('offset + 1 < slot.valid',body)
        self.assertIn('slot.state.load(std::memory_order_seq_cst) == 2',body)
        self.assertNotIn('std::mutex',body)
        self.assertNotIn('new ',body)
        self.assertNotIn('reader.read(',body)
    def test_source_boundary_fallback_no_changes_to_slice(self):
        cpp=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        self.assertIn('tryReadFramePair(frame,l0,r0,l1,r1)',cpp)
        self.assertIn('if (interpolateBuffer(sliceWarmups[idx],sliceWarmupStarts[idx]))',cpp)
        self.assertIn('if (loopHeadStart >= 0 && interpolateBuffer(loopHead,loopHeadStart))',cpp)
        self.assertIn('if (frame+1 < attackFrames && interpolateBuffer(data,0))',cpp)
        self.assertIn('auto get = [&](std::int64_t n',cpp)
        self.assertIn('planSliceWarmup(',cpp)
    def test_qa_distinguishes_thread_cpu_from_wall(self):
        stress=(ROOT/'tests/rompler_audio_stress_v044.cpp').read_text('utf-8')
        for s in ('CLOCK_THREAD_CPUTIME_ID','thread_cpu_ms',
                  'callback_spikes','callback_p99_ms','underflow_timeline'):
            self.assertIn(s,stress)
        script=(ROOT/'scripts/apply_v052_bundle.py').read_text('utf-8')
        self.assertNotIn('BankLibrary.cpp',script)
        self.assertNotIn('PluginEditor.cpp',script)
        self.assertNotIn('PluginProcessor.cpp',script)
