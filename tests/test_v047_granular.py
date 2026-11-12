"""v0.47: layer-local grain wet/dry and legacy automation integrity."""
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class GranularV047(unittest.TestCase):
    def test_blend_compiles_and_runs_without_juce(self):
        compiler=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            exe=Path(d)/'grain'
            subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra',
                            '-Werror','-pedantic',str(ROOT/'tests/granular_blend_v047_test.cpp'),
                            '-o',str(exe)],check=True,timeout=60)
            r=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
            self.assertEqual(r.returncode,0,r.stderr)
    def test_grain_is_per_layer_and_non_destructive(self):
        voice=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        pp=(ROOT/'Source/PluginProcessor.cpp').read_text('utf-8')
        ed=(ROOT/'Source/PluginEditor.cpp').read_text('utf-8')
        self.assertIn('layer.granularWet.load',voice)
        self.assertIn('granular_v047::blend',voice)
        self.assertIn('granular_v047::nextWet',voice)
        self.assertIn('getLayerGrainWet(int i)',(ROOT/'Source/PluginProcessor.h').read_text('utf-8'))
        for key in ('libertyLayerGrainWet','libertyLayerGrain','granularLayer','granularWet'):
            self.assertIn(key,pp)
        self.assertIn('granularLayerButtons[(size_t)i].setBounds',ed)
        self.assertIn('granularLayerMixes[(size_t)i].setBounds',ed)
        # Preserve all four v0.46 values and existing host automation!
        self.assertIn('juce::StringArray{"SAMPLES","ANALOG","WAVETABLE","FM 2 OP"}',pp)
    def test_legacy_granular_and_streaming_contract(self):
        s=(ROOT/'Source/SampleEngine.cpp').read_text('utf-8')
        streamed=s.split('void SampleVoice::renderStreamedNextBlock',1)[1]
        self.assertNotIn('layer.granularWet.load',streamed)
        self.assertIn('controls.granularActive.load',streamed)
        self.assertIn('if (s->isStreamed()) { renderStreamedNextBlock',s)
        pp=(ROOT/'Source/PluginProcessor.cpp').read_text('utf-8')
        self.assertIn('l->hasProperty("granularLayer")',pp)
        self.assertIn('setValue(0.f);',pp)
    def test_geometry_minimum_card_never_overlaps(self):
        lastrow=(73+3*20,73+3*20+19)
        toggle=(156,177)
        slider=(155,178)
        self.assertLessEqual(lastrow[1],toggle[0])
        self.assertLessEqual(lastrow[1],slider[0])
        self.assertLess(slider[1],180)
