#pragma once
// Liberty ROMpler v0.15: JUCE sample loop toolbar, ready to mount in SAMPLES.
// This component NEVER opens an audio device or changes the DAW state itself.
// The owning editor supplies current loop coordinates and the callbacks below.

#include "LoopFrameInputV015.h"
#include "LoopToolbarLayoutV015.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <functional>
#include <string>

namespace liberty::loop_v015
{
class LoopEditorPanelV015 final : public juce::Component
{
public:
    explicit LoopEditorPanelV015 (Loop loop)
        : pending (loop)
    {
        setComponentID ("liberty-loop-editor-v015");

        startLabel.setText (juce::String::fromUTF8 (u8"D\u00c9BUT"), juce::dontSendNotification);
        endLabel.setText (juce::String::fromUTF8 ("FIN"), juce::dontSendNotification);
        startLabel.setJustificationType (juce::Justification::centredLeft);
        endLabel.setJustificationType (juce::Justification::centredLeft);
        for (auto* label : {&startLabel, &endLabel})
        {
            label->setFont (juce::FontOptions (12.0f));
            label->setColour (juce::Label::textColourId, juce::Colours::white);
        }

        for (auto* field : {&startFrame, &endFrame})
        {
            field->setInputRestrictions (19, "0123456789");
            field->setJustification (juce::Justification::centred);
            field->setColour (juce::TextEditor::backgroundColourId,
                              juce::Colour::fromRGB (20, 29, 45));
            field->setColour (juce::TextEditor::textColourId, juce::Colours::white);
            field->setColour (juce::TextEditor::outlineColourId,
                              juce::Colour::fromRGB (91, 114, 151));
        }

        startMinus.setButtonText ("-");
        endMinus.setButtonText ("-");
        startPlus.setButtonText ("+");
        endPlus.setButtonText ("+");
        preview.setButtonText (juce::String::fromUTF8 (u8"PR\u00c9\u00c9COUTE"));
        apply.setButtonText ("APPLIQUER");
        cancel.setButtonText ("ANNULER");

        for (auto* button : {&startMinus, &endMinus, &startPlus, &endPlus,
                             &preview, &apply, &cancel})
        {
            button->setColour (juce::TextButton::buttonColourId,
                               juce::Colour::fromRGB (36, 52, 76));
            button->setColour (juce::TextButton::textColourOffId,
                               juce::Colours::white);
        }

        startMinus.onClick = [this] { step (Marker::begin, -stepFrames); };
        startPlus.onClick = [this] { step (Marker::begin, stepFrames); };
        endMinus.onClick = [this] { step (Marker::end, -stepFrames); };
        endPlus.onClick = [this] { step (Marker::end, stepFrames); };
        startFrame.onReturnKey = [this] { commitTyped (Marker::begin); };
        endFrame.onReturnKey = [this] { commitTyped (Marker::end); };
        startFrame.onEscapeKey = [this] { refreshFields(); };
        endFrame.onEscapeKey = [this] { refreshFields(); };

        preview.onClick = [this]
        {
            if (onPreviewRequested && pending.valid())
                onPreviewRequested (pending.value());
        };
        apply.onClick = [this]
        {
            if (onApplyRequested && pending.valid()
                && onApplyRequested (pending.value()))
            {
                pending.markApplied();
                refreshFields();
            }
        };
        cancel.onClick = [this]
        {
            pending.cancelPending();
            refreshFields();
            notifyPending();
        };

        for (auto* component : controls())
            addAndMakeVisible (*component);
        refreshFields();
    }

    // The parent must update this when changing the selected sample zone.
    void setLoop (Loop r)
    {
        pending = PendingFrameEdit (r);
        refreshFields();
    }

    const Loop& getPendingLoop() const noexcept { return pending.value(); }
    bool hasChanges() const noexcept { return pending.changed(); }
    void setStepFrames (Frame step) noexcept { stepFrames = std::max (Frame {1}, step); }

    // Called on the UI thread. The host should build the seam preview outside
    // the audio callback, then hand it off using its own thread-safe mechanism.
    std::function<void (const Loop&)> onPreviewRequested;
    std::function<bool (const Loop&)> onApplyRequested;
    std::function<void (const Loop&)> onPendingChanged;

    void resized() override
    {
        const auto layout = makeLoopToolbarLayout (getWidth(), getHeight());
        const auto rects = layout.all();
        const auto components = controls();
        for (std::size_t i = 0; i < rects.size(); ++i)
        {
            auto* component = components[i];
            const auto& box = rects[i];
            const bool show = layout.fits && box.visible();
            component->setVisible (show);
            if (show)
                component->setBounds (box.x, box.y, box.width, box.height);
        }
    }

private:
    void step (Marker marker, Frame distance)
    {
        pending.step (marker, distance);
        refreshFields();
        notifyPending();
    }

    void commitTyped (Marker marker)
    {
        auto& editor = marker == Marker::begin ? startFrame : endFrame;
        const auto entered = editor.getText().toStdString();
        if (pending.setTyped (marker, entered))
            notifyPending();
        refreshFields();
    }

    void notifyPending()
    {
        if (onPendingChanged)
            onPendingChanged (pending.value());
    }

    void refreshFields()
    {
        startFrame.setText (juce::String (static_cast<juce::int64> (
                                pending.value().begin)), false);
        endFrame.setText (juce::String (static_cast<juce::int64> (
                              pending.value().end)), false);
    }

    std::array<juce::Component*, 11> controls() noexcept
    {
        return {&startLabel, &startMinus, &startFrame, &startPlus,
                &endLabel, &endMinus, &endFrame, &endPlus,
                &preview, &apply, &cancel};
    }

    PendingFrameEdit pending;
    Frame stepFrames = 1;
    juce::Label startLabel, endLabel;
    juce::TextEditor startFrame, endFrame;
    juce::TextButton startMinus, startPlus, endMinus, endPlus;
    juce::TextButton preview, apply, cancel;
};
} // namespace liberty::loop_v015
