import unittest
from tools.analyze_v055_long_control import compare
class CompareLongControl(unittest.TestCase):
    def report(self,mode,spikes):
        return {"scenario":mode,"elapsed_wall_sec":59.9,
                "rate_hz":44100,"block_frames":512,
                "callback_over_budget":len(spikes),"callback_spikes":spikes}
    def spike(self):
        return {"callback_ms":24,"thread_cpu_ms":0.5}
    def test_control_isolation_without_claim_of_causality(self):
        result=compare(self.report("poly64",[self.spike()]),
                       self.report("switch",[self.spike(),self.spike()]))
        self.assertTrue(result["control_has_off_cpu_stalls"])
        self.assertTrue(result["switch_has_off_cpu_stalls"])
        self.assertGreater(result["difference_overruns_per_min"],0)
        self.assertFalse(result["causality_proven"])
        self.assertFalse(result["qualification_override"])
    def test_no_spikes_control(self):
        result=compare(self.report("poly64",[]),self.report("switch",[self.spike()]))
        self.assertEqual(result["constant_64_voices_control"]["overruns"],0)
    def test_bad_counter_rejected(self):
        control=self.report("poly64",[])
        control["callback_over_budget"]=1
        with self.assertRaises(ValueError):
            compare(control,self.report("switch",[]))
    def test_short_control_rejected(self):
        control=self.report("poly64",[])
        control["elapsed_wall_sec"]=5
        with self.assertRaises(ValueError):
            compare(control,self.report("switch",[]))
if __name__=="__main__":
    unittest.main()
