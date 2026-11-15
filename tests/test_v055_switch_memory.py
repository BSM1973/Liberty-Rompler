import unittest
from tools.analyze_v055_switch_memory import analyze

class SwitchMemorySummary(unittest.TestCase):
    def test_realistic_trend(self):
        rows=[{"at_sec":float(i),"rss_mb":100.+.5*i} for i in range(20)]
        result=analyze({"scenario":"switch","switch_rss_trace":rows})
        self.assertEqual(result["rss_points"],20)
        self.assertGreater(result["median_quarter_growth_mb"],0)
        self.assertFalse(result["full_plugin_or_daw_qualified"])

    def test_reject_insufficient_data(self):
        with self.assertRaises(ValueError):
            analyze({"scenario":"switch","switch_rss_trace":[{"at_sec":0,"rss_mb":100}]})

if __name__=="__main__":
    unittest.main()
