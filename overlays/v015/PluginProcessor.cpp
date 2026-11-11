#include "PluginProcessor.h"
#include "LoopCrossfade.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <memory>

LibertyRomplerAudioProcessor::LibertyRomplerAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      library(liberty::BankLibrary::defaultDirectory()) {
    // Stable IDs; v0.36 is opt-in so all previous project mixes remain unchanged.
    morphActiveParam = new juce::AudioParameterBool(
        juce::ParameterID("libertyMorphActive", 1), "Liberty MORPH Active", false);
    addParameter(morphActiveParam);
    morphXParam = new juce::AudioParameterFloat(
        juce::ParameterID("libertyMorphX", 1), "Liberty MORPH X",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f);
    addParameter(morphXParam);
    morphYParam = new juce::AudioParameterFloat(
        juce::ParameterID("libertyMorphY", 1), "Liberty MORPH Y",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f);
    addParameter(morphYParam);
    morphMidiParam = new juce::AudioParameterBool(
        juce::ParameterID("libertyMorphMidi", 1), "Liberty MORPH MIDI CC16/17", false);
    addParameter(morphMidiParam);
    formats.registerBasicFormats();
    for (auto& synth : synths)
        for (int i = 0; i < 32; ++i) synth.addVoice(new liberty::SampleVoice(controls));
    for (size_t i = 0; i < controls.masterFx.size(); ++i)
        controls.masterFx[i].store((int)i+1);
    for (auto& row : controls.layerFx)
        for (auto& slot : row) slot.store(0);
    if (!library.getBanks().empty() && !library.getBanks()[0].presets.empty()) {
        const auto& bank = library.getBanks()[0];
        selectPreset(bank.id, bank.presets[0].id);
    }
}

void LibertyRomplerAudioProcessor::prepareToPlay(double rate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = juce::jmax(8000.0, rate);
    arpEngine.prepare(currentSampleRate);
    for (auto& synth : synths) synth.setCurrentPlaybackSampleRate(currentSampleRate);
    // No allocation in the render loop; 65536 supports unusually large host buffers.
    layerScratch.setSize(2, juce::jmax(65536, maximumExpectedSamplesPerBlock));
    layerScratch.clear();
    for (auto& gain : morphLayerGain) {
        gain.reset(currentSampleRate, 0.045); // 45 ms click-free transition
        gain.setCurrentAndTargetValue(1.0f); // legacy sound unchanged by default
    }
    for (auto& rack : fxRacks) rack.prepare(currentSampleRate, juce::jmax(65536, maximumExpectedSamplesPerBlock));
    juce::dsp::ProcessSpec spec { currentSampleRate,
                                  (juce::uint32)juce::jmax(1, maximumExpectedSamplesPerBlock), 2 };
    lowPass.prepare(spec);
    lowPass.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    lowPass.setCutoffFrequency(juce::jmin((float)(currentSampleRate*0.45), controls.cutoffHz.load()));
    lowPass.setResonance(controls.resonance.load());
    lowPass.reset();
    filterEnvelope.setSampleRate(currentSampleRate);
    filterEnvelope.reset();
    heldFilterNotes.fill(false);
    heldFilterNoteCount = 0;
    lfoPhaseOne = lfoPhaseTwo = 0.0;
    currentModWheel = 0.0f;
}
float LibertyRomplerAudioProcessor::morphDisplayX() const noexcept {
    return liberty::morph_v036::normalized(morphMidiParam->get()
        ? midiMorphX.load(std::memory_order_relaxed) : morphXParam->get());
}
float LibertyRomplerAudioProcessor::morphDisplayY() const noexcept {
    return liberty::morph_v036::normalized(morphMidiParam->get()
        ? midiMorphY.load(std::memory_order_relaxed) : morphYParam->get());
}
bool LibertyRomplerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
void LibertyRomplerAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    audio.clear();
    // MIDI CC16/CC17 move XY when enabled. Do not notify the host from
    // processBlock: automation uses dedicated JUCE parameters instead.
    const bool useMidiMorph = morphMidiParam->get();
    // Capture the channel modulation wheel before the keyboard consumes input.
    for (const auto metadata : midi) {
        auto message = metadata.getMessage();
        if (message.isController() && message.getControllerNumber() == 1)
            currentModWheel = (float)message.getControllerValue() / 127.0f;
        if (useMidiMorph && message.isController()) {
            if (message.getControllerNumber() == 16)
                midiMorphX.store((float)message.getControllerValue() / 127.0f,
                                 std::memory_order_relaxed);
            else if (message.getControllerNumber() == 17)
                midiMorphY.store((float)message.getControllerValue() / 127.0f,
                                 std::memory_order_relaxed);
        }
    }
    keyboard.processNextMidiBuffer(midi, 0, audio.getNumSamples(), true);
    // Generate sample-accurate arpeggio MIDI before all four layers and the
    // filter envelope see it. Disabled by default for legacy presets.
    juce::Optional<juce::AudioPlayHead::PositionInfo> hostPosition;
    if (auto* playHead = getPlayHead()) hostPosition = playHead->getPosition();
    arpEngine.process(midi, audio.getNumSamples(), arpSettings,
                      hostPosition.hasValue() ? &*hostPosition : nullptr);
    // The virtual keyboard has just merged its MIDI events, so both live MIDI
    // and on-screen notes trigger the global filter ADSR.
    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            if (note >= 0 && note < 128 && !heldFilterNotes[(size_t)note]) {
                heldFilterNotes[(size_t)note] = true;
                if (++heldFilterNoteCount == 1) filterEnvelope.noteOn();
            }
        } else if (msg.isNoteOff()) {
            const int note = msg.getNoteNumber();
            if (note >= 0 && note < 128 && heldFilterNotes[(size_t)note]) {
                heldFilterNotes[(size_t)note] = false;
                if (--heldFilterNoteCount == 0) filterEnvelope.noteOff();
            }
        } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
            heldFilterNotes.fill(false);
            heldFilterNoteCount = 0;
            filterEnvelope.noteOff();
        }
    }
    // Layer voices are rendered independently to make per-layer FX chains real.
    // Each JUCE synth reads the same MIDI events; its sounds belong to one layer.
    const int blockLength = audio.getNumSamples();
    if (blockLength > layerScratch.getNumSamples()) return; // guard host contract
    const bool morphOn = morphActiveParam->get();
    const auto morphWeights = liberty::morph_v036::weights(
        useMidiMorph ? midiMorphX.load(std::memory_order_relaxed) : morphXParam->get(),
        useMidiMorph ? midiMorphY.load(std::memory_order_relaxed) : morphYParam->get());
    for (int layer=0; layer<4; ++layer) {
        auto& gainSmoother = morphLayerGain[(size_t)layer];
        gainSmoother.setTargetValue(morphOn ? morphWeights[(size_t)layer] : 1.0f);
        layerScratch.clear(0,blockLength);
        synths[(size_t)layer].renderNextBlock(layerScratch, midi, 0, blockLength);
        for (int slot=0; slot<2; ++slot) {
            const int effect=controls.layerFx[(size_t)layer][(size_t)slot].load(std::memory_order_relaxed);
            fxRacks[(size_t)layer].process(effect,layerScratch,controls,blockLength);
        }
        // MORPH is applied after each layer FX chain, before the master FX.
        // The bypass fast path is sample-equivalent to pre-v0.36 mixing.
        const int channels = juce::jmin(2, audio.getNumChannels());
        if (!morphOn && !gainSmoother.isSmoothing()) {
            for (int channel=0; channel<channels; ++channel)
                audio.addFrom(channel,0,layerScratch,channel,0,blockLength);
        } else if (channels > 0) {
            float* destL = audio.getWritePointer(0);
            const float* srcL = layerScratch.getReadPointer(0);
            float* destR = channels > 1 ? audio.getWritePointer(1) : nullptr;
            const float* srcR = channels > 1 ? layerScratch.getReadPointer(1) : nullptr;
            for (int sample=0; sample<blockLength; ++sample) {
                const float gain = gainSmoother.getNextValue();
                destL[sample] += srcL[sample] * gain;
                if (destR != nullptr) destR[sample] += srcR[sample] * gain;
            }
        }
    }

    // Audio-thread-only LFO state: 4 independent user matrix slots, 2 LFOs,
    // MIDI CC1. Never allocate or lock on the real-time audio thread.
    const auto rateOne = juce::jlimit(0.05f, 12.0f, controls.lfoOneHz.load(std::memory_order_relaxed));
    const auto rateTwo = juce::jlimit(0.05f, 12.0f, controls.lfoTwoHz.load(std::memory_order_relaxed));
    const auto count = audio.getNumSamples();
    const auto lfoWave = [](double phase, int wave) -> float {
        const float p = (float)phase;
        switch (wave) {
            case 1: return 1.0f - 4.0f * std::abs(p - 0.5f); // triangle
            case 2: return 2.0f * p - 1.0f;                 // sawtooth
            case 3: return p < 0.5f ? 1.0f : -1.0f;         // square
            default: return std::sin(juce::MathConstants<float>::twoPi * p);
        }
    };
    const float lfoOne = lfoWave(lfoPhaseOne, controls.lfoOneWave.load(std::memory_order_relaxed));
    const float lfoTwo = lfoWave(lfoPhaseTwo, controls.lfoTwoWave.load(std::memory_order_relaxed));
    lfoPhaseOne = std::fmod(lfoPhaseOne + (double)rateOne * (double)count / currentSampleRate, 1.0);
    lfoPhaseTwo = std::fmod(lfoPhaseTwo + (double)rateTwo * (double)count / currentSampleRate, 1.0);
    float filterMod = 0.0f, ampMod = 0.0f;
    for (const auto& route : controls.matrix) {
        const int source = route.source.load(std::memory_order_relaxed);
        const int target = route.target.load(std::memory_order_relaxed);
        if (source == 0 || target == 0) continue;
        const auto signal = source == 1 ? lfoOne : source == 2 ? lfoTwo :
                            source == 3 ? currentModWheel : 0.0f;
        const auto contribution = signal * route.amount.load(std::memory_order_relaxed);
        if (target == 1) filterMod += contribution;
        if (target == 2) ampMod += contribution;
    }
    juce::ADSR::Parameters env;
    env.attack  = juce::jlimit(0.001f, 5.0f, controls.filterAttack.load(std::memory_order_relaxed));
    env.decay   = juce::jlimit(0.01f, 5.0f, controls.filterDecay.load(std::memory_order_relaxed));
    env.sustain = juce::jlimit(0.0f, 1.0f, controls.filterSustain.load(std::memory_order_relaxed));
    env.release = juce::jlimit(0.01f, 8.0f, controls.filterRelease.load(std::memory_order_relaxed));
    filterEnvelope.setParameters(env);
    float filterEnvLevel = 0.0f;
    for (int i = 0; i < count; ++i) filterEnvLevel = filterEnvelope.getNextSample();
    const float envDepth = juce::jlimit(-1.0f, 1.0f, controls.filterEnvDepth.load(std::memory_order_relaxed));
    const float nyquistSafe = (float)currentSampleRate * 0.45f;
    const float baseCutoff = controls.cutoffHz.load(std::memory_order_relaxed);
    const float octaves = juce::jlimit(-6.0f, 6.0f, filterMod * 3.0f + filterEnvLevel * envDepth * 4.0f);
    lowPass.setCutoffFrequency(juce::jlimit(50.0f, nyquistSafe,
        baseCutoff * std::pow(2.0f, octaves)));
    lowPass.setResonance(juce::jlimit(0.5f, 4.0f, controls.resonance.load(std::memory_order_relaxed)));
    juce::dsp::AudioBlock<float> block(audio);
    lowPass.process(juce::dsp::ProcessContextReplacing<float>(block));

    // True rearrangeable master FX chain. IDs are read atomically once per block.
    for (int slot=0; slot<6; ++slot) {
        const int effect = controls.masterFx[(size_t)slot].load(std::memory_order_relaxed);
        fxRacks[4].process(effect,audio,controls);
    }
    const float level = controls.masterGain.load(std::memory_order_relaxed) *
        juce::jlimit(0.0f, 2.0f, 1.0f + ampMod * 0.8f);
    // One-shot loop audition is mixed only when the UI has already prepared
    // an immutable snapshot. tryLock never blocks the real-time audio thread.
    if (audio.getNumChannels() > 0) {
        juce::ScopedTryLock previewTryLock(previewLock);
        if (previewTryLock.isLocked()) {
            float* out[2] = { audio.getWritePointer(0),
                audio.getNumChannels() > 1 ? audio.getWritePointer(1) : nullptr };
            previewVoice.process(out, (size_t)juce::jmin(2, audio.getNumChannels()),
                                 (size_t)audio.getNumSamples(), 0.32f);
        }
    }
    audio.applyGain(level);
}

juce::Result LibertyRomplerAudioProcessor::previewSampleZoneLoop(
    const juce::File& wav, int startFrame, int endFrame, int crossfadeFrames) {
    // All file I/O, memory allocations and sample-rate conversion occur on
    // the caller's UI thread; processBlock only mixes the immutable result.
    if (!wav.existsAsFile())
        return juce::Result::fail("Source WAV introuvable.");
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(wav));
    if (!reader || reader->sampleRate <= 0.0 || reader->numChannels < 1)
        return juce::Result::fail("Source WAV non lisible.");
    const int total = liberty::samplezones::loadedFrames(
        reader->lengthInSamples, reader->sampleRate);
    const liberty::loop_v015::Loop loop {startFrame, endFrame, total};
    if (!liberty::loop_v015::isValid(loop, 11))
        return juce::Result::fail("Boucle invalide pour la pr\u00e9\u00e9coute.");
    if (!liberty::loopfade::valid(startFrame, endFrame, crossfadeFrames))
        return juce::Result::fail("Fondu de boucle hors limites.");

    const int channels = juce::jlimit(1, 2, (int)reader->numChannels);
    const int length = endFrame - startFrame;
    const int contextFrames = juce::jmin(
        length, juce::jmax(256, juce::jmax(crossfadeFrames + 128,
                            (int)std::round(reader->sampleRate * 0.06))));
    juce::AudioBuffer<float> decoded(channels, total);
    decoded.clear();
    if (!reader->read(&decoded, 0, total, 0, true, true))
        return juce::Result::fail("Lecture des \u00e9chantillons impossible.");
    std::vector<std::vector<float>> samples((size_t)channels);
    for (int ch = 0; ch < channels; ++ch) {
        const auto* first = decoded.getReadPointer(ch);
        samples[(size_t)ch].assign(first, first + total);
    }

    auto snapshot = liberty::loop_v015::renderSeamPreview(
        samples, loop, contextFrames, crossfadeFrames);
    if (snapshot.channels.empty() || snapshot.channels[0].empty())
        return juce::Result::fail("Pr\u00e9\u00e9coute indisponible.");

    // The source WAV may have a different sample rate from the host. Linear
    // resampling is sufficient for this short audition only (not for exports).
    const double ratio = currentSampleRate / reader->sampleRate;
    if (!std::isfinite(ratio) || ratio < 0.1 || ratio > 16.0)
        return juce::Result::fail("Fr\u00e9quence d'\u00e9chantillonnage incompatible.");
    if (std::abs(ratio - 1.0) > 0.000001) {
        const auto inputFrames = snapshot.channels[0].size();
        const auto required = std::ceil((double)inputFrames * ratio);
        if (!std::isfinite(required) || required > 131072.0 || required < 1.0)
            return juce::Result::fail("Pr\u00e9\u00e9coute trop longue.");
        const auto outputFrames = (size_t)required;
        for (auto& channel : snapshot.channels) {
            std::vector<float> converted(outputFrames);
            for (size_t i = 0; i < outputFrames; ++i) {
                const double position = std::min((double)(inputFrames - 1),
                                                  (double)i / ratio);
                const auto index = (size_t)position;
                const auto next = std::min(index + 1, inputFrames - 1);
                const float blend = (float)(position - (double)index);
                converted[i] = channel[index] * (1.0f - blend)
                               + channel[next] * blend;
            }
            channel.swap(converted);
        }
    }

    // This lock is held only for the pointer swap; the real-time callback
    // uses ScopedTryLock and silently skips one audition block if contended.
    {
        const juce::ScopedLock lock(previewLock);
        previewVoice.stop();
        previewVoice.prepare(nullptr);
        previewSnapshot = std::move(snapshot);
        if (!previewVoice.prepare(&previewSnapshot, 64))
            return juce::Result::fail("Impossible de pr\u00e9parer la pr\u00e9\u00e9coute.");
        previewVoice.start();
    }
    return juce::Result::ok();
}

bool LibertyRomplerAudioProcessor::getSampleZone(const juce::String& bank,
    const juce::String& presetId, int index, liberty::ZoneDescriptor& out) const {
    const auto* preset = library.find(bank, presetId);
    if (!preset || index < 0 || index >= (int)preset->zones.size()) return false;
    out = preset->zones[(size_t)index];
    for (const auto& edit : zoneEdits)
        if (edit.bankId == bank && edit.presetId == presetId && edit.zoneIndex == index) {
            out.lowNote = edit.lowNote;
            out.highNote = edit.highNote;
            out.lowVelocity = edit.lowVelocity;
            out.highVelocity = edit.highVelocity;
            out.loopStart = edit.loopStart;
            out.loopEnd = edit.loopEnd;
            out.loopCrossfadeFrames = edit.loopCrossfadeFrames;
            break;
        }
    return true;
}

juce::Result LibertyRomplerAudioProcessor::setSampleZoneEdit(const SampleZoneEdit& edit) {
    const auto* preset = library.find(edit.bankId, edit.presetId);
    if (!preset || edit.zoneIndex < 0 || edit.zoneIndex >= (int)preset->zones.size())
        return juce::Result::fail("Zone multisample introuvable.");
    const auto mapping = liberty::samplezones::Mapping {
        edit.lowNote, edit.highNote, edit.lowVelocity, edit.highVelocity };
    if (!liberty::samplezones::valid(mapping))
        return juce::Result::fail("Plages de notes ou de velocite incorrectes.");
    const auto& original = preset->zones[(size_t)edit.zoneIndex];
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(original.file));
    if (!reader) return juce::Result::fail("WAV introuvable ou illisible.");
    const auto frames = liberty::samplezones::loadedFrames(reader->lengthInSamples, reader->sampleRate);
    if (!liberty::samplezones::validLoop(edit.loopStart, edit.loopEnd, frames))
        return juce::Result::fail("Points de boucle hors de la duree audio.");
    if (!liberty::loopfade::valid(edit.loopStart, edit.loopEnd, edit.loopCrossfadeFrames))
        return juce::Result::fail("Fondu de boucle hors limites.");
    auto next = zoneEdits;
    next.erase(std::remove_if(next.begin(), next.end(), [&](const SampleZoneEdit& item) {
        return item.bankId == edit.bankId && item.presetId == edit.presetId &&
               item.zoneIndex == edit.zoneIndex;
    }), next.end());
    const bool isOriginal = original.lowNote == edit.lowNote && original.highNote == edit.highNote &&
        original.lowVelocity == edit.lowVelocity && original.highVelocity == edit.highVelocity &&
        original.loopStart == edit.loopStart && original.loopEnd == edit.loopEnd &&
        original.loopCrossfadeFrames == edit.loopCrossfadeFrames;
    if (!isOriginal) {
        if (next.size() >= 128) return juce::Result::fail("Limite de 128 edits de samples.");
        next.push_back(edit);
    }
    auto previous = zoneEdits;
    zoneEdits = std::move(next);
    bool currentlyPlayingSource = false;
    for (const auto& source : layerSources)
        if (source.bankId == edit.bankId && source.presetId == edit.presetId) {
            currentlyPlayingSource = true;
            break;
        }
    if (currentlyPlayingSource) {
        const auto status = reloadLayerSounds(layerSources);
        if (status.failed()) {
            zoneEdits = std::move(previous);
            return status;
        }
    }
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
    return juce::Result::ok();
}

juce::Result LibertyRomplerAudioProcessor::resetSampleZoneEdit(const juce::String& bank,
    const juce::String& presetId, int index) {
    const auto* preset = library.find(bank, presetId);
    if (!preset || index < 0 || index >= (int)preset->zones.size())
        return juce::Result::fail("Zone introuvable.");
    const auto& zone = preset->zones[(size_t)index];
    SampleZoneEdit edit;
    edit.bankId = bank; edit.presetId = presetId; edit.zoneIndex = index;
    edit.lowNote = zone.lowNote; edit.highNote = zone.highNote;
    edit.lowVelocity = zone.lowVelocity; edit.highVelocity = zone.highVelocity;
    edit.loopStart = zone.loopStart; edit.loopEnd = zone.loopEnd;
    edit.loopCrossfadeFrames = zone.loopCrossfadeFrames;
    return setSampleZoneEdit(edit);
}

LibertyRomplerAudioProcessor::LayerSource
LibertyRomplerAudioProcessor::getLayerSource(int index) const {
    return index >= 0 && index < 4 ? layerSources[(size_t)index] : LayerSource {};
}

juce::Result LibertyRomplerAudioProcessor::reloadLayerSounds(
    const std::array<LayerSource, 4>& desired) {
    // Prepare on message thread before replacing any playing sounds.
    juce::ReferenceCountedArray<juce::SynthesiserSound> pending;
    int totalZones = 0;
    for (int i = 0; i < 4; ++i) {
        const auto& source = desired[(size_t)i];
        if (source.bankId.isEmpty() && source.presetId.isEmpty()) continue;
        const auto* preset = library.find(source.bankId, source.presetId);
        if (!preset || preset->zones.empty() || !preset->layers.empty())
            return juce::Result::fail("Source absente ou preset multicouche non pris en charge comme source.");
        totalZones += (int)preset->zones.size();
        if (totalZones > 64)
            return juce::Result::fail("Limite de 64 zones audio pour les quatre couches.");
        for (int zoneIndex = 0; zoneIndex < (int)preset->zones.size(); ++zoneIndex) {
            liberty::ZoneDescriptor zone;
            if (!getSampleZone(source.bankId, source.presetId, zoneIndex, zone))
                return juce::Result::fail("Zone multisample incorrecte.");
            auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(zone.file));
            if (!reader) return juce::Result::fail("Fichier audio illisible : " + zone.file.getFileName());
            auto* sound = new liberty::MappedSampleSound(zone, *reader, controls.layers[(size_t)i], i);
            if (!sound->isLoaded()) {
                delete sound;
                return juce::Result::fail("Sample invalide : " + zone.file.getFileName());
            }
            pending.add(sound);
        }
    }
    // Four instruments share no voices; each has an independent sound pool.
    for (int i=0; i<4; ++i) {
        synths[(size_t)i].allNotesOff(0, false);
        synths[(size_t)i].clearSounds();
    }
    for (auto* sound : pending) {
        auto* mapped = dynamic_cast<liberty::MappedSampleSound*>(sound);
        if (mapped && mapped->getLayerIndex() >= 0 && mapped->getLayerIndex() < 4)
            synths[(size_t)mapped->getLayerIndex()].addSound(sound);
    }
    layerSources = desired;
    return juce::Result::ok();
}

juce::Result LibertyRomplerAudioProcessor::setLayerSource(
    int index, const juce::String& bankId, const juce::String& presetId) {
    if (index < 0 || index >= 4)
        return juce::Result::fail("Indice de couche incorrect.");
    if (bankId.isEmpty() != presetId.isEmpty())
        return juce::Result::fail("Banque et preset doivent etre definis ensemble.");
    auto desired = layerSources;
    desired[(size_t)index] = {bankId, presetId};
    const auto status = reloadLayerSounds(desired);
    if (status.wasOk()) {
        controls.layers[(size_t)index].enabled.store(bankId.isNotEmpty());
        updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
    }
    return status;
}

juce::Result LibertyRomplerAudioProcessor::selectPreset(
    const juce::String& bankId, const juce::String& presetId) {
    const auto* preset = library.find(bankId, presetId);
    if (!preset) return juce::Result::fail("Preset ou banque introuvable.");

    std::array<LayerSource, 4> desired {};
    if (preset->layers.empty()) {
        desired[0] = {bankId, presetId};
    } else {
        for (size_t i = 0; i < preset->layers.size() && i < 4; ++i) {
            const auto& l = preset->layers[i];
            desired[i] = {l.bankId, l.presetId};
        }
    }
    const auto result = reloadLayerSounds(desired);
    if (result.failed()) return result;
    for (int i = 0; i < 4; ++i) {
        auto& ctrl = controls.layers[(size_t)i];
        ctrl.enabled.store(false, std::memory_order_relaxed);
        ctrl.gain.store(i == 0 ? 1.0f : 0.55f, std::memory_order_relaxed);
        ctrl.pan.store(0.0f, std::memory_order_relaxed);
        ctrl.cutoffHz.store(20000.0f, std::memory_order_relaxed);
        ctrl.transpose.store(0, std::memory_order_relaxed);
        ctrl.lowNote.store(0, std::memory_order_relaxed);
        ctrl.highNote.store(127, std::memory_order_relaxed);
        ctrl.lowVelocity.store(1, std::memory_order_relaxed);
        ctrl.highVelocity.store(127, std::memory_order_relaxed);
        if (preset->layers.empty()) {
            ctrl.enabled.store(i == 0, std::memory_order_relaxed);
        } else if ((size_t)i < preset->layers.size()) {
            const auto& def = preset->layers[(size_t)i];
            ctrl.enabled.store(def.enabled, std::memory_order_relaxed);
            ctrl.gain.store(def.gain, std::memory_order_relaxed);
            ctrl.pan.store(def.pan, std::memory_order_relaxed);
            ctrl.cutoffHz.store(def.cutoffHz, std::memory_order_relaxed);
            ctrl.transpose.store(def.transpose, std::memory_order_relaxed);
            ctrl.lowNote.store(def.lowNote, std::memory_order_relaxed);
            ctrl.highNote.store(def.highNote, std::memory_order_relaxed);
            ctrl.lowVelocity.store(def.lowVelocity, std::memory_order_relaxed);
            ctrl.highVelocity.store(def.highVelocity, std::memory_order_relaxed);
        }
    }
    currentBankId = bankId;
    currentPresetId = presetId;
    currentPresetName = preset->name;
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
    return juce::Result::ok();
}

void LibertyRomplerAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    auto state = juce::var(new juce::DynamicObject());
    auto* object = state.getDynamicObject();
    object->setProperty("bank", currentBankId);
    object->setProperty("preset", currentPresetId);
    object->setProperty("morphActive", morphActiveParam->get());
    object->setProperty("morphX", morphXParam->get());
    object->setProperty("morphY", morphYParam->get());
    object->setProperty("morphMidi", morphMidiParam->get());
    object->setProperty("morphMidiX", midiMorphX.load(std::memory_order_relaxed));
    object->setProperty("morphMidiY", midiMorphY.load(std::memory_order_relaxed));
    object->setProperty("gain", controls.masterGain.load());
    object->setProperty("attack", controls.attackSeconds.load());
    object->setProperty("release", controls.releaseSeconds.load());
    object->setProperty("ampDecay", controls.decaySeconds.load());
    object->setProperty("ampSustain", controls.sustainLevel.load());
    object->setProperty("filterEnvAttack", controls.filterAttack.load());
    object->setProperty("filterEnvDecay", controls.filterDecay.load());
    object->setProperty("filterEnvSustain", controls.filterSustain.load());
    object->setProperty("filterEnvRelease", controls.filterRelease.load());
    object->setProperty("filterEnvDepth", controls.filterEnvDepth.load());
    object->setProperty("cutoff", controls.cutoffHz.load());
    object->setProperty("resonance", controls.resonance.load());
    object->setProperty("chorusRate", controls.chorusRate.load());
    object->setProperty("chorusDepth", controls.chorusDepth.load());
    object->setProperty("chorusMix", controls.chorusMix.load());
    object->setProperty("reverbRoom", controls.reverbRoom.load());
    object->setProperty("reverbDamping", controls.reverbDamping.load());
    object->setProperty("reverbMix", controls.reverbMix.load());
    object->setProperty("delayTimeMs", controls.delayTimeMs.load());
    object->setProperty("delayFeedback", controls.delayFeedback.load());
    object->setProperty("delayMix", controls.delayMix.load());
    object->setProperty("distortionDriveDb", controls.distortionDriveDb.load());
    object->setProperty("distortionToneHz", controls.distortionToneHz.load());
    object->setProperty("distortionMix", controls.distortionMix.load());
    object->setProperty("lfoOneHz", controls.lfoOneHz.load());
    object->setProperty("lfoTwoHz", controls.lfoTwoHz.load());
    object->setProperty("lfoOneWave", controls.lfoOneWave.load());
    object->setProperty("lfoTwoWave", controls.lfoTwoWave.load());
    object->setProperty("eqLowDb", controls.eqLowDb.load());
    object->setProperty("eqMidDb", controls.eqMidDb.load());
    object->setProperty("eqHighDb", controls.eqHighDb.load());
    object->setProperty("compThresholdDb", controls.compThresholdDb.load());
    object->setProperty("compRatio", controls.compRatio.load());
    object->setProperty("compMakeupDb", controls.compMakeupDb.load());
    for (int slot = 0; slot < 4; ++slot) {
        const auto suffix = juce::String(slot);
        object->setProperty(juce::Identifier("src" + suffix), controls.matrix[(size_t)slot].source.load());
        object->setProperty(juce::Identifier("dst" + suffix), controls.matrix[(size_t)slot].target.load());
        object->setProperty(juce::Identifier("amt" + suffix), controls.matrix[(size_t)slot].amount.load());
    }
    // v0.6: independent layer sources and DSP settings, including custom stacks.
    juce::Array<juce::var> layerArray;
    for (int i = 0; i < 4; ++i) {
        const auto& source = layerSources[(size_t)i];
        const auto& layer = controls.layers[(size_t)i];
        juce::var entry(new juce::DynamicObject());
        auto* l = entry.getDynamicObject();
        l->setProperty("bank", source.bankId);
        l->setProperty("preset", source.presetId);
        l->setProperty("enabled", layer.enabled.load());
        l->setProperty("gain", layer.gain.load());
        l->setProperty("pan", layer.pan.load());
        l->setProperty("transpose", layer.transpose.load());
        l->setProperty("cutoff", layer.cutoffHz.load());
        l->setProperty("lowNote", layer.lowNote.load());
        l->setProperty("highNote", layer.highNote.load());
        l->setProperty("lowVelocity", layer.lowVelocity.load());
        l->setProperty("highVelocity", layer.highVelocity.load());
        layerArray.add(entry);
    }
    object->setProperty("layerState", layerArray);
    // Session-only, non-destructive editing of original bank zones.
    juce::Array<juce::var> zoneState;
    for (const auto& edit : zoneEdits) {
        juce::var entry(new juce::DynamicObject());
        auto* e = entry.getDynamicObject();
        e->setProperty("bank", edit.bankId);
        e->setProperty("preset", edit.presetId);
        e->setProperty("index", edit.zoneIndex);
        e->setProperty("lowNote", edit.lowNote);
        e->setProperty("highNote", edit.highNote);
        e->setProperty("lowVelocity", edit.lowVelocity);
        e->setProperty("highVelocity", edit.highVelocity);
        e->setProperty("loopStart", edit.loopStart);
        e->setProperty("loopEnd", edit.loopEnd);
        e->setProperty("loopCrossfadeFrames", edit.loopCrossfadeFrames);
        zoneState.add(entry);
    }
    object->setProperty("sampleZoneEdits", zoneState);

    juce::Array<juce::var> masterSlots, layerSlots;
    for (const auto& slot : controls.masterFx) masterSlots.add(slot.load());
    for (const auto& row : controls.layerFx)
        for (const auto& slot : row) layerSlots.add(slot.load());
    object->setProperty("routingMaster", masterSlots);
    object->setProperty("routingLayers", layerSlots);
    // v0.8 Arpeggiator: optional settings to preserve legacy DAW sessions.
    object->setProperty("arpEnabled", arpSettings.enabled.load());
    object->setProperty("arpMode", arpSettings.mode.load());
    object->setProperty("arpOctaves", arpSettings.octaves.load());
    object->setProperty("arpSteps", arpSettings.steps.load());
    object->setProperty("arpDivision", arpSettings.division.load());
    object->setProperty("arpSwing", arpSettings.swing.load());
    object->setProperty("arpGate", arpSettings.gate.load());
    juce::Array<juce::var> arpPattern;
    for (const auto& step : arpSettings.pattern) {
        juce::var entry(new juce::DynamicObject());
        auto* o = entry.getDynamicObject();
        o->setProperty("velocity", step.velocity.load());
        o->setProperty("transpose", step.transpose.load());
        o->setProperty("enabled", step.enabled.load());
        arpPattern.add(entry);
    }
    object->setProperty("arpPattern", arpPattern);

    const auto json = juce::JSON::toString(state);
    dest.replaceAll(json.toRawUTF8(), (size_t)json.getNumBytesAsUTF8());
}
void LibertyRomplerAudioProcessor::setStateInformation(const void* data, int size) {
    if (size <= 0 || !data) return;
    const auto json = juce::String::fromUTF8((const char*)data, size);
    const auto state = juce::JSON::parse(json);
    if (auto* o = state.getDynamicObject()) {
        // Explicit legacy bypass when loading sessions created before MORPH.
        static_cast<juce::AudioProcessorParameter*>(morphActiveParam)->setValue(
            o->hasProperty("morphActive") && (bool)o->getProperty("morphActive") ? 1.0f : 0.0f);
        static_cast<juce::AudioProcessorParameter*>(morphMidiParam)->setValue(
            o->hasProperty("morphMidi") && (bool)o->getProperty("morphMidi") ? 1.0f : 0.0f);
        const auto safeMorph = [](const juce::var& value) {
            return liberty::morph_v036::normalized((float)(double)value);
        };
        static_cast<juce::AudioProcessorParameter*>(morphXParam)->setValue(
            o->hasProperty("morphX") ? safeMorph(o->getProperty("morphX")) : 0.5f);
        static_cast<juce::AudioProcessorParameter*>(morphYParam)->setValue(
            o->hasProperty("morphY") ? safeMorph(o->getProperty("morphY")) : 0.5f);
        midiMorphX.store(o->hasProperty("morphMidiX")
            ? safeMorph(o->getProperty("morphMidiX")) : 0.5f, std::memory_order_relaxed);
        midiMorphY.store(o->hasProperty("morphMidiY")
            ? safeMorph(o->getProperty("morphMidiY")) : 0.5f, std::memory_order_relaxed);
        controls.masterGain = juce::jlimit(0.0f, 1.5f, (float)(double)o->getProperty("gain"));
        controls.attackSeconds = juce::jlimit(0.0f, 2.0f, (float)(double)o->getProperty("attack"));
        controls.releaseSeconds = juce::jlimit(0.02f, 5.0f, (float)(double)o->getProperty("release"));
        // Compatible with DAW projects saved before filter controls existed.
        controls.cutoffHz = o->hasProperty("cutoff") ?
            juce::jlimit(80.0f, 20000.0f, (float)(double)o->getProperty("cutoff")) : 20000.0f;
        controls.resonance = o->hasProperty("resonance") ?
            juce::jlimit(0.5f, 4.0f, (float)(double)o->getProperty("resonance")) : 0.707f;
        // Missing fields retain their defaults for compatibility with older sessions.
        auto restore = [o](const char* key, std::atomic<float>& value, float low, float high) {
            if (o->hasProperty(key)) value.store(juce::jlimit(low, high, (float)(double)o->getProperty(key)));
        };
        restore("chorusRate", controls.chorusRate, 0.05f, 8.0f);
        restore("chorusDepth", controls.chorusDepth, 0.0f, 1.0f);
        restore("chorusMix", controls.chorusMix, 0.0f, 1.0f);
        restore("reverbRoom", controls.reverbRoom, 0.0f, 1.0f);
        restore("reverbDamping", controls.reverbDamping, 0.0f, 1.0f);
        restore("reverbMix", controls.reverbMix, 0.0f, 1.0f);
        restore("delayTimeMs", controls.delayTimeMs, 60.0f, 1500.0f);
        restore("delayFeedback", controls.delayFeedback, 0.0f, 0.85f);
        restore("delayMix", controls.delayMix, 0.0f, 1.0f);
        restore("distortionDriveDb", controls.distortionDriveDb, 0.0f, 30.0f);
        restore("distortionToneHz", controls.distortionToneHz, 500.0f, 18000.0f);
        restore("distortionMix", controls.distortionMix, 0.0f, 1.0f);
        restore("ampDecay", controls.decaySeconds, 0.01f, 5.0f);
        restore("ampSustain", controls.sustainLevel, 0.0f, 1.0f);
        restore("filterEnvAttack", controls.filterAttack, 0.001f, 5.0f);
        restore("filterEnvDecay", controls.filterDecay, 0.01f, 5.0f);
        restore("filterEnvSustain", controls.filterSustain, 0.0f, 1.0f);
        restore("filterEnvRelease", controls.filterRelease, 0.01f, 8.0f);
        restore("filterEnvDepth", controls.filterEnvDepth, -1.0f, 1.0f);
        restore("eqLowDb", controls.eqLowDb, -12.0f, 12.0f);
        restore("eqMidDb", controls.eqMidDb, -12.0f, 12.0f);
        restore("eqHighDb", controls.eqHighDb, -12.0f, 12.0f);
        restore("compThresholdDb", controls.compThresholdDb, -48.0f, 0.0f);
        restore("compRatio", controls.compRatio, 1.0f, 12.0f);
        restore("compMakeupDb", controls.compMakeupDb, 0.0f, 18.0f);
        if (o->hasProperty("lfoOneWave")) controls.lfoOneWave.store(
            juce::jlimit(0, 3, (int)o->getProperty("lfoOneWave")));
        if (o->hasProperty("lfoTwoWave")) controls.lfoTwoWave.store(
            juce::jlimit(0, 3, (int)o->getProperty("lfoTwoWave")));
        restore("lfoOneHz", controls.lfoOneHz, 0.05f, 12.0f);
        restore("lfoTwoHz", controls.lfoTwoHz, 0.05f, 12.0f);
        for (int slot = 0; slot < 4; ++slot) {
            const auto suffix = juce::String(slot);
            const juce::Identifier src("src" + suffix), dst("dst" + suffix), amt("amt" + suffix);
            if (o->hasProperty(src)) controls.matrix[(size_t)slot].source.store(
                juce::jlimit(0, 3, (int)o->getProperty(src)));
            if (o->hasProperty(dst)) controls.matrix[(size_t)slot].target.store(
                juce::jlimit(0, 2, (int)o->getProperty(dst)));
            if (o->hasProperty(amt)) controls.matrix[(size_t)slot].amount.store(
                juce::jlimit(-1.0f, 1.0f, (float)(double)o->getProperty(amt)));
        }
        // Reject malformed routing payloads; old projects keep historical FX order.
        const auto masterRoutingState = o->getProperty("routingMaster");
        if (auto* list=masterRoutingState.getArray()) {
            if (list->size()==6) {
                std::array<bool,7> used {{false,false,false,false,false,false,false}};
                bool valid=true;
                for (const auto& value : *list) {
                    const int id=(int)value;
                    if (id<1 || id>6 || used[(size_t)id]) valid=false;
                    else used[(size_t)id]=true;
                }
                if (valid)
                    for (int i=0;i<6;++i) controls.masterFx[(size_t)i].store((int)list->getReference(i));
            }
        }
        const auto layerRoutingState = o->getProperty("routingLayers");
        if (auto* list=layerRoutingState.getArray()) {
            if (list->size()==8)
                for (int i=0;i<8;++i)
                    controls.layerFx[(size_t)i/2][(size_t)i%2].store(juce::jlimit(0,6,(int)list->getReference(i)));
        }
        // In old sessions no arp data exists; always default to disabled.
        arpSettings.enabled.store(o->hasProperty("arpEnabled") && (bool)o->getProperty("arpEnabled"));
        if (o->hasProperty("arpMode")) arpSettings.mode.store(juce::jlimit(0,3,(int)o->getProperty("arpMode")));
        if (o->hasProperty("arpOctaves")) arpSettings.octaves.store(juce::jlimit(1,4,(int)o->getProperty("arpOctaves")));
        if (o->hasProperty("arpSteps")) arpSettings.steps.store(juce::jlimit(1,32,(int)o->getProperty("arpSteps")));
        if (o->hasProperty("arpDivision")) arpSettings.division.store(juce::jlimit(0,3,(int)o->getProperty("arpDivision")));
        restore("arpSwing", arpSettings.swing, 0.0f, 0.75f);
        restore("arpGate", arpSettings.gate, 0.05f, 0.98f);
        const auto arpPatternState = o->getProperty("arpPattern");
        if (auto* list = arpPatternState.getArray()) {
            if (list->size()==32) for(int i=0;i<32;++i) {
                if (const auto* step=list->getReference(i).getDynamicObject()) {
                    auto& dst=arpSettings.pattern[(size_t)i];
                    dst.velocity.store(juce::jlimit(1,127,(int)step->getProperty("velocity")));
                    dst.transpose.store(juce::jlimit(-24,24,(int)step->getProperty("transpose")));
                    dst.enabled.store((bool)step->getProperty("enabled"));
                }
            }
        }
        // Restore zone overlays *before* any samples are loaded.
        zoneEdits.clear();
        const auto rawZoneState = o->getProperty("sampleZoneEdits");
        if (auto* entries = rawZoneState.getArray()) {
            if (entries->size() <= 128) for (const auto& value : *entries) {
                const auto* entry = value.getDynamicObject();
                if (!entry) continue;
                SampleZoneEdit edit;
                edit.bankId = entry->getProperty("bank").toString();
                edit.presetId = entry->getProperty("preset").toString();
                edit.zoneIndex = (int)entry->getProperty("index");
                edit.lowNote = (int)entry->getProperty("lowNote");
                edit.highNote = (int)entry->getProperty("highNote");
                edit.lowVelocity = (int)entry->getProperty("lowVelocity");
                edit.highVelocity = (int)entry->getProperty("highVelocity");
                edit.loopStart = (int)entry->getProperty("loopStart");
                edit.loopEnd = (int)entry->getProperty("loopEnd");
                edit.loopCrossfadeFrames = entry->hasProperty("loopCrossfadeFrames")
                    ? (int)entry->getProperty("loopCrossfadeFrames") : 0;
                const auto* zonePreset = library.find(edit.bankId, edit.presetId);
                const auto mapping = liberty::samplezones::Mapping {
                    edit.lowNote, edit.highNote, edit.lowVelocity, edit.highVelocity };
                if (!zonePreset || edit.zoneIndex < 0 ||
                    edit.zoneIndex >= (int)zonePreset->zones.size() ||
                    !liberty::samplezones::valid(mapping)) continue;
                const auto& original = zonePreset->zones[(size_t)edit.zoneIndex];
                auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(original.file));
                if (!reader || !liberty::samplezones::validLoop(edit.loopStart, edit.loopEnd,
                    liberty::samplezones::loadedFrames(reader->lengthInSamples, reader->sampleRate)) ||
                    !liberty::loopfade::valid(edit.loopStart, edit.loopEnd,
                                             edit.loopCrossfadeFrames)) continue;
                const auto duplicate = std::any_of(zoneEdits.begin(), zoneEdits.end(),
                    [&](const SampleZoneEdit& prior) {
                        return prior.bankId == edit.bankId && prior.presetId == edit.presetId &&
                               prior.zoneIndex == edit.zoneIndex;
                    });
                if (!duplicate) zoneEdits.push_back(std::move(edit));
            }
        }
        selectPreset(o->getProperty("bank").toString(), o->getProperty("preset").toString());
        // Old DAW sessions had no layerState property: slot A remains as before.
        const auto rawLayers = o->getProperty("layerState");
        if (auto* entries = rawLayers.getArray()) {
            if (entries->size() == 4) {
                std::array<LayerSource, 4> desired {};
                bool valid = true;
                for (int i = 0; i < 4; ++i) {
                    auto* l = entries->getReference(i).getDynamicObject();
                    if (!l) { valid = false; break; }
                    desired[(size_t)i] = {l->getProperty("bank").toString(),
                                          l->getProperty("preset").toString()};
                }
                if (valid && reloadLayerSounds(desired).wasOk()) {
                    for (int i = 0; i < 4; ++i) {
                        auto* l = entries->getReference(i).getDynamicObject();
                        auto& layer = controls.layers[(size_t)i];
                        layer.enabled.store((bool)l->getProperty("enabled"));
                        layer.gain.store(juce::jlimit(0.0f, 1.5f, (float)(double)l->getProperty("gain")));
                        layer.pan.store(juce::jlimit(-1.0f, 1.0f, (float)(double)l->getProperty("pan")));
                        layer.transpose.store(juce::jlimit(-24, 24, (int)l->getProperty("transpose")));
                        layer.cutoffHz.store(juce::jlimit(80.0f, 20000.0f, (float)(double)l->getProperty("cutoff")));
                        layer.lowNote.store(juce::jlimit(0, 127, (int)l->getProperty("lowNote")));
                        layer.highNote.store(juce::jlimit(0, 127, (int)l->getProperty("highNote")));
                        layer.lowVelocity.store(juce::jlimit(1, 127, (int)l->getProperty("lowVelocity")));
                        layer.highVelocity.store(juce::jlimit(1, 127, (int)l->getProperty("highVelocity")));
                    }
                }
            }
        }
    }
}
juce::AudioProcessorEditor* LibertyRomplerAudioProcessor::createEditor() {
    return new LibertyRomplerAudioProcessorEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new LibertyRomplerAudioProcessor();
}
