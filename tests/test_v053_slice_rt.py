"""v0.53 bounded SLICE requests and preserved streaming/DAW contracts."""
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]

class SliceRealtimeV053(unittest.TestCase):
    def test_cadence_compiles_and_does_not_skip_boundary(self):
        compiler=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            exe=Path(d)/'slice-request'
            subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra',
                '-pedantic','-Werror',str(ROOT/'tests/slice_prefetch_v053_test.cpp'),
                '-o',str(exe)],check=True,timeout=60)
            p=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
            self.assertEqual(p.returncode,0,p.stderr)
            self.assertIn('SLICE prefetch cadence',p.stdout)

    def test_rt_render_keeps_disk_worker_separate(self):
        cpp=(ROOT/'Source/SampleEngine.cpp').read_text('utf8')
        h=(ROOT/'Source/SampleEngine.h').read_text('utf8')
        self.assertIn('slicePrefetch.reset()',cpp)
        self.assertIn('slicePrefetch.shouldRequest(',cpp)
        self.assertIn('if (prefetchNow) {',cpp)
        self.assertIn('s->requestStreamCritical(wanted);',cpp)
        self.assertIn('s->requestStreamAhead(wanted);',cpp)
        self.assertIn('liberty::slice_v053::PrefetchCadence slicePrefetch;',h)
        self.assertIn('planSliceWarmup(',cpp)
        self.assertIn('loopHead.getReadPointer',cpp)
        callback=cpp.split('void SampleVoice::renderStreamedNextBlock(',1)[1]
        for forbidden in ('reader.read(', 'new ', 'std::mutex', 'std::lock_guard',
                          'std::this_thread::sleep', 'data.setSize('):
            self.assertNotIn(forbidden,callback)

    def test_plugin_state_and_safe_cache_capacity_unchanged(self):
        w=(ROOT/'.github/workflows/build-rompler.yml').read_text('utf8')
        self.assertIn('python3 scripts/apply_v052_bundle.py',w)
        self.assertIn('python3 scripts/apply_v053_slice.py',w)
        self.assertIn('python3 tools/verify_slice_repeat_v053.py',w)
        policy=(ROOT/'Source/ResourcePolicyV051.h').read_text('utf8')
        self.assertIn('streamCachePages = 32',policy)
        loader=(ROOT/'scripts/apply_v053_slice.py').read_text('utf8')
        self.assertNotIn('PluginProcessor.cpp',loader)
        self.assertNotIn('PluginEditor',loader)
        self.assertNotIn('BankLibrary.cpp',loader)

    def test_qa_only_reads_benchmark_reports(self):
        script=(ROOT/'tools/verify_slice_repeat_v053.py').read_text('utf8')
        self.assertIn('thread_cpu_ms',script)
        self.assertIn('underflow_blocks',script)
        self.assertIn('callback_p99_ms',script)
