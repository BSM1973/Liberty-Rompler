import unittest
from tools.analyze_v055_slice_first_fault import summarize
class FirstFault(unittest.TestCase):
    def test_detects_first_precise_event(self):
        r={"scenario":"slice","underflow_blocks":2,"cache_misses":2,
           "slice_last_second_diagnostics":[
              {"at_sec":5.4,"underflows":0,"cache_misses":0,"dropped_pages":0,"urgent_dropped":0},
              {"at_sec":5.4116,"underflows":2,"cache_misses":2,"dropped_pages":0,"urgent_dropped":0}]}
        report=summarize(r)
        self.assertAlmostEqual(report["first_observed"]["at_sec"],5.4116)
        self.assertEqual(report["events_before_trace_or_unobserved"]["underflows"],0)
    def test_prior_fault_not_given_invented_time(self):
        r={"scenario":"slice","underflow_blocks":2,"cache_misses":2,
           "slice_last_second_diagnostics":[]}
        report=summarize(r)
        self.assertIsNone(report["first_observed"])
        self.assertEqual(report["events_before_trace_or_unobserved"]["cache_misses"],2)
if __name__=="__main__":
    unittest.main()
