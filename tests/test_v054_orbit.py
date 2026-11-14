"""Liberty ORBIT v0.54 - tested DSP math, host automation and UI."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class OrbitV054(unittest.TestCase):
    def test_pure_rt_audio_math(self):
        cc=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(cc)
        with tempfile.TemporaryDirectory() as d:
            exe=Path(d)/'orbit'
            subprocess.run([cc,'-std=c++17','-O2','-Wall','-Wextra','-Werror',
                '-pedantic',str(ROOT/'tests/orbit_engine_v054_test.cpp'),
                '-o',str(exe)],check=True,timeout=60)
            r=subprocess.run([str(exe)],text=True,capture_output=True,timeout=10)
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertIn('MIDI gravity PASS',r.stdout)
    def test_backward_compatible_plugin_integration(self):
        p=(ROOT/'Source/PluginProcessor.cpp').read_text('utf8')
        h=(ROOT/'Source/PluginProcessor.h').read_text('utf8')
        for needle in ('libertyMorphActive','libertyMorphX','libertyMorphY',
                       'libertyMorphMidi','libertyOrbitMode','libertyOrbitRate',
                       'libertyOrbitDepth','libertyOrbitGravity','libertyOrbitSeed',
                       'object->setProperty("orbitMode"','object->setProperty("orbitGravity"',
                       'o->hasProperty("orbitMode")','orbitEngine.next(',
                       'orbitEngine.noteOn(','orbitEngine.noteOff(',
                       'orbitEngine.allNotesOff()',
                       'morphOn ? orbitModeParam->getIndex() : 0'):
            self.assertIn(needle,p)
        for needle in ('getOrbitMode()','getOrbitRate()','getOrbitDepth()',
                       'getOrbitGravity()','getOrbitSeed()','OrbitEngineV054.h'):
            self.assertIn(needle,h)
        self.assertIn('false);\n    addParameter(orbitGravityParam);',p)
    def test_french_and_non_overlapping_morph_controls(self):
        ui=(ROOT/'Source/LibertyMorphPadV036.h').read_text('utf8')
        for needle in ('ORBIT','INFINITY','CONSTELLATION','GRAVIT\\u00c9 MIDI',
                       'MOUVEMENT','VITESSE','AMPLITUDE','GRAINE',
                       'const int colWidth=(width-3*colGap)/4',
                       'getOrbitMode()','getOrbitDepth()'):
            self.assertIn(needle,ui)
        self.assertNotIn('\ufffd',ui)
    def test_version_wiring_and_diff_integrity(self):
        script=(ROOT/'scripts/apply_v054_orbit.py').read_text('utf8')
        workflow=(ROOT/'.github/workflows/build-rompler.yml').read_text('utf8')
        self.assertIn('assets/v054/orbit.patch.xz.b64',script)
        self.assertIn('hashlib.sha256',script)
        self.assertIn('python3 scripts/apply_v054_orbit.py',workflow)
        for forbidden in ('BankLibrary.cpp','SampleEngine.cpp','PluginEditor.cpp'):
            self.assertNotIn(forbidden,script)
