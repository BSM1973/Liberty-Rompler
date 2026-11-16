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
    def test_allocator_can_release_in_use_while_rss_stays_constant(self):
        d=self.sample()
        d.update({"prior_rss_mb":490.16,"after_single_preset_rss_mb":490.16,
                  "idle_rss_mb":490.16,
                  "malloc_in_use_before_mb":360.0,
                  "malloc_in_use_after_switch_mb":310.0,
                  "malloc_in_use_idle_mb":310.0,
                  "malloc_reserved_before_mb":450.0,
                  "malloc_reserved_after_switch_mb":450.0,
                  "malloc_reserved_idle_mb":450.0})
        x=evaluate(d)
        self.assertTrue(x["allocator_measurements_available"])
        self.assertEqual(x["malloc_in_use_released_on_return_mb"],50.0)
        self.assertEqual(x["malloc_reserved_released_on_return_mb"],0.0)
        self.assertEqual(x["rss_change_on_return_mb"],0.0)

    def test_vm_footprint_can_fall_while_rss_does_not(self):
        d=self.sample()
        d.update({"vm_footprint_before_mb":430.0,"vm_footprint_after_switch_mb":380.0,
                  "vm_footprint_idle_mb":380.0,
                  "vm_internal_before_mb":420.0,"vm_internal_after_switch_mb":370.0,
                  "vm_internal_idle_mb":370.0,
                  "vm_compressed_before_mb":10.0,"vm_compressed_after_switch_mb":10.0,
                  "vm_compressed_idle_mb":10.0})
        result=evaluate(d)
        self.assertTrue(result["macos_vm_measurements_available"])
        self.assertEqual(result["vm_footprint_released_on_return_mb"],50.0)
        self.assertEqual(result["vm_internal_released_on_return_mb"],50.0)
        self.assertFalse(result["leak_excluded"])

    def test_bad_allocator_field_fails(self):
        d=self.sample()
        for key in ("malloc_in_use_before_mb","malloc_in_use_after_switch_mb",
                    "malloc_in_use_idle_mb","malloc_reserved_before_mb",
                    "malloc_reserved_after_switch_mb","malloc_reserved_idle_mb"):
            d[key]=250.
        d["malloc_in_use_idle_mb"]=-2.
        with self.assertRaises(ValueError):evaluate(d)

    def test_invalid_count_fails(self):
        d=self.sample()
        d["live_stream_zones_after_750ms"]=-1
        with self.assertRaises(ValueError):evaluate(d)
if __name__=="__main__":
    unittest.main()
