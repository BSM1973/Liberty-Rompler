import unittest
from tools.analyze_v055_callback_spikes import classify
class CallbackSpikes(unittest.TestCase):
    def test_off_cpu(self):
        scene={"rate_hz":44100,"block_frames":512,
               "callback_spikes":[{"at_sec":5.0,"callback_ms":20,"thread_cpu_ms":0.4}]}
        r=classify({"scenarios":{name:scene for name in ("poly64","switch","slice")}})
        self.assertEqual(r["off_cpu_or_blocked"],3)
        self.assertEqual(r["cpu_bound"],0)
        self.assertFalse(r["qualification_override"])
    def test_cpu_bound_and_unknown(self):
        scene={"rate_hz":44100,"block_frames":512,"callback_spikes":[
            {"callback_ms":20,"thread_cpu_ms":17},{"callback_ms":14}]}
        r=classify({"scenarios":{name:scene for name in ("poly64","switch","slice")}})
        self.assertEqual(r["cpu_bound"],3)
        self.assertEqual(r["unclassified"],3)
if __name__=="__main__":
    unittest.main()
