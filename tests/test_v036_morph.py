"""Liberty MORPH v0.36: real host parameters, MIDI, DSP, UI and legacy safety."""
from pathlib import Path
import subprocess
import tempfile
import shutil
import unittest

ROOT = Path(__file__).resolve().parent.parent


class LibertyMorphV036(unittest.TestCase):
    def test_weights_compiled_cpp17(self):
        compiler = shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory(prefix="liberty-v036-") as folder:
            output = Path(folder) / "morph-weights"
            subprocess.run(
                [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                 "-pedantic", str(ROOT / "tests/morph_v036_test.cpp"),
                 "-o", str(output)],
                check=True, timeout=120, cwd=ROOT)
            result = subprocess.run([str(output)], text=True,
                                    capture_output=True, timeout=20, cwd=ROOT)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("MORPH weights, MIDI and boundaries OK", result.stdout)

    def test_real_audio_dsp_and_host_automation(self):
        hdr = (ROOT / "Source/PluginProcessor.h").read_text(encoding="ascii")
        cpp = (ROOT / "Source/PluginProcessor.cpp").read_text(encoding="ascii")
        for name in ("libertyMorphActive", "libertyMorphX", "libertyMorphY",
                     "libertyMorphMidi"):
            with self.subTest(name=name):
                self.assertIn(name, cpp)
        for required in (
            '#include "MorphWeightsV036.h"',
            'juce::AudioParameterBool* morphActiveParam',
            'juce::AudioParameterFloat* morphXParam',
            'juce::AudioParameterFloat* morphYParam',
            'juce::AudioParameterBool* morphMidiParam',
            "morphDisplayX() const noexcept",
            "morphDisplayY() const noexcept",
            "morphLayerGain",
        ):
            self.assertIn(required, hdr)
        for required in (
            "addParameter(morphActiveParam);",
            "addParameter(morphXParam);",
            "addParameter(morphYParam);",
            "addParameter(morphMidiParam);",
            "morphLayerGain[(size_t)layer]",
            "gain.reset(currentSampleRate, 0.045);",
            "gain.setCurrentAndTargetValue(1.0f)",
            "liberty::morph_v036::weights(",
            "liberty::morph_v036::fromMidi(",
            "message.getControllerNumber() == 16",
            "message.getControllerNumber() == 17",
            "gainSmoother.getNextValue()",
            "morphActiveParam->get()",
            'object->setProperty("morphActive"',
            'object->setProperty("morphX"',
            'object->setProperty("morphY"',
            'object->setProperty("morphMidi"',
            'o->hasProperty("morphActive")',
            "audio.addFrom(channel,0,layerScratch,channel,0,blockLength);",
        ):
            with self.subTest(required=required):
                self.assertIn(required, cpp)
        render = cpp[cpp.index("void LibertyRomplerAudioProcessor::processBlock("):
                     cpp.index("juce::Result LibertyRomplerAudioProcessor::previewSampleZoneLoop(")]
        self.assertLess(render.index("morphLayerGain[(size_t)layer]"),
                        render.index("fxRacks[4].process("))
        self.assertNotIn("setValueNotifyingHost(", render)
        self.assertNotIn("new juce::", render)

    def test_real_xy_pad_and_responsive_tenth_tab(self):
        header = (ROOT / "Source/PluginEditor.h").read_text(encoding="ascii")
        src = (ROOT / "Source/PluginEditor.cpp").read_text(encoding="ascii")
        pad = (ROOT / "Source/LibertyMorphPadV036.h").read_text(encoding="ascii")
        for required in (
            "Page { Presets, Layers, Morph, Routing",
            'juce::TextButton morphTab {"MORPH"};',
            "LibertyMorphPadV036 morphPad;",
        ):
            self.assertIn(required, header)
        for required in (
            "morphPad(p)",
            "makeTab(morphTab, Page::Morph);",
            "morphPad.setVisible(next==Page::Morph);",
            "morphPad.setBounds(work.reduced(16,8)",
            "activePage==Page::Morph ?",
            "juce::jlimit(72,116,(getWidth()-294-24-9*tabGap)/10)",
        ):
            self.assertIn(required, src)
        for required in (
            "getMorphX().beginChangeGesture()",
            "getMorphY().endChangeGesture()",
            "getMorphX().setValueNotifyingHost(x)",
            "getMorphY().setValueNotifyingHost(y)",
            "getMorphMidiMode()",
            "morphDisplayX()",
            "morphDisplayY()",
            "mouseDrag(const juce::MouseEvent& event)",
            "startTimerHz(24)",
        ):
            self.assertIn(required, pad)
        for width in (1120,1360,1920):
            gap = 4
            tab_width = max(72,min(116,(width-294-24-9*gap)//10))
            last_right = 294+10*tab_width+9*gap
            self.assertLessEqual(last_right,width-16)
        # At the smallest supported size, XY pad and control row have space.
        minimum_morph_height = 750-125-146-16-40-8
        side = min(400,minimum_morph_height-139)
        self.assertGreaterEqual(side,260)
        self.assertGreaterEqual((minimum_morph_height-side)/2+6,43)
        self.assertLessEqual((minimum_morph_height+side)/2+6,
                             minimum_morph_height-49)

    def test_french_accents_and_non_destructive_compatibility(self):
        for path in ("Source/LibertyMorphPadV036.h", "Source/PluginEditor.cpp",
                     "Source/PluginEditor.h", "Source/PluginProcessor.cpp"):
            data = (ROOT / path).read_bytes()
            data.decode("ascii")
            self.assertNotIn(b"\xef\xbf\xbd",data)
        self.assertIn("liberty::loop_v035::", (ROOT/"Source/SampleZoneView.h").read_text(encoding="ascii"))
        source = (ROOT/"Source/PluginProcessor.cpp").read_text(encoding="ascii")
        self.assertIn('o->hasProperty("morphActive")',source)
        self.assertIn("morphActiveParam)->setValue(",source)
        self.assertIn("juce::AudioChannelSet::stereo()",source)
        self.assertIn("sampleZoneEdits",source)
        self.assertIn("arpEnabled",source)


if __name__ == "__main__":
    unittest.main()
