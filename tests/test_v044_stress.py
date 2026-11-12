"""The bank is validated against the real v1 manifest validator."""
from pathlib import Path
from importlib.util import spec_from_file_location, module_from_spec
import json
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class StressV044(unittest.TestCase):
    def test_generator_validates_and_does_not_overwrite(self):
        sys.path.insert(0,str(ROOT/'tools'))
        from validate_banks import check_root
        spec=spec_from_file_location('stress_v044',ROOT/'tools/generate_streaming_stress_v044.py')
        module=module_from_spec(spec)
        spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as tmp:
            bank=Path(tmp)/'StressBank'
            manifest=module.generate(bank,.12,22050,2)
            errors,banks,presets=check_root(Path(tmp))
            self.assertEqual((errors,banks,presets),([],1,5))
            doc=json.loads(manifest.read_text(encoding='utf-8'))
            self.assertEqual(len(doc['presets'][-1]['layers']),4)
            self.assertEqual(sum(len(p['zones']) for p in doc['presets']),8)
            with self.assertRaises(FileExistsError):
                module.generate(bank,.12,22050,2)
    def test_native_engine_runner_has_genuine_audio_and_metrics(self):
        s=(ROOT/'tests/rompler_audio_stress_v044.cpp').read_text('utf-8')
        for text in ('processor.processBlock(audio,midi)', 'processor.selectPreset(',
                     'processor.getStreamingDiagnostics()', 'getrusage(RUSAGE_SELF',
                     'std::this_thread::sleep_until', 'callback_over_budget',
                     'peak_rss_mb', 'underflow_blocks', 'nonfinite_samples',
                     'suspicious_silent_sustain_blocks', 'zones_at_start'):
            self.assertIn(text,s)
        self.assertIn('insist(before.streamedZones==64',s)
    def test_macos_release_has_real_audio_stress(self):
        s=(ROOT/'.github/workflows/build-rompler.yml').read_text('utf-8')
        for key in ('LIBERTY_BUILD_STRESS_V044=ON',
                    'tools/generate_streaming_stress_v044.py',
                    'LibertyStressV044','verify_stress_v044.py'):
            self.assertIn(key,s)
