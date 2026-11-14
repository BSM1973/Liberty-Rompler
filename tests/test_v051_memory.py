"""v0.51 memory and callback safeguards without modifying bank content."""
from pathlib import Path
import subprocess
import tempfile
import shutil
import unittest
ROOT = Path(__file__).resolve().parents[1]

class V051MemoryTest(unittest.TestCase):
    def test_pure_cpp_memory_policy(self):
        c = shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(c)
        with tempfile.TemporaryDirectory() as folder:
            exe = Path(folder)/'resource-policy'
            subprocess.run([c, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                            '-pedantic', str(ROOT/'tests/resource_policy_v051_test.cpp'),
                            '-o',str(exe)],check=True,timeout=60)
            r = subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertIn('unchanged-layer reuse policy',r.stdout)

    def test_cache_preserved_for_slice_reliability(self):
        zone=(ROOT/'Source/StreamedZoneV040.h').read_text('utf-8')
        policy=(ROOT/'Source/ResourcePolicyV051.h').read_text('utf-8')
        self.assertIn('resource_v051::streamCachePages',zone)
        self.assertIn('streamCachePages = 32',policy)
        self.assertIn('PageCache<pageFrames, cachePages, 96>',zone)
        self.assertIn('static constexpr std::size_t pageFrames = 4096',zone)
        self.assertIn('requestCritical',zone)
        self.assertIn('serviceUrgentRequest',zone)
        self.assertIn('std::make_unique<Slot[]>(PageSlots)',
                      (ROOT/'Source/StreamingPageCacheV039.h').read_text('utf-8'))

    def test_changed_layer_only_rebuilds_and_edited_zones_force(self):
        c=(ROOT/'Source/PluginProcessor.cpp').read_text('utf-8')
        h=(ROOT/'Source/PluginProcessor.h').read_text('utf-8')
        self.assertIn('resource_v051::canReuseLayer',c)
        self.assertIn('pending.add(synths[(size_t)i].getSound(zoneIndex).get())',c)
        self.assertIn('reusedLayer[(size_t)i] = true;',c)
        self.assertIn('if (!reusedLayer[(size_t)i])',c)
        self.assertIn('reloadLayerSounds(layerSources, true)',c)
        self.assertIn('reloadLayerSounds(desired, true)',c)
        self.assertIn('toString(), true)',c)
        self.assertIn('bool forceReload = false',h)
        self.assertIn('synths[(size_t)i].clearSounds();',c)

    def test_dsp_sound_and_slice_are_untouched(self):
        s=(ROOT/'scripts/apply_v051_bundle.py').read_text('utf-8')
        self.assertNotIn("edit('SampleEngine.cpp'",s)
        self.assertNotIn("edit('SampleEngine.h'",s)
        self.assertNotIn('PluginEditor.cpp',s)
        self.assertNotIn('BankLibrary.cpp',s)
        self.assertIn('planSliceWarmup(',
                      (ROOT/'Source/SampleEngine.cpp').read_text('utf-8'))

    def test_bench_instruments_cpu_spikes_and_switch_memory(self):
        s=(ROOT/'tests/rompler_audio_stress_v044.cpp').read_text('utf-8')
        self.assertIn('callback_spikes',s)
        self.assertIn('switch_rss_trace',s)
        self.assertIn('underflow_timeline',s)
        self.assertIn('callback_over_budget',s)
