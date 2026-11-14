from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class StreamingV049(unittest.TestCase):
    def test_slice_warmup_plan_cpp(self):
        cc=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory() as folder:
            exe=Path(folder)/'slice-warmup'
            subprocess.run([cc,'-std=c++17','-O2','-Wall','-Wextra','-Werror','-pedantic',
                            str(ROOT/'tests/slice_warmup_v049_test.cpp'),'-o',str(exe)],
                           check=True,timeout=40)
            result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertIn('bounded SLICE warmups',result.stdout)

    def test_only_leaders_warm_distant_heads_outside_audio(self):
        c=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        h=(ROOT/'Source/SampleEngine.h').read_text('utf-8')
        self.assertIn('if (longSample && leader)',c)
        self.assertIn('planSliceWarmup',c)
        self.assertIn('reader.sampleRate*3.25',c)
        self.assertIn('warmedEnd > position',c)
        self.assertIn('getSliceWarmupEnd',h)
        self.assertIn('reader.read(&buffer,0,warm.frames,warm.begin,true,true)',c)
        self.assertIn('n-first < warm.getNumSamples()',c)
        self.assertIn('readStereoAt(position,left,right,sliceIndex)',c)
        self.assertIn('std::array<juce::AudioBuffer<float>, 8> sliceWarmups',h)
        callback=c.split('void SampleVoice::renderStreamedNextBlock',1)[1]
        for forbidden in ('reader.read(', 'buffer.setSize(', 'std::mutex', 'std::lock_guard'):
            self.assertNotIn(forbidden,callback)

    def test_cache_hint_is_pinned_and_lock_free(self):
        c=(ROOT/'Source/StreamingPageCacheV039.h').read_text('utf-8')
        self.assertIn('hitSlotHint',c)
        self.assertIn('if (readSlot(preferred)) return true',c)
        self.assertIn('slot.readers.fetch_add',c)
        self.assertIn('slot.state.load(std::memory_order_seq_cst) == 2',c)
        self.assertIn('slot.readers.fetch_sub',c)
        self.assertNotIn('std::mutex',c)

    def test_unchanged_presets_and_ui_contract(self):
        script=(ROOT/'scripts/apply_v049_streaming.py').read_text('utf-8')
        self.assertNotIn('BankLibrary.cpp',script)
        self.assertNotIn('PluginEditor',script)
        wf=(ROOT/'.github/workflows/build-rompler.yml').read_text('utf-8')
        self.assertIn('python3 scripts/apply_v049_streaming.py',wf)
        self.assertTrue('scripts/verify_release_v049.py' in wf or
                        'scripts/verify_release_v050.py' in wf)
