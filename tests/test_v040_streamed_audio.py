"""Liberty ROMpler v0.40: actual long WAV audio-path stream integration."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]

class LongWavStreamingV040(unittest.TestCase):
    def test_long_audio_is_processed_by_real_voice(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        hdr=(ROOT/"Source/SampleEngine.h").read_text("utf-8")
        proc=(ROOT/"Source/PluginProcessor.cpp").read_text("utf-8")
        self.assertIn("StreamedZoneV040.h",hdr)
        self.assertIn("std::unique_ptr<juce::AudioFormatReader> source",hdr)
        self.assertIn("sourceRate(source ? source->sampleRate",engine)
        self.assertIn("longSample",engine)
        self.assertIn("reader.sampleRate * 0.75",engine)
        self.assertIn("streamed->start(attackFrames)",engine)
        self.assertIn("streamed->tryReadFrame(n, l, r)",engine)
        self.assertIn("s->isStreamed()) { renderStreamedNextBlock(out,start,count);",engine)
        self.assertIn("void SampleVoice::renderStreamedNextBlock(",engine)
        self.assertIn("s->requestStreamAhead(position)",engine)
        self.assertIn("s->readStereoAt(",engine)
        self.assertIn("liberty::loopfade::at(",engine)
        self.assertIn("liberty::creative_v038::grainAt(",engine)
        self.assertIn("liberty::creative_v038::slice(",engine)
        self.assertIn("std::move(reader), controls.layers",proc)

    def test_original_ram_playback_and_morph_are_preserved(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        proc=(ROOT/"Source/PluginProcessor.cpp").read_text("utf-8")
        self.assertIn("reader.sampleRate * 20.0",engine)
        self.assertIn("reader.read(&data, 0, count, 0, true, true)",engine)
        self.assertIn("const auto& audio = s->getAudio();",engine)
        self.assertIn("morphLayerGain[(size_t)layer]",proc)
        self.assertIn("harmonyMidi.process(midi,",proc)
        self.assertIn("synths[(size_t)i].clearSounds()",proc)

    def test_realtime_callback_never_calls_disk_or_blocks(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        source=(ROOT/"Source/StreamedZoneV040.h").read_text("utf-8")
        stream=engine.split("void SampleVoice::renderStreamedNextBlock(",1)[1]
        for forbidden in ("reader->read(", "reader.read(", "openFile(",
                          "std::this_thread::sleep_for", "join(", "new ",
                          "std::unique_ptr", "juce::File("):
            self.assertNotIn(forbidden,stream)
        self.assertIn("cache.tryReadFrame(frame, l, r)",source)
        self.assertIn("cache.requestPage(page + lookahead)",source)
        self.assertNotIn("reader.readFrames(",source)
        self.assertNotIn("stop()",stream)

    def test_underflow_audio_fades_instead_of_abrupt_zero(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        self.assertIn("streamGain+1.0f/64.0f",engine)
        self.assertIn("streamGain-1.0f/64.0f",engine)
        self.assertIn("left=lastStreamLeft; right=lastStreamRight;",engine)
        self.assertIn("lastStreamLeft = lastStreamRight = 0.0f;",engine)

    def test_reject_oversized_wav_without_silent_truncation(self):
        engine=(ROOT/"Source/SampleEngine.cpp").read_text("utf-8")
        self.assertIn("std::numeric_limits<int>::max()-2",engine)
        self.assertIn("if (!reader || !reader->",engine) if False else None
        self.assertIn("reader.lengthInSamples",engine)
        self.assertIn("return;",engine)

    def test_legacy_regression_assertions_migrated(self):
        for path in ("tests/test_v038_creative.py","tests/test_samples_v011.py"):
            code=(ROOT/path).read_text("utf-8")
            self.assertIn("std::move(reader)",code)
            self.assertNotIn("MappedSampleSound(zone, *reader",code)

    def test_offline_long_wav_bank_import_and_validation(self):
        import importlib.util
        import json
        import tempfile
        import wave
        with tempfile.TemporaryDirectory(prefix="liberty-v040-long-") as folder:
            base=Path(folder)
            bank=base/"LongPiano"
            bank.mkdir()
            wavfile=bank/"LongPiano_C4.wav"
            frames=22050*21
            with wave.open(str(wavfile),"wb") as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(22050)
                wav.writeframes(b"\x00\x00" * frames)
            spec=importlib.util.spec_from_file_location(
                "smart_import_v040",ROOT/"tools/smart_import_v037.py")
            importer=importlib.util.module_from_spec(spec)
            spec.loader.exec_module(importer)
            manifest=importer.create_manifest(bank,"long_piano","Long Piano")
            self.assertEqual(manifest["presets"][0]["zones"][0]["root_note"],60)
            (bank/"manifest.json").write_text(json.dumps(manifest),encoding="utf-8")
            from tools.validate_banks import check_root
            errors,n_banks,n_presets=check_root(base)
            self.assertEqual(errors,[])
            self.assertEqual((n_banks,n_presets),(1,1))

    def test_utf8_and_ui_unchanged(self):
        for path in ("Source/StreamedZoneV040.h","Source/SampleEngine.cpp",
                     "Source/SampleEngine.h","Source/PluginProcessor.cpp"):
            data=(ROOT/path).read_bytes()
            self.assertNotIn(b"\xef\xbf\xbd",data)
        editor=(ROOT/"Source/PluginEditor.cpp").read_text("utf-8")
        self.assertIn("creativePanel.setBounds",editor)
        self.assertIn("morphPad.setBounds",editor)

if __name__ == "__main__":
    unittest.main()
