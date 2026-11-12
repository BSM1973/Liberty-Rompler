from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
class SlowPoolV044(unittest.TestCase):
    def test_64_zone_priority_under_synthetic_delay(self):
        compiler=shutil.which('c++') or shutil.which('clang++')
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as folder:
            exe=Path(folder)/'slow-pool'
            subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra',
                            '-Werror','-pedantic','-pthread',
                            str(ROOT/'tests/streaming_slow_pool_v044_test.cpp'),
                            '-o',str(exe)],check=True,timeout=120)
            result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=12)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertIn('64-zone artificial 2ms slow worker priority OK',result.stdout)
