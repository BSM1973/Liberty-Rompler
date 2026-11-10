"""Liberty ROMpler v0.15: contract checks AFTER the reconstructed JUCE
source has been replaced by the reviewed v0.15 integration overlays.
No audio-device access, no Mac build, no destructive file operations.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent.parent


class IntegratedSamplesV015(unittest.TestCase):
    def test_rebuilt_juce_editor_has_real_controls(self):
        source = (ROOT / "Source" / "SampleZoneView.h").read_text(encoding="ascii")
        for fragment in (
            "nudgeLoopMarker(bool startMarker, int delta)",
            "previewSelectedLoop()",
            "processor.previewSampleZoneLoop(",
            "liberty::loop_v015::crossfadeBands(",
            "frameNudges[(size_t)i]",
            "const bool loopControlsFit = showLoops && rw >= 355 && rh >= 340",
        ):
            with self.subTest(fragment=fragment):
                self.assertIn(fragment, source)
        self.assertNotIn("const int fadeX =", source)
        self.assertIn("previewButton.setVisible(loopControlsFit)", source)

    def test_audio_callback_never_reads_wav(self):
        source = (ROOT / "Source" / "PluginProcessor.cpp").read_text(encoding="ascii")
        start = source.index("void LibertyRomplerAudioProcessor::processBlock(")
        end = source.index("bool LibertyRomplerAudioProcessor::getSampleZone(")
        callback = source[start:source.index("juce::Result LibertyRomplerAudioProcessor::previewSampleZoneLoop(", start)]
        precompute = source[source.index("juce::Result LibertyRomplerAudioProcessor::previewSampleZoneLoop(", start):end]
        self.assertIn("juce::ScopedTryLock previewTryLock(previewLock)", callback)
        self.assertIn("previewVoice.process(", callback)
        self.assertNotIn("formats.createReaderFor(", callback)
        self.assertNotIn("renderSeamPreview(", callback)
        self.assertIn("renderSeamPreview(", precompute)
        self.assertIn("const juce::ScopedLock lock(previewLock)", precompute)
        self.assertIn("currentSampleRate / reader->sampleRate", precompute)

    def test_processor_declares_safe_preview(self):
        header = (ROOT / "Source" / "PluginProcessor.h").read_text(encoding="ascii")
        for fragment in ("previewSampleZoneLoop(", "juce::CriticalSection previewLock",
                         "SeamPreview previewSnapshot", "SeamPreviewVoice previewVoice"):
            self.assertIn(fragment, header)

    def test_french_ui_uses_unicode_escapes(self):
        for filename in ("SampleZoneView.h", "PluginProcessor.cpp", "PluginProcessor.h"):
            with self.subTest(filename=filename):
                content = (ROOT / "Source" / filename).read_bytes()
                content.decode("ascii")  # Prevents mojibake in distributed source.
                self.assertNotIn(b"\xef\xbf\xbd", content)


if __name__ == "__main__":
    unittest.main()
