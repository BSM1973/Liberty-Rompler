import unittest
from tools.analyze_v055_long_switch_spikes import analyze

class LongSwitchSpikes(unittest.TestCase):
    def base(self):
        return {"scenario":"switch","rate_hz":44100,"block_frames":512,
                "callback_over_budget":2,
                "switch_rss_trace":[{"at_sec":1.0,"rss_mb":150,"layers":1},
                                    {"at_sec":2.0,"rss_mb":160,"layers":4}],
                "callback_spikes":[
                    {"at_sec":1.012,"callback_ms":20.,"thread_cpu_ms":0.5},
                    {"at_sec":1.650,"callback_ms":18.,"thread_cpu_ms":0.4}]}
    def test_correlates_but_does_not_claim_causation(self):
        result=analyze(self.base())
        self.assertEqual(result["spikes_near_preset_switch"],1)
        self.assertEqual(result["off_cpu_or_blocked"],2)
        self.assertTrue(result["counter_matches_spikes"])
        self.assertFalse(result["causality_proven"])
    def test_missing_timestamps_report_uncertainty(self):
        data=self.base()
        data["switch_rss_trace"]=[]
        result=analyze(data)
        self.assertIsNone(result["spikes"][0]["nearest_preset_switch_sec"])
    def test_rejects_non_switch(self):
        data=self.base()
        data["scenario"]="poly64"
        with self.assertRaises(ValueError):analyze(data)
if __name__=="__main__":
    unittest.main()
