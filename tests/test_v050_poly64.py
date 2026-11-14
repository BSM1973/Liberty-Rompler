"""v0.50: bounded poly64 attack / loop-head PCM with no RT allocations."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]

class Poly64V050(unittest.TestCase):
    def test_bounded_loop_window_cpp(self):
        compiler=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            exe=Path(d)/'loopwarm'
            subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra',
                            '-Werror','-pedantic',str(ROOT/'tests/loop_transition_v050_test.cpp'),
                            '-o',str(exe)],check=True,timeout=120)
            p=subprocess.run([str(exe)],text=True,capture_output=True,timeout=20)
            self.assertEqual(p.returncode,0,p.stderr)
            self.assertIn('bounded poly64',p.stdout)

    def test_slices_and_legacy_small_wavs_are_preserved(self):
        cpp=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        h=(ROOT/'Source/SampleEngine.h').read_text('utf-8')
        for needle in ('if (longSample && leader)', 'planSliceWarmup(',
                       'n-first < warm.getNumSamples()',
                       'getSliceWarmupEnd(', 'if (longSample) {',
                       'reader.sampleRate * 20.0',
                       'reader.read(&loopHead,0,loop.frames,loop.begin,true,true)',
                       'loopHeadStart', 'getLoopWarmupEnd()',
                       'reader.sampleRate * 1.50'):
            self.assertIn(needle,cpp+h)
        self.assertIn('std::array<juce::AudioBuffer<float>, 8> sliceWarmups',h)
        self.assertIn('loopHead.getReadPointer',cpp)
        self.assertIn('if (s->getLoopWarmupEnd() <= s->getLoopStart())',cpp)

    def test_dsp_callback_stays_without_disk_and_allocations(self):
        cpp=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        callback=cpp.split('void SampleVoice::renderStreamedNextBlock',1)[1]
        for forbidden in ('reader.read(', 'loopHead.setSize', 'new ', 'std::mutex',
                          'std::lock_guard', 'sleep_for('):
            self.assertNotIn(forbidden,callback)
        script=(ROOT/'scripts/apply_v050_poly64.py').read_text('utf-8')
        for forbidden in ('BankLibrary.cpp','PluginEditor.cpp','PluginEditor.h'):
            self.assertNotIn(forbidden,script)

    def test_stress_json_retains_scenario_and_tracks_incident_times(self):
        cpp=(ROOT/'tests/rompler_audio_stress_v044.cpp').read_text('utf-8')
        self.assertIn('underflow_timeline',cpp)
        self.assertIn('processor.getStreamingDiagnostics()',cpp)
        self.assertIn('underflow_blocks',cpp)
        workflow=(ROOT/'.github/workflows/build-rompler.yml').read_text('utf-8')
        self.assertIn('python3 scripts/apply_v050_poly64.py',workflow)
        self.assertIn('python3 scripts/verify_release_v050.py',workflow)
