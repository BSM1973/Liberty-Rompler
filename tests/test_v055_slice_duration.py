import unittest
from tools.analyze_v055_slice_duration import compare
class SliceDuration(unittest.TestCase):
    def report(self,sec,fail):
        return {"scenario":"slice","elapsed_wall_sec":sec,
                "underflow_blocks":int(fail),"cache_misses":int(fail),
                "slice_last_second_diagnostics":[
                  {"at_sec":sec-0.1,"underflows":0,"cache_misses":0,"dropped_pages":0,"urgent_dropped":0},
                  {"at_sec":sec-0.02,"underflows":int(fail),"cache_misses":int(fail),"dropped_pages":0,"urgent_dropped":0}]}
    def test_reports_tail_without_waiving_failure(self):
        r=compare(self.report(6,True),self.report(8,False))
        self.assertEqual(r["by_duration"]["short_6s"]["underflow_blocks"],1)
        self.assertAlmostEqual(r["by_duration"]["short_6s"]["first_observed_sec_before_end"],.02)
        self.assertFalse(r["qualification_override"])
    def test_full_run_trace_exposes_early_cache_fault(self):
        short=self.report(6,False)
        long=self.report(8,True)
        long["slice_full_run_diagnostics"]=[
            {"at_sec":0.0,"underflows":0,"cache_misses":0,"dropped_pages":0,"urgent_dropped":0},
            {"at_sec":3.25,"underflows":1,"cache_misses":1,"dropped_pages":0,"urgent_dropped":0},
            {"at_sec":7.98,"underflows":1,"cache_misses":1,"dropped_pages":0,"urgent_dropped":0}]
        r=compare(short,long)
        self.assertEqual(r["by_duration"]["long_8s"]["first_observed_at_sec"],3.25)
        self.assertAlmostEqual(r["by_duration"]["long_8s"]["first_observed_sec_before_end"],4.75)
        self.assertFalse(r["qualification_override"])

    def test_rejects_invalid_scenario(self):
        x=self.report(6,False);x["scenario"]="switch"
        with self.assertRaises(ValueError):compare(x,self.report(8,False))
if __name__=="__main__":
    unittest.main()
