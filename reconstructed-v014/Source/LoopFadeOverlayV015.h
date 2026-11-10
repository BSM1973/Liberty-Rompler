#pragma once
// Liberty ROMpler v0.15: non-interactive loop-crossfade overlay for the JUCE
// waveform component. Host should keep it aligned with zoom and scroll.

#include "LoopEditorV015.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

namespace liberty::loop_v015
{
class LoopFadeOverlayV015 final : public juce::Component
{
public:
    LoopFadeOverlayV015()
    {
        setComponentID ("liberty-loop-fade-overlay-v015");
        setInterceptsMouseClicks (false, false);
        setOpaque (false);
    }

    // Call from the message thread when the selected WAV, zoom or loop changes.
    void setState (Loop nextLoop, Frame nextFadeFrames, Viewport nextViewport)
    {
        loop = nextLoop;
        fadeFrames = std::max (Frame {0}, nextFadeFrames);
        viewport = nextViewport;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        if (! isValid (loop) || fadeFrames == 0 || getWidth() <= 0 || getHeight() <= 0)
            return;

        auto visible = viewport;
        visible.left = 0.0;
        visible.width = static_cast<double> (getWidth());
        const auto bands = crossfadeBands (loop, fadeFrames, visible);

        const auto drawBand = [&] (VisibleBand b, juce::Colour shade,
                                  juce::Colour line)
        {
            if (! b.visible || ! std::isfinite (b.x) || ! std::isfinite (b.width))
                return;
            const auto x = static_cast<float> (b.x);
            const auto width = static_cast<float> (b.width);
            g.setColour (shade);
            g.fillRect (juce::Rectangle<float> (x, 0.0f, width,
                                                static_cast<float> (getHeight())));
            g.setColour (line);
            g.drawVerticalLine (static_cast<int> (std::round (x)),
                                0.0f, static_cast<float> (getHeight()));
            g.drawVerticalLine (static_cast<int> (std::round (x + width)),
                                0.0f, static_cast<float> (getHeight()));
        };

        const auto purple = juce::Colour::fromRGB (181, 126, 237);
        drawBand (bands.tail, purple.withAlpha (0.19f), purple.withAlpha (0.75f));
        drawBand (bands.head, juce::Colour::fromRGB (100, 176, 236).withAlpha (0.17f),
                  juce::Colour::fromRGB (100, 176, 236).withAlpha (0.7f));
    }

private:
    Loop loop {};
    Frame fadeFrames = 0;
    Viewport viewport {};
};
} // namespace liberty::loop_v015
