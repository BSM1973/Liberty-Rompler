#pragma once
#include <JuceHeader.h>
#include "Arpeggiator.h"
#include <utility>
#include <cstdint>
#include "ArpPatternTools.h"

// Liberty Rompler v0.9: 32 steps, six starter patterns and musical variations.
// All edits affect the pre-existing atomic ArpControls fields, which are saved
// with the DAW state. Buttons only appear inside the grid component.
class LibertyArpStepGrid final : public juce::Component, private juce::Timer {
public:
    explicit LibertyArpStepGrid(liberty::ArpControls& c) : controls(c) {
        setName("Grille arpege 32 pas");
        setMouseCursor(juce::MouseCursor::PointingHandCursor);

        patternMenu.setTextWhenNothingSelected(fr(u8"MOD\u00c8LES"));
        patternMenu.addItem("DROIT", 1);
        patternMenu.addItem("HOUSE", 2);
        patternMenu.addItem("TRANCE", 3);
        patternMenu.addItem("BASSE", 4);
        patternMenu.addItem(fr(u8"SYNCOPE"), 5);
        patternMenu.addItem(fr(u8"MONT\u00c9E 32 PAS"), 6);
        patternMenu.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff223247));
        patternMenu.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        patternMenu.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff385064));
        patternMenu.onChange = [this] {
            const int id = patternMenu.getSelectedId();
            if (id > 0) liberty::arp_patterns::applyTemplate(controls, id);
            repaint();
        };
        addAndMakeVisible(patternMenu);

        configure(reverseButton, "INVERSER", [this] { liberty::arp_patterns::reverse(controls); repaint(); });
        configure(rotateButton, fr(u8"D\u00c9CALER"), [this] { liberty::arp_patterns::rotate(controls); repaint(); });
        configure(humanizeButton, "HUMANISER", [this] {
            liberty::arp_patterns::humanize(controls, ++humanizeSeed * 0x9e3779b9u);
            repaint();
        });
        configure(resetButton, fr(u8"R\u00c9INITIALISER"), [this] {
            liberty::arp_patterns::reset(controls);
            patternMenu.setSelectedId(0, juce::dontSendNotification);
            repaint();
        });
        startTimerHz(12);
    }

    void resized() override {
        auto top = getLocalBounds().reduced(7, 9).removeFromTop(35);
        const int gap = 6;
        const int cellWidth = juce::jmax(1, (top.getWidth() - 4 * gap) / 5);
        juce::Component* controlsToPlace[] = {
            &patternMenu, &reverseButton, &rotateButton, &humanizeButton, &resetButton
        };
        int x = top.getX();
        for (int i = 0; i < 5; ++i) {
            const int width = (i == 4) ? juce::jmax(1, top.getRight() - x) : cellWidth;
            controlsToPlace[i]->setBounds(x, top.getY(), width, top.getHeight());
            x += width + gap;
        }
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff121c2b));
        const int active = juce::jlimit(1, 32, controls.steps.load());
        const int playing = controls.currentStep.load();
        const auto outer = gridBounds();
        if (outer.isEmpty()) return;
        const float cellWidth = (float) outer.getWidth() / 16.0f;
        const float cellHeight = (float) outer.getHeight() / 2.0f;
        for (int i = 0; i < 32; ++i) {
            const float cx = (float) outer.getX() + (i % 16) * cellWidth;
            const float cy = (float) outer.getY() + (i / 16) * cellHeight;
            const auto r = juce::Rectangle<float>(cx + 2, cy + 2,
                juce::jmax(0.0f, cellWidth - 4), juce::jmax(0.0f, cellHeight - 4));
            if (r.isEmpty()) continue;
            const bool inRange = i < active;
            const bool on = controls.pattern[(size_t) i].enabled.load();
            const bool highlighted = inRange && i == playing;
            g.setColour(highlighted ? juce::Colour(0xff315968) : juce::Colour(0xff1b2838));
            g.fillRoundedRectangle(r, 5.0f);
            g.setColour(highlighted ? juce::Colour(0xff68d9cc) : juce::Colour(0xff354558));
            g.drawRoundedRectangle(r, 5.0f, highlighted ? 1.6f : 1.0f);
            const float alpha = inRange ? 1.0f : 0.28f;
            const float height = r.getHeight();
            const float numberH = height >= 72.0f ? 22.0f : 15.0f;
            const float labelH = height >= 72.0f ? 19.0f : 13.0f;
            const float labelY = r.getBottom() - labelH - 3.0f;
            const float meterY = r.getY() + numberH + 3.0f;
            const float meterH = juce::jmax(0.0f, labelY - meterY - 3.0f);

            g.setColour(juce::Colours::white.withAlpha(alpha));
            g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
            g.drawFittedText(juce::String(i + 1),
                juce::Rectangle<int>((int) r.getX() + 2, (int) r.getY(),
                                     juce::jmax(1, (int) r.getWidth() - 4), (int) numberH),
                juce::Justification::centred, 1, 0.8f);
            const auto meter = juce::Rectangle<float>(r.getX() + 7.0f, meterY,
                juce::jmax(1.0f, r.getWidth() - 14.0f), meterH);
            if (meterH >= 2.0f) {
                g.setColour(juce::Colour(0xff0b1420));
                g.fillRoundedRectangle(meter, 2.5f);
                const float velocity = (float) juce::jlimit(1, 127,
                    controls.pattern[(size_t) i].velocity.load()) / 127.0f;
                g.setColour(on ? juce::Colour(0xff68d9cc).withAlpha(alpha)
                               : juce::Colour(0xff6d7482).withAlpha(alpha));
                if (on) g.fillRoundedRectangle(juce::Rectangle<float>(meter.getX(),
                    meter.getBottom() - meter.getHeight() * velocity,
                    meter.getWidth(), meter.getHeight() * velocity), 2.0f);
            }
            g.setColour(juce::Colour(0xffc5cfdb).withAlpha(alpha));
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            const int transpose = controls.pattern[(size_t) i].transpose.load();
            const juce::String title = on ? ((transpose > 0 ? "+" : "") + juce::String(transpose)) : "OFF";
            g.drawFittedText(title,
                juce::Rectangle<int>((int) r.getX() + 2, (int) labelY,
                                     juce::jmax(1, (int) r.getWidth() - 4), (int) labelH),
                juce::Justification::centred, 1, 0.8f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override { edit(e, true); }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (!e.mods.isPopupMenu()) edit(e, false);
    }

private:
    liberty::ArpControls& controls;
    juce::ComboBox patternMenu;
    juce::TextButton reverseButton, rotateButton, humanizeButton, resetButton;
    std::uint32_t humanizeSeed = 0x6c8e9cf1u;

    static juce::String fr(const char* text) { return juce::String::fromUTF8(text); }
    template <typename Callback>
    void configure(juce::TextButton& button, const juce::String& title, Callback&& callback) {
        button.setButtonText(title);
        button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff223247));
        button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        button.onClick = std::forward<Callback>(callback);
        addAndMakeVisible(button);
    }
    juce::Rectangle<int> gridBounds() const {
        return getLocalBounds().reduced(7, 9).withTrimmedTop(43);
    }
    void timerCallback() override { if (isVisible()) repaint(); }

    void edit(const juce::MouseEvent& e, bool isDown) {
        const auto bounds = gridBounds();
        if (!bounds.contains(e.getPosition()) || bounds.getWidth() < 16 || bounds.getHeight() < 2)
            return;
        const int col = juce::jlimit(0, 15, (e.x - bounds.getX()) * 16 / bounds.getWidth());
        const int row = juce::jlimit(0, 1, (e.y - bounds.getY()) * 2 / bounds.getHeight());
        const int index = row * 16 + col;
        if (index >= controls.steps.load()) return;
        auto& step = controls.pattern[(size_t) index];
        if (e.mods.isPopupMenu()) {
            if (isDown) step.enabled.store(!step.enabled.load());
            repaint();
            return;
        }
        const int halfHeight = juce::jmax(1, bounds.getHeight() / 2);
        const int relativeY = (e.y - bounds.getY()) - row * halfHeight;
        if (e.mods.isShiftDown())
            step.transpose.store(juce::jlimit(-24, 24, 24 - relativeY * 48 / halfHeight));
        else
            step.velocity.store(juce::jlimit(1, 127, 127 - relativeY * 126 / halfHeight));
        step.enabled.store(true);
        repaint();
    }
};
