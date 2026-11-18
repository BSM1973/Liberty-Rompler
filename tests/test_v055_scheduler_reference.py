import unittest
from tools.analyze_v055_scheduler_reference import compare

class SchedulerReferenceDiagnostic(unittest.TestCase):
    def reference(self, spikes=0):
        return {'scenario':'scheduler_reference','rate_hz':44100,'block_frames':512,
                'elapsed_wall_sec':60.0,'qos_interactive_confirmed':True,
                'synthetic_cpu_work_target_ms':1.0,'callback_over_budget':spikes,
                'spikes_truncated':False,'callback_spikes':
                [{'callback_ms':20.,'thread_cpu_ms':1.0}] * spikes}
    def real(self, spikes=3):
        return {'scenario':'poly64','rate_hz':44100,'block_frames':512,
                'elapsed_wall_sec':60.,'callback_over_budget':spikes,
                'callback_spikes':[{'callback_ms':15.,'thread_cpu_ms':2.0}]*spikes}
    def test_reference_spike_is_only_evidence(self):
        result=compare(self.reference(1),self.real(3))
        self.assertTrue(result['evidence_of_external_runner_jitter'])
        self.assertFalse(result['root_cause_proven'])
        self.assertFalse(result['qualification_override'])
        self.assertFalse(result['real_stress_passed'])
    def test_zero_reference_does_not_clear_real_overruns(self):
        result=compare(self.reference(),self.real())
        self.assertFalse(result['evidence_of_external_runner_jitter'])
        self.assertEqual(result['real_64voice_overruns'],3)
    def test_qos_required(self):
        ref=self.reference()
        ref['qos_interactive_confirmed']=False
        with self.assertRaises(ValueError): compare(ref,self.real())
    def test_mismatched_budget_rejected(self):
        ref=self.reference()
        ref['block_frames']=128
        with self.assertRaises(ValueError): compare(ref,self.real())
    def test_missing_spikes_rejected(self):
        ref=self.reference()
        ref['callback_over_budget']=1
        with self.assertRaises(ValueError): compare(ref,self.real())

if __name__ == '__main__': unittest.main()
