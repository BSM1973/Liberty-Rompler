#pragma once
// Liberty MORPH v0.36 - real XY controller, dedicated page (no overlap).
// Host automation: active, X, Y, MIDI mode. MIDI source CC16=X, CC17=Y.
#include <JuceHeader.h>
#include <cmath>
#include "PluginProcessor.h"
#include "MorphWeightsV036.h"

class LibertyMorphPadV036 final : public juce::Component, private juce::Timer
{
public:
    explicit LibertyMorphPadV036(LibertyRomplerAudioProcessor& owner)
        : processor(owner)
    {
        addAndMakeVisible(activeButton);
        addAndMakeVisible(midiButton);
        activeButton.setClickingTogglesState(true);
        midiButton.setClickingTogglesState(true);
        activeButton.onClick = [this] {
            auto& parameter = processor.getMorphActive();
            parameter.beginChangeGesture();
            parameter.setValueNotifyingHost(activeButton.getToggleState() ? 1.0f : 0.0f);
            parameter.endChangeGesture();
            syncControls();
        };
        midiButton.onClick = [this] {
            auto& parameter = processor.getMorphMidiMode();
            parameter.beginChangeGesture();
            parameter.setValueNotifyingHost(midiButton.getToggleState() ? 1.0f : 0.0f);
            parameter.endChangeGesture();
            syncControls();
        };
        syncControls();
        startTimerHz(24);
    }

    ~LibertyMorphPadV036() override
    {
        stopTimer();
        finishGesture();
    }

    void resized() override
    {
        activeButton.setBounds(15, 11, 145, 32);
        midiButton.setBounds(171, 11, 184, 32);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff151e2b));
        const auto pad = padArea();
        if (pad.isEmpty()) return;

        g.setColour(juce::Colour(0xff0f1928));
        g.fillRoundedRectangle(pad, 11.0f);
        g.setColour(juce::Colour(0xff486279));
        g.drawRoundedRectangle(pad, 11.0f, 1.5f);
        g.setColour(juce::Colour(0xff3a5265));
        g.drawLine(pad.getCentreX(), pad.getY(), pad.getCentreX(),
                   pad.getBottom(), 1.0f);
        g.drawLine(pad.getX(), pad.getCentreY(), pad.getRight(),
                   pad.getCentreY(), 1.0f);

        const juce::Colour colours[4] = {
            juce::Colour(0xff68d9cc), juce::Colour(0xffe9ba74),
            juce::Colour(0xff988ae5), juce::Colour(0xff77c9a2)
        };
        const auto drawName = [&g](const juce::String& text,
                                   juce::Rectangle<float> area,
                                   juce::Colour colour) {
            g.setColour(colour);
            g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
            g.drawFittedText(text, area.toNearestInt(),
                             juce::Justification::centred, 1);
        };
        drawName("A", {pad.getX()+8.0f, pad.getY()+8.0f, 32.0f, 24.0f}, colours[0]);
        drawName("B", {pad.getRight()-40.0f, pad.getY()+8.0f, 32.0f, 24.0f}, colours[1]);
        drawName("C", {pad.getX()+8.0f, pad.getBottom()-32.0f, 32.0f, 24.0f}, colours[2]);
        drawName("D", {pad.getRight()-40.0f, pad.getBottom()-32.0f, 32.0f, 24.0f}, colours[3]);

        const float x = processor.morphDisplayX(), y = processor.morphDisplayY();
        const float markerX = pad.getX()+x*pad.getWidth();
        const float markerY = pad.getY()+y*pad.getHeight();
        g.setColour(juce::Colour(0xff65d8d0).withAlpha(0.2f));
        g.fillEllipse(markerX-18.0f, markerY-18.0f, 36.0f, 36.0f);
        g.setColour(juce::Colour(0xff65d8d0));
        g.drawEllipse(markerX-11.0f, markerY-11.0f, 22.0f, 22.0f, 2.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(markerX-4.0f, markerY-4.0f, 8.0f, 8.0f);

        const auto weights = liberty::morph_v036::weights(x, y);
        const auto values = juce::String("A ") + juce::String(weights[0]*100.0f, 1) +
             "%    B " + juce::String(weights[1]*100.0f, 1) +
             "%    C " + juce::String(weights[2]*100.0f, 1) +
             "%    D " + juce::String(weights[3]*100.0f, 1) + "%";
        g.setColour(juce::Colour(0xffe3edf2));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawFittedText(values,
            {15, getHeight()-49, getWidth()-30, 20},
            juce::Justification::centred, 1, 0.85f);
        g.setColour(juce::Colour(0xffa8b7c6));
        g.drawFittedText(juce::String::fromUTF8(
            u8"Glisser le point : fondu entre 4 couches. Double-clic : centre. MIDI : CC16 X / CC17 Y."),
            {15, getHeight()-26, getWidth()-30, 19},
            juce::Justification::centred, 1, 0.82f);
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (!event.mods.isLeftButtonDown() || !padArea().contains(event.position))
            return;
        if (processor.getMorphMidiMode().get()) {
            auto& midi = processor.getMorphMidiMode();
            midi.beginChangeGesture();
            midi.setValueNotifyingHost(0.0f);
            midi.endChangeGesture();
        }
        processor.getMorphX().beginChangeGesture();
        processor.getMorphY().beginChangeGesture();
        dragging = true;
        if (event.getNumberOfClicks() >= 2) {
            updateFromPoint(padArea().getCentre());
            finishGesture();
        } else updateFromPoint(event.position);
        syncControls();
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (dragging) updateFromPoint(event.position);
    }

    void mouseUp(const juce::MouseEvent&) override { finishGesture(); }

private:
    LibertyRomplerAudioProcessor& processor;
    juce::TextButton activeButton {"MORPH OFF"};
    juce::TextButton midiButton {"MIDI CC16/17 OFF"};
    bool dragging = false;
    float previousX = -1.0f, previousY = -1.0f;

    juce::Rectangle<float> padArea() const noexcept
    {
        const float side = juce::jmax(0.0f, juce::jmin(
            400.0f, juce::jmin((float)getWidth()-72.0f,
                              (float)getHeight()-139.0f)));
        return {(getWidth()-side)*0.5f, (getHeight()-side)*0.5f+6.0f,
                side, side};
    }

    void updateFromPoint(juce::Point<float> point)
    {
        const auto pad = padArea();
        if (pad.getWidth() <= 1.0f || pad.getHeight() <= 1.0f) return;
        const auto x = liberty::morph_v036::normalized(
            (point.x-pad.getX())/pad.getWidth());
        const auto y = liberty::morph_v036::normalized(
            (point.y-pad.getY())/pad.getHeight());
        processor.getMorphX().setValueNotifyingHost(x);
        processor.getMorphY().setValueNotifyingHost(y);
        repaint();
    }

    void finishGesture()
    {
        if (!dragging) return;
        dragging = false;
        processor.getMorphX().endChangeGesture();
        processor.getMorphY().endChangeGesture();
    }

    void syncControls()
    {
        const bool active = processor.getMorphActive().get();
        const bool midi = processor.getMorphMidiMode().get();
        activeButton.setToggleState(active, juce::dontSendNotification);
        midiButton.setToggleState(midi, juce::dontSendNotification);
        activeButton.setButtonText(active ? "MORPH ON" : "MORPH OFF");
        midiButton.setButtonText(midi ? "MIDI CC16/17 ON" : "MIDI CC16/17 OFF");
    }

    void timerCallback() override
    {
        if (!isVisible()) return;
        syncControls();
        const float x = processor.morphDisplayX(), y = processor.morphDisplayY();
        if (std::abs(x-previousX)>0.0001f || std::abs(y-previousY)>0.0001f) {
            previousX = x; previousY = y;
            repaint();
        }
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibertyMorphPadV036)
};
