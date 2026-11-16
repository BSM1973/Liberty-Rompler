import unittest
from tools.analyze_v055_stream_lifetime import evaluate
class NativeLifetime(unittest.TestCase):
    def sample(self):
        return {"prior_rss_mb":450.0,"after_single_preset_rss_mb":449.0,
                "idle_rss_mb":449.0,"live_stream_zones_before":64,
                "live_stream_zones_after_switch":16,
                "live_stream_zones_after_750ms":16,
                "streamed_zones_after_switch":16}
    def test_expected_return(self):
        x=evaluate(self.sample())
        self.assertTrue(x["count_matches_single_layer"])
        self.assertEqual(x["excess_live_instances_after_750ms"],0)
        self.assertFalse(x["leak_excluded"])
    def test_retain_objects_without_pretending_proven_leak(self):
        d=self.sample()
        d["live_stream_zones_after_750ms"]=64
        x=evaluate(d)
        self.assertEqual(x["excess_live_instances_after_750ms"],48)
        self.assertFalse(x["leak_proven"])
    def test_invalid_count_fails(self):
        d=self.sample()
        d["live_stream_zones_after_750ms"]=-1
        with self.assertRaises(ValueError):evaluate(d)
if __name__=="__main__":
    unittest.main()
