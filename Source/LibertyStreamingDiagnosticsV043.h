#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cstdint>

// Read-only UI: polls aggregate atomics on the message thread, never on audio.
// Embedded in REGLAGES: keeps the 11-tab navigation and host state unchanged.
class LibertyStreamingDiagnosticsV043 final : public juce::Component,
                                             private juce::Timer {
public:
    explicit LibertyStreamingDiagnosticsV043(LibertyRomplerAudioProcessor& p)
        : processor(p), copyButton("COPIER LE RAPPORT") {
        addAndMakeVisible(copyButton);
        copyButton.onClick = [this] {
            const auto d = processor.getStreamingDiagnostics();
            const auto& c = d.totals;
            juce::String report = "Liberty ROMpler v0.43 - Streaming disque\n";
            report << "Zones longues : " << juce::String((int)d.streamedZones) << "\n";
            report << "Cache hits : " << number(c.cacheHits) << "\n";
            report << "Cache misses : " << number(c.cacheMisses) << "\n";
            report << fr(u8"Pages demand\u00e9es : ") << number(c.queuedPages) << "\n";
            report << "Pages perdues : " << number(c.droppedPages) << "\n";
            report << fr(u8"Pages publi\u00e9es : ") << number(c.publishedPages) << "\n";
            report << fr(u8"Urgentes demand\u00e9es : ") << number(c.urgentQueued) << "\n";
            report << "Urgentes servies : " << number(c.urgentServiced) << "\n";
            report << fr(u8"Urgentes perdues : ") << number(c.urgentDropped) << "\n";
            report << fr(u8"Erreurs de d\u00e9codage : ") << number(c.decoderErrors) << "\n";
            report << fr(u8"Blocs sous-aliment\u00e9s : ") << number(c.underflowBlocks) << "\n";
            juce::SystemClipboard::copyTextToClipboard(report);
        };
        startTimerHz(4);
    }

    static juce::String fr(const char* text) { return juce::String::fromUTF8(text); }

    void resized() override {
        copyButton.setBounds(juce::jmax(0, getWidth() - 178), 10, 164, 29);
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour::fromRGB(15, 27, 49));
        g.fillRoundedRectangle(bounds, 9.0f);
        g.setColour(juce::Colour::fromRGB(48, 76, 110));
        g.drawRoundedRectangle(bounds.reduced(0.6f), 9.0f, 1.2f);
        auto label = [&g](const juce::String& s, juce::Rectangle<int> rect,
                         juce::Colour colour, float pointSize, bool bold = false) {
            g.setColour(colour);
            g.setFont(juce::Font(juce::FontOptions(pointSize,
                bold ? juce::Font::bold : juce::Font::plain)));
            g.drawFittedText(s, rect, juce::Justification::centredLeft, 1, 0.8f);
        };
        const auto white = juce::Colour::fromRGB(228, 237, 250);
        const auto blue = juce::Colour::fromRGB(70, 213, 240);
        const auto quiet = juce::Colour::fromRGB(159, 180, 204);
        const auto warning = juce::Colour::fromRGB(255, 183, 92);
        label("STREAMING DISQUE  /  DIAGNOSTICS", {16, 13,
              juce::jmax(90, getWidth()-215), 25}, blue, 14.0f, true);
        juce::String state;
        if (!current.hasStreamedZones()) state = fr(u8"Aucune zone longue charg\u00e9e");
        else if (current.hasIncidents()) state = fr(u8"INCIDENTS D\u00c9TECT\u00c9S : examiner le rapport");
        else if (!current.hasMeasurements()) state = fr(u8"Zones charg\u00e9es - en attente de lecture");
        else state = fr(u8"Aucun incident enregistr\u00e9 (tests audio encore n\u00e9cessaires)");
        label(state, {16, 42, juce::jmax(90, getWidth()-32), 21},
              current.hasIncidents() ? warning : quiet, 11.0f);
        g.setColour(juce::Colour::fromRGB(48, 76, 110));
        g.drawHorizontalLine(70, 17.0f, (float)getWidth()-17.0f);
        const auto& c = current.totals;
        const int width = juce::jmax(0, (getWidth()-42)/2);
        const int left = 17, right = 25+width;
        const auto metric = [&](int x, int line, const juce::String& name,
                                const juce::String& value, bool alert = false) {
            const int y = 78+line*22;
            const auto valueWidth = juce::jmin(110, juce::jmax(65, width/3));
            label(name, {x, y, juce::jmax(0,width-valueWidth-3), 19}, quiet, 11.0f);
            g.setColour(alert ? warning : white);
            g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
            g.drawFittedText(value, {x+width-valueWidth, y, valueWidth, 19},
                             juce::Justification::centredRight, 1, 0.8f);
        };
        metric(left, 0, "Zones longues", juce::String((int)current.streamedZones));
        metric(left, 1, "Cache hits / misses", number(c.cacheHits)+" / "+number(c.cacheMisses));
        metric(left, 2, "Taux de cache", current.hasMeasurements()
               ? juce::String(current.hitRatePercent(), 1)+" %" : "--");
        metric(left, 3, fr(u8"Pages publi\u00e9es / perdues"),
               number(c.publishedPages)+" / "+number(c.droppedPages), c.droppedPages>0);
        metric(right, 0, "Demandes urgentes", number(c.urgentQueued));
        metric(right, 1, "Urgentes servies / perdues",
               number(c.urgentServiced)+" / "+number(c.urgentDropped), c.urgentDropped>0);
        metric(right, 2, fr(u8"Erreurs de d\u00e9codage"), number(c.decoderErrors), c.decoderErrors>0);
        metric(right, 3, fr(u8"Blocs audio sous-aliment\u00e9s"), number(c.underflowBlocks), c.underflowBlocks>0);
        if (getHeight() >= 205)
            label(fr(u8"Valeurs cumul\u00e9es depuis le chargement des zones - rafra\u00eechissement 4 Hz"),
                  {17, getHeight()-30, juce::jmax(0,getWidth()-34), 20}, quiet, 10.0f);
    }

private:
    static juce::String number(std::uint64_t count) {
        return juce::String(static_cast<juce::int64>(count));
    }
    void timerCallback() override {
        if (isShowing()) {
            current = processor.getStreamingDiagnostics();
            repaint();
        }
    }
    LibertyRomplerAudioProcessor& processor;
    liberty::stream_v043::Diagnostics current;
    juce::TextButton copyButton;
};
