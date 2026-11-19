import unittest
from tools.analyze_v055_switch_memory import analyze

class SwitchMemorySummary(unittest.TestCase):
    def test_realistic_trend(self):
        rows=[{"at_sec":float(i),"rss_mb":100.+.5*i} for i in range(20)]
        result=analyze({"scenario":"switch","switch_rss_trace":rows})
        self.assertEqual(result["rss_points"],20)
        self.assertGreater(result["median_quarter_growth_mb"],0)
        self.assertFalse(result["full_plugin_or_daw_qualified"])

    def test_warmup_growth_then_stable_last_third(self):
        rows=[]
        for i in range(60):
            # Cache warmup in first 40 swaps, level in final third.
            rss=100.+2.*min(i,39)
            rows.append({"at_sec":float(i),"rss_mb":rss,"layers":4 if i%2==0 else 1})
        result=analyze({"scenario":"switch","switch_rss_trace":rows})
        self.assertEqual(result["rss_points"],60)
        self.assertEqual(result["last_third_growth_mb"],0.0)
        self.assertIn("4",result["by_preset_layer_count"])
        self.assertIn("1",result["by_preset_layer_count"])
        self.assertFalse(result["cache_stabilization_proven"])

    def test_continued_growth_detected_but_not_labeled_leak(self):
        rows=[{"at_sec":float(i),"rss_mb":100.+float(i),"layers":4 if i%2 else 1}
              for i in range(60)]
        result=analyze({"scenario":"switch","switch_rss_trace":rows})
        self.assertGreater(result["last_third_growth_mb"],10)
        self.assertFalse(result["cache_stabilization_proven"])

    def test_reject_insufficient_data(self):
        with self.assertRaises(ValueError):
            analyze({"scenario":"switch","switch_rss_trace":[{"at_sec":0,"rss_mb":100}]})

if __name__=="__main__":
    unittest.main()
