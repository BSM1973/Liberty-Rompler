"""v0.55 Factory Starter contract: 100 distinctive, valid v1 multisample programs."""
import unittest
import numpy as np
from tools.make_factory_100_v055 import LIBRARY, ROOTS, RANGES, SAMPLE_RATE, slug, synth

class Factory100Contract(unittest.TestCase):
    def test_exactly_100_unique_curated_programs(self):
        names=[x for group in LIBRARY.values() for x in group]
        self.assertEqual(len(LIBRARY),10)
        self.assertEqual(len(names),100)
        self.assertEqual(len({slug(x) for x in names}),100)
    def test_root_ranges_cover_midi_keyboard_without_overlap(self):
        self.assertEqual(len(ROOTS),4)
        self.assertEqual(len(RANGES),4)
        self.assertEqual([x for low,high in RANGES for x in (low,high)],
                         [0,44,45,60,61,76,77,127])
    def test_original_sustained_pcm_and_safe_loop(self):
        pcm,sustain,start,end=synth('Cinematic Pads',1,60)
        self.assertTrue(sustain)
        self.assertEqual(pcm.dtype,np.dtype('<i2'))
        self.assertEqual(pcm.shape[1],2)
        self.assertGreater(int(np.max(np.abs(pcm.astype(np.int32)))),1000)
        self.assertTrue(0<start<end<len(pcm))
    def test_original_percussion_has_no_loop(self):
        pcm,sustain,start,end=synth('Bells & Mallets',6,60)
        self.assertFalse(sustain)
        self.assertEqual((start,end),(-1,-1))
        self.assertTrue(np.isfinite(pcm).all())
    def test_audio_quality_baseline(self):
        pcm,*_=synth('Analog Basses',2,48)
        self.assertEqual(SAMPLE_RATE,32000)
        self.assertEqual(pcm.shape[1],2)
        self.assertLess(int(np.max(np.abs(pcm.astype(np.int32)))),32000)
        self.assertGreater(int(np.max(np.abs(pcm.astype(np.int32)))),2000)

if __name__=='__main__':
    unittest.main()
