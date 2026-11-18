import pathlib
import subprocess
import sys
import tempfile
import unittest

SCRIPT=pathlib.Path(__file__).resolve().parents[1]/"scripts/apply_v055_audio_qos_diagnostics.py"

class AudioQoSInjection(unittest.TestCase):
    def test_applies_qos_to_native_runner_without_dsp_edits(self):
        with tempfile.TemporaryDirectory() as folder:
            root=pathlib.Path(folder)
            target=root/"tests"/"rompler_audio_stress_v044.cpp"
            target.parent.mkdir()
            target.write_text('#include <mach/mach.h>\nint main(){\n'
                              '        juce::ScopedJuceInitialiser_GUI juceGui;\n}\n')
            subprocess.run([sys.executable,str(SCRIPT)],cwd=root,check=True,capture_output=True,text=True)
            result=target.read_text()
            self.assertIn("QOS_CLASS_USER_INTERACTIVE",result)
            self.assertIn("audioQosError==0",result)
            self.assertIn("LIBERTY_NATIVE_AUDIO_QOS",result)
            self.assertEqual(result.count("pthread_set_qos_class_self_np"),1)
    def test_refuses_unexpected_harness_source(self):
        with tempfile.TemporaryDirectory() as folder:
            root=pathlib.Path(folder)
            target=root/"tests"/"rompler_audio_stress_v044.cpp"
            target.parent.mkdir()
            target.write_text("int main(){}\n")
            result=subprocess.run([sys.executable,str(SCRIPT)],cwd=root,capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)

if __name__=="__main__":
    unittest.main()
