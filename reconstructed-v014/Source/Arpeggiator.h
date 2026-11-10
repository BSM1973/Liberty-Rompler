#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty {

struct ArpStep {
    std::atomic<int> velocity {100};   // 1-127, scales held-note velocity
    std::atomic<int> transpose {0};   // -24..24 semitones
    std::atomic<bool> enabled {true}; // rest when disabled
};

struct ArpControls {
    std::atomic<bool> enabled {false};  // legacy projects are unchanged
    std::atomic<int> mode {0};         // 0 Up, 1 Down, 2 Random, 3 Chord
    std::atomic<int> octaves {1};      // 1..4
    std::atomic<int> steps {16};       // 1..32
    std::atomic<int> division {2};     // 0=1/4 1=1/8 2=1/16 3=1/32
    std::atomic<float> swing {0.0f};   // 0..0.75
    std::atomic<float> gate {0.78f};   // 0.05..0.98 of current step
    std::array<ArpStep, 32> pattern;
    std::atomic<int> currentStep {-1};
};

// All playback state belongs to the audio thread. Parameters are atomics and
// the output MIDI buffer is preallocated in prepare() (never alloc in normal playback).
class Arpeggiator final {
public:
    void prepare(double sampleRate) {
        rate = juce::jmax(8000.0, sampleRate);
        rendered = 0;
        nextAt = -1.0;
        offAt = -1.0;
        patternTick = 0;
        melodicTick = 0;
        wasEnabled = false;
        wasHostPlaying = false;
        lastPpq = 0.0;
        lastHostBlockLength = 0;
        holds.fill(0);
        heldVelocity.fill(100);
        outputActive.fill(false);
        prepared.ensureSize(65536);
    }

    void process(juce::MidiBuffer& incoming, int numSamples, ArpControls& control,
                 const juce::AudioPlayHead::PositionInfo* position) {
        const bool enabled = control.enabled.load(std::memory_order_relaxed);
        const int samples = juce::jmax(0, numSamples);
        control.currentStep.store(enabled ? control.currentStep.load() : -1, std::memory_order_relaxed);
        prepared.clear();
        if (!enabled) {
            if (wasEnabled) {
                turnOffVoices(0);
                // Previously generated notes must be released, but normal input
                // MIDI remains intact when the user bypasses the arp.
                for (const auto e : prepared) incoming.addEvent(e.getMessage(), 0);
                holds.fill(0);
                nextAt = offAt = -1.0;
                control.currentStep.store(-1);
            }
            wasEnabled = false;
            rendered += samples;
            return;
        }
        if (!wasEnabled) {
            holds.fill(0); outputActive.fill(false);
            nextAt = offAt = -1.0;
            patternTick = melodicTick = 0;
            control.currentStep.store(-1);
        }
        wasEnabled = true;
        const double bpm = position && position->getBpm().hasValue()
                           ? juce::jlimit(20.0, 400.0, *position->getBpm()) : 120.0;
        const double stepBeats = 1.0 / std::pow(2.0, juce::jlimit(0, 3, control.division.load()));
        stepSamples = rate * 60.0 / bpm * stepBeats;
        const bool transportPlaying = position && position->getIsPlaying();
        const auto ppq = position ? position->getPpqPosition() : juce::Optional<double>{};
        const bool hostSynced = transportPlaying && ppq.hasValue();
        // Seek and loop detection. Host clock is only sampled once per audio
        // block, and the actual emitted MIDI events have precise sample offsets.
        if (hostSynced && wasHostPlaying) {
            const double expected = lastPpq + (double)lastHostBlockLength * bpm / (60.0 * rate);
            if (std::abs(*ppq - expected) > 0.12) {
                turnOffVoices(0);
                // Re-arm held keys on the new host position, even if no new
                // note-on arrives after a loop/seek. Never leave notes silent.
                const double newBeat = 1.0 / std::pow(2.0, juce::jlimit(0,3,control.division.load()));
                const double nextGrid = std::ceil((*ppq - 1.0e-8)/newBeat)*newBeat;
                nextAt = heldCount()>0 ? (double)rendered + juce::jmax(0.0,(nextGrid-*ppq)*rate*60.0/bpm) : -1.0;
                patternTick = melodicTick = 0;
                control.currentStep.store(-1);
            }
        }
        lastPpq = hostSynced ? *ppq : 0.0;
        lastHostBlockLength = samples;
        wasHostPlaying = hostSynced;
        // Use an internal sample cursor when the host is stopped/unavailable.
        // When playing, initial notes align to host PPQ grid (including swing).
        const double start = (double)rendered;
        const double end = start + samples;
        // Group simultaneous notes, so chord/descending modes see all notes
        // of a MIDI chord BEFORE triggering an arpeggio step at that sample.
        auto it = incoming.begin();
        while (it != incoming.end()) {
            const int offset = juce::jlimit(0, samples, (*it).samplePosition);
            const double moment = start + offset;
            emitUntil(moment, false, start, samples, control);
            while (it != incoming.end() && juce::jlimit(0, samples, (*it).samplePosition) == offset) {
                const auto msg = (*it).getMessage();
                if (msg.isNoteOn()) {
                    const int note = msg.getNoteNumber();
                    const bool wasEmpty = heldCount() == 0;
                    holds[(size_t)note] = juce::jmin(255, holds[(size_t)note] + 1);
                    heldVelocity[(size_t)note] = juce::jlimit(1,127,juce::roundToInt(msg.getVelocity()*127.0f));
                    if (wasEmpty) {
                        patternTick = melodicTick = 0;
                        nextAt = moment;
                        if (hostSynced) {
                            const double atPpq = *ppq + offset * bpm / (60.0 * rate);
                            const double nextGrid = std::ceil((atPpq - 1.0e-8) / stepBeats) * stepBeats;
                            nextAt = moment + juce::jmax(0.0, (nextGrid - atPpq)*rate*60.0/bpm);
                        }
                    }
                } else if (msg.isNoteOff()) {
                    const int note = msg.getNoteNumber();
                    holds[(size_t)note] = juce::jmax(0, holds[(size_t)note]-1);
                    if (heldCount()==0) {
                        turnOffVoices(offset);
                        nextAt=-1.0;
                        control.currentStep.store(-1);
                    }
                } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
                    turnOffVoices(offset);
                    holds.fill(0);
                    nextAt=-1.0;
                    control.currentStep.store(-1);
                    prepared.addEvent(msg,offset);
                } else prepared.addEvent(msg,offset);
                ++it;
            }
            emitUntil(moment, true, start, samples, control);
        }
        emitUntil(end, false, start, samples, control);
        incoming.swapWith(prepared);
        rendered += samples;
    }

private:
    juce::MidiBuffer prepared;
    std::array<int, 128> holds {};
    std::array<int, 128> heldVelocity {};
    std::array<bool, 128> outputActive {};
    std::int64_t rendered = 0;
    double rate = 44100.0, stepSamples = 5512.5;
    double nextAt = -1.0, offAt = -1.0;
    double lastPpq = 0.0;
    int lastHostBlockLength = 0;
    std::uint32_t randomState = 0x8aecc5f7;
    std::uint64_t patternTick = 0, melodicTick = 0;
    bool wasEnabled = false, wasHostPlaying = false;

    int heldCount() const {
        int count = 0;
        for (const auto n : holds) count += n > 0 ? 1 : 0;
        return count;
    }
    void turnOffVoices(int offset) {
        for (int n = 0; n < 128; ++n) {
            if (outputActive[(size_t)n]) {
                prepared.addEvent(juce::MidiMessage::noteOff(1, n), offset);
                outputActive[(size_t)n] = false;
            }
        }
        offAt = -1.0;
    }
    void trigger(double at, double blockStart, int blockSamples, ArpControls& c) {
        const int stepCount = juce::jlimit(1, 32, c.steps.load(std::memory_order_relaxed));
        const int index = (int)(patternTick % (std::uint64_t)stepCount);
        c.currentStep.store(index, std::memory_order_relaxed);
        const int sampleOffset = juce::jlimit(0, juce::jmax(0, blockSamples-1),
                                             (int)std::round(at - blockStart));
        turnOffVoices(sampleOffset); // always terminate previous gate first
        const auto& step = c.pattern[(size_t)index];
        const int noteCount = heldCount();
        if (step.enabled.load(std::memory_order_relaxed) && noteCount > 0) {
            std::array<int, 128> notes {};
            int count = 0;
            for (int i = 0; i < 128; ++i) if (holds[(size_t)i] > 0) notes[(size_t)count++] = i;
            const int octaves = juce::jlimit(1, 4, c.octaves.load(std::memory_order_relaxed));
            const int mode = juce::jlimit(0, 3, c.mode.load(std::memory_order_relaxed));
            const int transpose = juce::jlimit(-24, 24, step.transpose.load(std::memory_order_relaxed));
            const float velocityScaling = (float)juce::jlimit(1, 127, step.velocity.load(std::memory_order_relaxed)) / 127.0f;
            if (mode == 3) {
                const int octave = (int)(melodicTick % (std::uint64_t)octaves);
                for (int i = 0; i < count; ++i) {
                    const int base = notes[(size_t)i];
                    const int pitch = base + octave * 12;
                    if (pitch <= 127) {
                        // Velocity comes from the actual held note, even after octave shifts.
                        const int adjustedPitch = pitch + transpose;
                        if (adjustedPitch >= 0 && adjustedPitch < 128 && !outputActive[(size_t)adjustedPitch]) {
                            const int vel = juce::jlimit(1,127,juce::roundToInt(heldVelocity[(size_t)base]*velocityScaling));
                            prepared.addEvent(juce::MidiMessage::noteOn(1, adjustedPitch, (juce::uint8)vel), sampleOffset);
                            outputActive[(size_t)adjustedPitch] = true;
                        }
                    }
                }
            } else {
                const int total = count * octaves;
                int idx;
                if (mode == 2) {
                    randomState ^= randomState << 13;
                    randomState ^= randomState >> 17;
                    randomState ^= randomState << 5;
                    idx = (int)(randomState % (std::uint32_t)total);
                } else idx = (int)(melodicTick % (std::uint64_t)total);
                const int pitchIndex = idx % count;
                const int octave = idx / count;
                const int base = mode == 1 ? notes[(size_t)(count-1-pitchIndex)] : notes[(size_t)pitchIndex];
                const int transposed = base + (mode == 1 ? octaves - 1 - octave : octave)*12 + transpose;
                if (transposed >= 0 && transposed < 128) {
                    const int vel = juce::jlimit(1,127,juce::roundToInt(heldVelocity[(size_t)base]*velocityScaling));
                    prepared.addEvent(juce::MidiMessage::noteOn(1, transposed, (juce::uint8)vel), sampleOffset);
                    outputActive[(size_t)transposed] = true;
                }
            }
            const double swing = (double)juce::jlimit(0.0f, 0.75f, c.swing.load(std::memory_order_relaxed));
            const double actualLength = stepSamples * (1.0 + ((patternTick % 2) ? -1.0 : 1.0) * swing * 0.5);
            offAt = at + juce::jmax(1.0, actualLength * (double)juce::jlimit(0.05f,0.98f,c.gate.load()));
            ++melodicTick;
        }
        const double swing = (double)juce::jlimit(0.0f, 0.75f, c.swing.load(std::memory_order_relaxed));
        const double duration = stepSamples * (1.0 + ((patternTick % 2) ? -1.0 : 1.0) * swing * 0.5);
        nextAt = at + juce::jmax(1.0, duration);
        ++patternTick;
    }
    void emitUntil(double limit, bool inclusive, double blockStart,
                   int blockSamples, ArpControls& c) {
        if (heldCount() == 0) return;
        // Bound the amount of generated MIDI in case a host jumps the timeline.
        for (int safety = 0; safety < 4096; ++safety) {
            const double next = nextAt >= 0.0 ? nextAt : 1.0e99;
            const double off = offAt >= 0.0 ? offAt : 1.0e99;
            const double at = juce::jmin(next, off);
            if (!std::isfinite(at) || (inclusive ? at > limit : at >= limit) || at >= 1.0e98) break;
            const int offset = juce::jlimit(0, juce::jmax(0,blockSamples-1), (int)std::round(at-blockStart));
            if (off <= next) turnOffVoices(offset);
            else trigger(at, blockStart, blockSamples, c);
        }
    }
};
} // namespace liberty
