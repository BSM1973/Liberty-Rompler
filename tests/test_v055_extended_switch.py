import unittest
from tools.enforce_v055_extended_switch import failures

class ExtendedSwitch(unittest.TestCase):
    def report(self):
        return {"scenario":"switch","elapsed_wall_sec":60.0,
                "callback_over_budget":0,"underflow_blocks":0,"cache_misses":0,
                "dropped_pages":0,"urgent_dropped":0,"decoder_errors":0,
                "nonfinite_samples":0,"signal_peak":0.8}
    def test_clean_pass(self):
        self.assertEqual(failures(self.report()),[])
    def test_wall_overruns_fail(self):
        r=self.report()
        r["callback_over_budget"]=8
        self.assertIn("callback_over_budget: 8",failures(r))
    def test_fixed_polyphony_uses_same_strict_budget(self):
        r=self.report()
        r["scenario"]="poly64"
        self.assertEqual(failures(r,"poly64"),[])
        r["callback_over_budget"]=1
        self.assertIn("callback_over_budget: 1",failures(r,"poly64"))

    def test_fixed_polyphony_rejects_switch_report(self):
        self.assertTrue(failures(self.report(),"poly64"))

    def test_short_run_fails(self):
        r=self.report()
        r["elapsed_wall_sec"]=20.
        self.assertTrue(failures(r))
    def test_cache_miss_fails(self):
        r=self.report()
        r["cache_misses"]=1
        self.assertTrue(failures(r))
if __name__=="__main__":
    unittest.main()
