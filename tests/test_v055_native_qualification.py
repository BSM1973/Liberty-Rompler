import unittest
from tools.enforce_v055_native_qualification import disqualifications
class StrictQualification(unittest.TestCase):
    def report(self):
        cases={x:{"streaming_qualified":True,"callback_over_budget":0,
                   "underflow_blocks":0,"decoder_errors":0,"urgent_dropped":0,
                   "dropped_pages":0,"nonfinite_samples":0,
                   "suspicious_silent_sustain_blocks":0}
               for x in ("poly64","switch","slice")}
        return {"qualified_on_ci_machine":True,"scenarios":cases}
    def test_clean_passes(self):
        self.assertEqual(disqualifications(self.report()),[])
    def test_budget_failure_cannot_pass(self):
        data=self.report()
        data["scenarios"]["poly64"]["callback_over_budget"]=3
        data["scenarios"]["poly64"]["streaming_qualified"]=False
        data["qualified_on_ci_machine"]=False
        self.assertTrue(disqualifications(data))
    def test_slice_cache_and_underflow_failure(self):
        data=self.report()
        data["scenarios"]["slice"]["underflow_blocks"]=1
        self.assertTrue(disqualifications(data))
    def test_broken_data_fails_closed(self):
        data=self.report()
        data["scenarios"]["switch"]["callback_over_budget"]=-1
        self.assertTrue(disqualifications(data))
    def test_missing_scenario_fails(self):
        data=self.report()
        del data["scenarios"]["slice"]
        self.assertTrue(disqualifications(data))
if __name__=="__main__":
    unittest.main()
