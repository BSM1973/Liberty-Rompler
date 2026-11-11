#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LoopCrossfade.h"
#include "ZeroCrossSnap.h"
#include "WaveformZoom.h"
#include "LoopEditorV015.h"
#include "LoopSeamSearchV018.h"
#include "LoopPairedSeamV019.h"
#include "LoopPairedNudgeV020.h"
#include "LoopDragV021.h"
#include "LoopFocusV022.h"
#include "LoopCursorZoomV023.h"
#include "LoopPanV024.h"
#include "LoopWheelPanV025.h"
#include "LoopCenterViewV026.h"
#include "LoopViewHistoryV027.h"
#include "LoopMarkerFocusV028.h"
#include "LoopMarkerDetailV029.h"
#include "LoopTimingV030.h"
#include "LoopMarkerTimesV031.h"
#include "LoopFadeRatioV032.h"
#include "LoopCoverageV033.h"
#include "LoopSegmentsV034.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

// Non-destructive editor: DAW session overlays are applied by the processor.
// WAV files and bank manifests are never rewritten by this view.
class LibertySampleZoneView final : public juce::Component,
                                    private juce::ListBoxModel,
                                    private juce::ChangeListener {
public:
    explicit LibertySampleZoneView(LibertyRomplerAudioProcessor& owner)
        : processor(owner), thumbnail(256, formats, thumbnailCache) {
        formats.registerBasicFormats();
        thumbnail.addChangeListener(this);
        list.setModel(this);
        list.setRowHeight(52);
        list.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff121c2b));
        list.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(list);
        for (auto* button : {&mappingMode, &loopMode, &apply, &reset,
                             &snapStartButton, &snapEndButton, &cancelButton, &fitWaveButton}) {
            addAndMakeVisible(*button);
            button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff243e50));
            button->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe3edf2));
        }
        mappingMode.setButtonText("MAPPING");
        loopMode.setButtonText("BOUCLES");
        loopMode.setTooltip(juce::String::fromUTF8(
            u8"Maj + glisser la zone color\u00e9e pour d\u00e9placer toute la boucle."
            u8" Option + double-clic : cadrer la boucle."
            u8" Commande + double-clic : centrer la vue."
            u8" Maj + double-clic : revenir au cadrage pr\u00e9c\u00e9dent."));
        apply.setButtonText("APPLIQUER");
        reset.setButtonText("RESTAURER");
        snapStartButton.setButtonText(juce::String::fromUTF8(u8"Z\u00c9RO D\u00c9BUT"));
        snapEndButton.setButtonText(juce::String::fromUTF8(u8"Z\u00c9RO FIN"));
        cancelButton.setButtonText("ANNULER");
        fitWaveButton.setButtonText("TOUT");
        fitWaveButton.setTooltip("Afficher la totalite de la forme d'onde");
        fitWaveButton.onClick = [this] { zoomSlider.setValue(1.0); };
        addAndMakeVisible(autoSnap);
        autoSnap.setButtonText(juce::String::fromUTF8(u8"AUTO Z\u00c9RO"));
        autoSnap.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffd5e8e9));
        autoSnap.setTooltip("Calage automatique au passage par zero apres un glissement de marqueur");
        for (auto* label : {&zoomCaption, &positionCaption}) {
            addAndMakeVisible(*label);
            label->setColour(juce::Label::textColourId, juce::Colour(0xffc0d9e5));
            label->setFont(juce::Font(juce::FontOptions(10.5f)));
        }
        zoomCaption.setText("ZOOM", juce::dontSendNotification);
        zoomCaption.setTooltip(juce::String::fromUTF8(
            u8"Option + molette sur la forme d'onde : zoom sur le point vis\u00e9."));
        positionCaption.setText("POSITION", juce::dontSendNotification);
        positionCaption.setTooltip(juce::String::fromUTF8(
            u8"Option + glisser la forme d'onde : d\u00e9placer la vue sans toucher aux marqueurs."
            u8" Maj + molette : navigation horizontale rapide."));
        addAndMakeVisible(zoomSlider);
        zoomSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        zoomSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 20);
        zoomSlider.setRange(1.0, liberty::wavezoom::maxZoom, 0.0);
        zoomSlider.setTextValueSuffix("x");
        zoomSlider.setValue(1.0, juce::dontSendNotification);
        zoomSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff63dccc));
        zoomSlider.onValueChange = [this] { updateZoom(); };
        addAndMakeVisible(positionSlider);
        positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        positionSlider.setRange(0.0, 1.0, 0.0);
        positionSlider.setValue(0.0, juce::dontSendNotification);
        positionSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff63dccc));
        positionSlider.onValueChange = [this] { updatePosition(); };
        snapStartButton.setTooltip("Clic : z\u00e9ro. Option : SAUT MIN. Maj + Option : PAIRE (dur\u00e9e fixe). Commande + clic : centrer D\u00c9BUT. Commande + Option : ZOOM D\u00c9TAIL.");
        snapEndButton.setTooltip("Clic : z\u00e9ro. Option : SAUT MIN. Maj + Option : PAIRE (dur\u00e9e fixe). Commande + clic : centrer FIN. Commande + Option : ZOOM D\u00c9TAIL.");
        cancelButton.setTooltip("Annuler les modifications non appliquees de cette zone");
        snapStartButton.onClick = [this] {
            const auto modifiers = juce::ModifierKeys::getCurrentModifiers();
            if (modifiers.isAltDown() && modifiers.isShiftDown())
                optimizeLoopPair();
            else if (modifiers.isAltDown()) {
                if (modifiers.isCommandDown())
                    focusLoopMarkerDetail(true);
                else
                    optimizeLoopMarker(true);
            }
            else if (modifiers.isCommandDown() && !modifiers.isShiftDown())
                focusLoopMarker(true);
            else
                snapLoopMarker(true);
        };
        snapEndButton.onClick = [this] {
            const auto modifiers = juce::ModifierKeys::getCurrentModifiers();
            if (modifiers.isAltDown() && modifiers.isShiftDown())
                optimizeLoopPair();
            else if (modifiers.isAltDown()) {
                if (modifiers.isCommandDown())
                    focusLoopMarkerDetail(false);
                else
                    optimizeLoopMarker(false);
            }
            else if (modifiers.isCommandDown() && !modifiers.isShiftDown())
                focusLoopMarker(false);
            else
                snapLoopMarker(false);
        };
        cancelButton.onClick = [this] {
            if (selected < 0 || !dirty) return;
            selectedRowsChanged(selected);
            status = juce::String::fromUTF8(u8"Modifications annul\u00e9es");
            updateFeedback();
        };
        mappingMode.onClick = [this] { showLoops = false; updateLayout(); repaint(); };
        loopMode.onClick = [this] { showLoops = true; updateLayout(); repaint(); };
        apply.onClick = [this] { applyEdits(); };
        reset.onClick = [this] {
            if (const auto* row = currentRow()) {
                const auto result = processor.resetSampleZoneEdit(row->bankId, row->presetId, row->index);
                refresh();
                status = result.wasOk() ? "Zone restauree dans le projet." : result.getErrorMessage();
                updateFeedback();
            }
        };
        for (int i = 0; i < 7; ++i) {
            auto& slider = sliders[(size_t)i];
            auto& label = labels[(size_t)i];
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
            slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff59cdbb));
            slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe4edf4));
            slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            slider.setRange(i == 2 || i == 3 ? 1 : 0, 127, 1);
            label.setColour(juce::Label::textColourId, juce::Colour(0xffbed0dc));
            label.setFont(juce::Font(juce::FontOptions(11.0f)));
            label.setText(i == 0 ? "NOTE DE" : i == 1 ? "NOTE A" :
                          i == 2 ? "VEL DE" : i == 3 ? "VEL A" :
                          i == 4 ? "DEBUT (FRAME)" : i == 5 ? "FIN (FRAME)" :
                          "FONDU (FRAMES)", juce::dontSendNotification);
            slider.onValueChange = [this, i] { reconcile(i); };
            if (i == 4 || i == 5)
                slider.onDragEnd = [this, i] {
                    if (showLoops && autoSnap.getToggleState()) snapLoopMarker(i == 4);
                };
            addAndMakeVisible(slider);
            addAndMakeVisible(label);
        }
        for (int i = 0; i < 4; ++i) {
            auto& button = frameNudges[(size_t)i];
            addAndMakeVisible(button);
            button.setButtonText(i % 2 == 0 ? "-1" : "+1");
            button.setTooltip(i < 2
                ? "D\u00e9but : clic 1 frame, Option + clic = 10 frames; Maj = boucle 1f; Maj+Option = boucle 10f"
                : "Fin : clic 1 frame, Option + clic = 10 frames; Maj = boucle 1f; Maj+Option = boucle 10f");
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff243e50));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe3edf2));
            button.onClick = [this, i] {
                const auto modifiers = juce::ModifierKeys::getCurrentModifiers();
                const int distance = modifiers.isAltDown() ? 10 : 1;
                const int delta = (i % 2 == 0 ? -1 : 1) * distance;
                if (modifiers.isShiftDown())
                    nudgeLoopPair(delta);
                else
                    nudgeLoopMarker(i < 2, delta);
            };
        }
        addAndMakeVisible(previewButton);
        previewButton.setButtonText(juce::String::fromUTF8(u8"PR\u00c9\u00c9COUTE"));
        previewButton.setTooltip(
            "Clic : B avec fondu. Maj : A sans fondu. Option : longue (250 ms)."
            " Mesure BRUT : saut d'amplitude, pas une analyse des clics.");
        previewButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff243e50));
        previewButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe3edf2));
        previewButton.onClick = [this] {
            const auto keys = juce::ModifierKeys::getCurrentModifiers();
            previewSelectedLoop(keys.isShiftDown(), keys.isAltDown());
        };
        addAndMakeVisible(feedback);
        feedback.setColour(juce::Label::textColourId, juce::Colour(0xffa8dcca));
        feedback.setFont(juce::Font(juce::FontOptions(10.0f)));
        feedback.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(loopEnabled);
        loopEnabled.setButtonText(juce::String::fromUTF8(u8"Boucle activ\u00e9e"));
        loopEnabled.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffd5e8e9));
        loopEnabled.onClick = [this] {
            if (loopEnabled.getToggleState() && !hasLoopValues()) {
                loading = true;
                sliders[4].setValue(0, juce::dontSendNotification);
                sliders[5].setValue(juce::jmax(12, sampleFrames), juce::dontSendNotification);
                sliders[6].setValue(0, juce::dontSendNotification);
                loading = false;
            }
            dirty = true;
            updateLayout();
            updateFeedback();
            repaint();
        };
        updateLayout();
    }

    ~LibertySampleZoneView() override {
        thumbnail.removeChangeListener(this);
        list.setModel(nullptr);
    }

    void refresh() {
        const auto previous = currentRow();
        const auto previousBank = previous ? previous->bankId : juce::String();
        const auto previousPreset = previous ? previous->presetId : juce::String();
        const auto previousIndex = previous ? previous->index : -1;
        const auto& library = processor.getLibrary();
        rows.clear();
        // Follow actual active layer routing, including user-assigned instruments.
        for (int layer = 0; layer < 4; ++layer) {
            const auto source = processor.getLayerSource(layer);
            const auto* preset = library.find(source.bankId, source.presetId);
            if (!preset) continue;
            for (int zoneIndex = 0; zoneIndex < (int)preset->zones.size(); ++zoneIndex) {
                liberty::ZoneDescriptor zone;
                if (processor.getSampleZone(source.bankId, source.presetId, zoneIndex, zone))
                    rows.push_back({zone, source.bankId, source.presetId, zoneIndex,
                                    "C" + juce::String(layer + 1) + "  /  " + preset->name});
            }
        }
        list.updateContent();
        int wanted = rows.empty() ? -1 : 0;
        for (int i = 0; i < (int)rows.size(); ++i)
            if (rows[(size_t)i].bankId == previousBank &&
                rows[(size_t)i].presetId == previousPreset &&
                rows[(size_t)i].index == previousIndex) { wanted = i; break; }
        selected = -1;
        list.deselectAllRows();
        if (wanted >= 0) list.selectRow(wanted);
        else { thumbnail.clear(); updateLayout(); repaint(); }
    }

    void resized() override { updateLayout(); }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().reduced(5);
        g.setColour(juce::Colour(0xff142131));
        g.fillRoundedRectangle(b.toFloat(), 9.0f);
        g.setColour(juce::Colour(0xff3a5668));
        g.drawRoundedRectangle(b.toFloat(), 9.0f, 1.0f);
        drawText(g, "SAMPLES  /  MAPPING & BOUCLES",
                 {b.getX() + 15, b.getY() + 7, b.getWidth() - 30, 23}, 15, true);
        drawText(g, juce::String::fromUTF8(u8"Sources WAV et zones de v\u00e9locit\u00e9"),
                 {b.getX() + 18, b.getY() + 33, 320, 16}, 11, false);
        const auto* row = currentRow();
        auto panel = rightArea();
        if (panel.isEmpty()) return;
        g.setColour(juce::Colour(0xff0b1522));
        g.fillRoundedRectangle(panel.toFloat(), 6.0f);
        if (!row) {
            drawText(g, juce::String::fromUTF8(u8"Choisissez une zone WAV \u00e0 gauche"),
                     panel.reduced(15), 13, false);
            return;
        }
        auto filename = panel.withTrimmedTop(31).withHeight(23).reduced(11, 0);
        drawText(g, row->zone.file.getFileName() + "  |  ROOT " + juce::String(row->zone.rootNote),
                 filename, 12, true);
        if (!showLoops) {
            drawNoteMap(g);
            drawVelocityLayers(g);
            drawText(g, juce::String::fromUTF8(u8"Glisser les bords de la zone pour modifier les notes"),
                     mapArea.translated(0, mapArea.getHeight() + 2).withHeight(13), 10, false);
        } else {
            drawWaveform(g);
            // Reuse the existing 14px hint line, so there is no new
            // component or text overlap in the compact loop editor.
            const auto timing = loopTiming();
            const auto hint = timing.valid
                ? (juce::String("BOUCLE ") + juce::String(timing.durationFrames)
                   + " frames | " + juce::String(timing.durationMs, 2) + " ms")
                : juce::String::fromUTF8(
                    u8"Glisser les marqueurs ou r\u00e9gler le fondu");
            drawText(g, hint,
                     waveArea.translated(0, waveArea.getHeight() + 2).withHeight(14),
                     10, false);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        dragging = -1;
        pairedDrag.cancel();
        panHistoryPending = false;
        if (showLoops) {
            if (!currentRow() || sampleFrames < 12
                || !waveArea.contains(e.getPosition())) return;
            // Option-drag has its own anchored gesture; it must take
            // priority over individual marker handles but never over
            // Shift-drag of the entire loop from v0.21.
            if (e.mods.isAltDown() && !e.mods.isShiftDown()
                && viewSpan > 0 && waveArea.getWidth() > 0) {
                panFirstAtDown = viewStart;
                panAnchorX = e.x;
                panHistoryPending = true;
                dragging = 5;
                return;
            }
            if (!loopEnabled.getToggleState()) return;
            const int loopStart = (int)sliders[4].getValue();
            const int loopEnd = (int)sliders[5].getValue();
            const int startX = waveArea.getX() + liberty::wavezoom::pixelAt(
                loopStart, waveArea.getWidth(), viewStart, viewSpan);
            const int endX = waveArea.getX() + liberty::wavezoom::pixelAt(
                loopEnd, waveArea.getWidth(), viewStart, viewSpan);
            // Shift-drag the filled loop area to translate both markers.
            // Capture the original position so there is no cumulative drift.
            if (e.mods.isShiftDown() && e.x >= startX && e.x < endX) {
                const int pressed = liberty::wavezoom::frameAt(
                    e.x - waveArea.getX(), waveArea.getWidth(),
                    viewStart, viewSpan, sampleFrames);
                const int anchored = juce::jlimit(loopStart, loopEnd - 1, pressed);
                if (pairedDrag.begin({loopStart, loopEnd, sampleFrames}, anchored)) {
                    dragging = 4;
                    return;
                }
            }
            const int distStart = std::abs(e.x - startX), distEnd = std::abs(e.x - endX);
            // Do not jump offscreen markers when clicking an arbitrary waveform position.
            if (juce::jmin(distStart, distEnd) > 18) return;
            dragging = distStart <= distEnd ? 2 : 3;
            mouseDrag(e);
            return;
        }
        if (!mapArea.contains(e.getPosition()) || rows.empty()) return;
        const int first = firstVisibleZone();
        const int count = visibleZoneCount(first);
        if (!count) return;
        const int rowIndex = first + juce::jlimit(0, count - 1,
            ((e.y - mapArea.getY()) * count) / juce::jmax(1, mapArea.getHeight()));
        if (rowIndex != selected) { list.selectRow(rowIndex); return; }
        const int note = liberty::samplezones::positionToNote(e.x - mapArea.getX(), mapArea.getWidth());
        const int low = (int)sliders[0].getValue(), high = (int)sliders[1].getValue();
        dragging = std::abs(note - low) <= std::abs(note - high) ? 0 : 1;
        mouseDrag(e);
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (dragging < 0 || !currentRow()) return;
        if (dragging == 5) {
            const auto panned = liberty::loop_v024::panFromAnchor(
                sampleFrames, panFirstAtDown, viewSpan, waveArea.getWidth(),
                panAnchorX, e.x);
            if (!panned.valid || panned.firstFrame == viewStart) return;
            if (panHistoryPending) {
                rememberViewport();
                panHistoryPending = false;
            }
            viewStart = panned.firstFrame;
            syncPositionControl();
            repaint();
            return;
        }
        if (dragging == 4 && pairedDrag.active()) {
            const int pointer = liberty::wavezoom::frameAt(
                e.x - waveArea.getX(), waveArea.getWidth(),
                viewStart, viewSpan, sampleFrames);
            const auto candidate = pairedDrag.move(pointer);
            if (!candidate.valid) return;
            if ((int)sliders[4].getValue() == candidate.loop.begin
                && (int)sliders[5].getValue() == candidate.loop.end)
                return;
            // Move both handles silently; reconcile once to retain fade
            // and pending edit flags. Never AUTO ZERO a locked duration.
            sliders[4].setValue((double)candidate.loop.begin,
                                juce::dontSendNotification);
            sliders[5].setValue((double)candidate.loop.end,
                                juce::dontSendNotification);
            reconcile(4);
            status = juce::String("GLISSE BOUCLE ")
                     + (candidate.appliedFrames > 0 ? "+" : "")
                     + juce::String((int)candidate.appliedFrames) + " f";
            updateFeedback();
            return;
        }
        if (dragging >= 2) {
            const int frame = liberty::wavezoom::frameAt(
                e.x - waveArea.getX(), waveArea.getWidth(), viewStart, viewSpan, sampleFrames);
            if (dragging == 2) sliders[4].setValue(juce::jmin(frame, (int)sliders[5].getValue() - 11));
            else sliders[5].setValue(juce::jmax(frame, (int)sliders[4].getValue() + 11));
            return;
        }
        const int note = liberty::samplezones::positionToNote(e.x - mapArea.getX(), mapArea.getWidth());
        if (dragging == 0) sliders[0].setValue(juce::jmin(note, (int)sliders[1].getValue()));
        else sliders[1].setValue(juce::jmax(note, (int)sliders[0].getValue()));
    }
    void mouseUp(const juce::MouseEvent&) override {
        const int moved = dragging;
        dragging = -1;
        pairedDrag.cancel();
        panHistoryPending = false;
        if (showLoops && (moved == 2 || moved == 3) && autoSnap.getToggleState())
            snapLoopMarker(moved == 2);
    }
    void mouseDoubleClick(const juce::MouseEvent& event) override {
        if (!showLoops || !waveArea.contains(event.getPosition())) return;
        // Shift+double-click toggles the previous zoom/position snapshot.
        // All older double-click gestures retain their previous behavior.
        if (event.mods.isShiftDown() && !event.mods.isAltDown()
            && !event.mods.isCommandDown()) {
            restorePreviousViewport();
            return;
        }
        // Keep plain double-click as FIT ALL; Option+double-click focuses
        // the chosen loop without moving either marker or changing the fade.
        if (event.mods.isAltDown() && focusLoopViewport())
            return;
        if (event.mods.isCommandDown() && !event.mods.isAltDown()
            && centerViewportOnClick(event.x - waveArea.getX()))
            return;
        zoomSlider.setValue(1.0);
    }
    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override {
        if (showLoops && waveArea.contains(event.getPosition())) {
            // Shift+wheel pans the current viewport, even with a disabled
            // loop; never edits the loop start/end/fade or triggers AUTO ZERO.
            // Some macOS trackpads expose Shift+vertical as deltaX instead.
            if (event.mods.isShiftDown() && !event.mods.isAltDown()
                && (wheel.deltaY != 0.0f || wheel.deltaX != 0.0f)
                && viewSpan > 0) {
                const double wheelAmount = wheel.deltaY != 0.0f
                                              ? wheel.deltaY : wheel.deltaX;
                const auto nextView = liberty::loop_v025::panByWheel(
                    sampleFrames, viewStart, viewSpan, wheelAmount);
                if (nextView.valid) {
                    if (nextView.firstFrame != viewStart)
                        rememberViewport();
                    viewStart = nextView.firstFrame;
                    syncPositionControl();
                    repaint();
                    return;
                }
            }
            if (wheel.deltaY != 0.0f) {
                const double current = zoomSlider.getValue();
                const double next = juce::jlimit(1.0, liberty::wavezoom::maxZoom,
                                                 current * std::pow(2.0, wheel.deltaY * 3.0));
                if (event.mods.isAltDown()
                    && sampleFrames > 0 && viewSpan > 0) {
                    const int nextSpan = liberty::wavezoom::span(sampleFrames, next);
                    const auto viewport = liberty::loop_v023::zoomAtCursor(
                        sampleFrames, viewStart, viewSpan, nextSpan,
                        waveArea.getWidth(), event.x - waveArea.getX());
                    if (viewport.valid) {
                        // Changing the zoom slider with notifications would
                        // re-center the view on its former middle. Instead,
                        // set zoom and scroll together, leaving markers intact.
                        if (viewport.spanFrames != viewSpan
                            || viewport.firstFrame != viewStart)
                            rememberViewport();
                        zoomSlider.setValue(next, juce::dontSendNotification);
                        viewSpan = viewport.spanFrames;
                        viewStart = viewport.firstFrame;
                        lastAppliedZoom = next;
                        syncPositionControl();
                        repaint();
                        return;
                    }
                }
                zoomSlider.setValue(next);
                return;
            }
            if (wheel.deltaX != 0.0f && viewSpan < sampleFrames) {
                positionSlider.setValue(juce::jlimit(0.0, 1.0,
                    positionSlider.getValue() - wheel.deltaX * 0.20));
                return;
            }
        }
        juce::Component::mouseWheelMove(event, wheel);
    }

private:
    struct Row {
        liberty::ZoneDescriptor zone;
        juce::String bankId, presetId;
        int index;
        juce::String source;
    };
    LibertyRomplerAudioProcessor& processor;
    juce::AudioFormatManager formats;
    juce::AudioThumbnailCache thumbnailCache {8};
    juce::AudioThumbnail thumbnail;
    juce::ListBox list;
    juce::TextButton mappingMode, loopMode, apply, reset;
    juce::TextButton snapStartButton, snapEndButton, cancelButton, fitWaveButton;
    // Frame-accurate controls integrated with the existing sample-zone sliders.
    std::array<juce::TextButton, 4> frameNudges;
    juce::TextButton previewButton;
    juce::ToggleButton autoSnap;
    juce::Slider zoomSlider, positionSlider;
    juce::Label zoomCaption, positionCaption;
    juce::Label feedback;
    std::array<juce::Slider, 7> sliders;
    std::array<juce::Label, 7> labels;
    juce::ToggleButton loopEnabled;
    std::vector<Row> rows;
    juce::Rectangle<int> mapArea, velocityArea, waveArea;
    juce::String status;
    int selected = -1, sampleFrames = 0, dragging = -1;
    int panFirstAtDown = 0, panAnchorX = 0;
    bool panHistoryPending = false;
    liberty::loop_v021::PairedLoopDrag pairedDrag;
    int viewStart = 0, viewSpan = 0;
    double lastAppliedZoom = 1.0;
    liberty::loop_v027::ViewHistory viewHistory;
    double loadedSeconds = 0.0;
    double loadedSampleRate = 0.0;
    bool showLoops = false, loading = false, dirty = false;

    liberty::loop_v034::Segments loopSegments() const noexcept {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState())
            return {};
        return liberty::loop_v034::describeSegments(
            sampleFrames, (int)sliders[4].getValue(),
            (int)sliders[5].getValue());
    }
    liberty::loop_v033::Coverage loopCoverage() const noexcept {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState())
            return {};
        return liberty::loop_v033::describeCoverage(
            sampleFrames, (int)sliders[4].getValue(),
            (int)sliders[5].getValue());
    }
    liberty::loop_v032::FadeRatio fadeRatio() const noexcept {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState())
            return {};
        return liberty::loop_v032::describeFade(
            sampleFrames, (int)sliders[4].getValue(),
            (int)sliders[5].getValue(), (int)sliders[6].getValue());
    }
    liberty::loop_v031::MarkerTimes markerTimes() const noexcept {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState())
            return {};
        return liberty::loop_v031::describePositions(
            sampleFrames, (int)sliders[4].getValue(),
            (int)sliders[5].getValue(), loadedSampleRate);
    }
    liberty::loop_v030::Timing loopTiming() const noexcept {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState())
            return {};
        return liberty::loop_v030::describe(
            sampleFrames,
            (int)sliders[4].getValue(), (int)sliders[5].getValue(),
            (int)sliders[6].getValue(), loadedSampleRate);
    }
    liberty::loop_v027::View currentViewport() const noexcept {
        return {sampleFrames, viewStart, viewSpan, lastAppliedZoom};
    }
    void rememberViewport() {
        viewHistory.remember(currentViewport());
    }
    bool restorePreviousViewport() {
        liberty::loop_v027::View restored;
        if (!viewHistory.toggle(currentViewport(), restored))
            return false;
        // Silent restoration keeps exactly the previous zoom and position.
        zoomSlider.setValue(restored.zoom, juce::dontSendNotification);
        viewSpan = restored.span;
        viewStart = restored.first;
        lastAppliedZoom = restored.zoom;
        syncPositionControl();
        repaint();
        return true;
    }
    void syncPositionControl() {
        positionSlider.setValue(liberty::wavezoom::toScroll(viewStart, sampleFrames, viewSpan),
                                juce::dontSendNotification);
        positionSlider.setEnabled(showLoops && viewSpan > 0 && viewSpan < sampleFrames);
        fitWaveButton.setEnabled(showLoops && zoomSlider.getValue() > 1.0);
    }
    // Command-double-click centers the current zoom on the clicked frame.
    // This is viewport-only; markers, fade, source WAV and session untouched.
    // Command-click on either existing ZERO button navigates to that loop
    // boundary, preserving zoom and the complete sample-zone edit state.
    // Command+Option-click on an existing ZERO button: zoom into the
    // selected boundary. Keep all marker positions and edits unchanged.
    // Respect the max zoom and preserve any already finer magnification.
    bool focusLoopMarkerDetail(bool startMarker) {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState()
            || sampleFrames < 12 || viewSpan < 1 || !hasLoopValues())
            return false;
        const int marker = (int)sliders[startMarker ? 4 : 5].getValue();
        const auto detail = liberty::loop_v029::planPrecisionZoom(
            sampleFrames, marker, zoomSlider.getValue(),
            liberty::wavezoom::maxZoom);
        if (!detail.valid) return false;
        const int nextSpan = liberty::wavezoom::span(
            sampleFrames, detail.requestedZoom);
        const auto target = liberty::loop_v028::focusOnMarker(
            sampleFrames, viewStart, nextSpan, detail.markerFrame);
        if (!target.valid) return false;
        if (nextSpan != viewSpan || target.firstFrame != viewStart)
            rememberViewport();
        // Apply zoom + viewport silently together, to avoid the old-center
        // updateZoom callback; Shift-double-click can restore both values.
        zoomSlider.setValue(detail.requestedZoom, juce::dontSendNotification);
        viewSpan = target.spanFrames;
        viewStart = target.firstFrame;
        lastAppliedZoom = detail.requestedZoom;
        syncPositionControl();
        repaint();
        return true;
    }
    bool focusLoopMarker(bool startMarker) {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState()
            || sampleFrames < 12 || viewSpan < 1 || !hasLoopValues())
            return false;
        const int marker = (int)sliders[startMarker ? 4 : 5].getValue();
        const auto target = liberty::loop_v028::focusOnMarker(
            sampleFrames, viewStart, viewSpan, marker);
        if (!target.valid) return false;
        if (target.firstFrame != viewStart)
            rememberViewport();
        viewStart = target.firstFrame;
        syncPositionControl();
        repaint();
        return true;
    }
    bool centerViewportOnClick(int localX) {
        if (!showLoops || !currentRow() || sampleFrames < 1 || viewSpan < 1)
            return false;
        const auto centered = liberty::loop_v026::centerOnPointer(
            sampleFrames, viewStart, viewSpan, waveArea.getWidth(), localX);
        if (!centered.valid) return false;
        if (centered.firstFrame != viewStart)
            rememberViewport();
        viewStart = centered.firstFrame;
        syncPositionControl();
        repaint();
        return true;
    }
    bool focusLoopViewport() {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState()
            || sampleFrames < 12) return false;
        const auto focus = liberty::loop_v022::planFocus(
            sampleFrames, (int)sliders[4].getValue(),
            (int)sliders[5].getValue(), liberty::wavezoom::maxZoom);
        if (!focus.valid) return false;

        // No notification: avoid updateZoom() recentering around the
        // previous viewport. Set zoom + center as one atomic UI operation.
        const int targetSpan = liberty::wavezoom::span(
            sampleFrames, focus.requestedZoom);
        const int targetStart = liberty::wavezoom::centeredStart(
            focus.centerFrame, sampleFrames, targetSpan);
        if (targetSpan != viewSpan || targetStart != viewStart)
            rememberViewport();
        zoomSlider.setValue(focus.requestedZoom, juce::dontSendNotification);
        viewSpan = targetSpan;
        viewStart = targetStart;
        lastAppliedZoom = focus.requestedZoom;
        syncPositionControl();
        status = juce::String("CADRAGE BOUCLE ") +
                 juce::String((int)sliders[5].getValue()
                              - (int)sliders[4].getValue()) + " frames";
        updateFeedback();
        repaint();
        return true;
    }
    void updateZoom() {
        const int center = viewSpan > 0 ? viewStart + viewSpan / 2 : sampleFrames / 2;
        const int nextSpan = liberty::wavezoom::span(sampleFrames, zoomSlider.getValue());
        const int nextStart = liberty::wavezoom::centeredStart(
            center, sampleFrames, nextSpan);
        if (nextSpan != viewSpan || nextStart != viewStart)
            rememberViewport();
        viewSpan = nextSpan;
        viewStart = nextStart;
        lastAppliedZoom = zoomSlider.getValue();
        syncPositionControl();
        repaint();
    }
    void updatePosition() {
        const int nextStart = liberty::wavezoom::fromScroll(
            positionSlider.getValue(), sampleFrames, viewSpan);
        if (nextStart != viewStart)
            rememberViewport();
        viewStart = nextStart;
        repaint();
    }
    const Row* currentRow() const {
        return selected >= 0 && selected < (int)rows.size() ? &rows[(size_t)selected] : nullptr;
    }
    bool hasLoopValues() const { return sliders[5].getValue() >= sliders[4].getValue() + 11; }
    int getNumRows() override { return (int)rows.size(); }
    void selectedRowsChanged(int index) override {
        dragging = -1;
        pairedDrag.cancel();
        panHistoryPending = false;
        viewHistory.clear();
        lastAppliedZoom = 1.0;
        selected = index >= 0 && index < (int)rows.size() ? index : -1;
        thumbnail.clear();
        sampleFrames = 0;
        loadedSeconds = 0.0;
        loadedSampleRate = 0.0;
        viewStart = viewSpan = 0;
        zoomSlider.setValue(1.0, juce::dontSendNotification);
        positionSlider.setValue(0.0, juce::dontSendNotification);
        if (const auto* row = currentRow()) {
            const auto& f = row->zone.file;
            if (f.existsAsFile()) {
                if (auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(f))) {
                    sampleFrames = liberty::samplezones::loadedFrames(reader->lengthInSamples, reader->sampleRate);
                    if (reader->sampleRate > 0) {
                        loadedSampleRate = reader->sampleRate;
                        loadedSeconds = (double)sampleFrames / reader->sampleRate;
                    }
                }
                thumbnail.setSource(new juce::FileInputSource(f));
            }
            loading = true;
            sliders[0].setValue(row->zone.lowNote, juce::dontSendNotification);
            sliders[1].setValue(row->zone.highNote, juce::dontSendNotification);
            sliders[2].setValue(row->zone.lowVelocity, juce::dontSendNotification);
            sliders[3].setValue(row->zone.highVelocity, juce::dontSendNotification);
            const int maxFrames = juce::jmax(12, sampleFrames);
            sliders[4].setRange(0, maxFrames - 12, 1);
            sliders[5].setRange(11, maxFrames, 1);
            sliders[4].setValue(row->zone.loopStart >= 0 ? row->zone.loopStart : 0, juce::dontSendNotification);
            sliders[5].setValue(row->zone.loopEnd >= 12 ? row->zone.loopEnd : maxFrames, juce::dontSendNotification);
            const int maxFade = liberty::loopfade::maximum((int)sliders[4].getValue(),
                                                             (int)sliders[5].getValue());
            sliders[6].setRange(0, maxFade, 1);
            sliders[6].setValue(juce::jlimit(0, maxFade, row->zone.loopCrossfadeFrames),
                                juce::dontSendNotification);
            loopEnabled.setToggleState(row->zone.loopStart >= 0 && row->zone.loopEnd > row->zone.loopStart,
                                       juce::dontSendNotification);
            loading = false;
        }
        viewSpan = liberty::wavezoom::span(sampleFrames, 1.0);
        syncPositionControl();
        dirty = false;
        status.clear();
        updateFeedback();
        updateLayout();
        repaint();
    }
    void changeListenerCallback(juce::ChangeBroadcaster*) override { repaint(); }
    void paintListBoxItem(int index, juce::Graphics& g, int w, int h, bool highlighted) override {
        if (index < 0 || index >= (int)rows.size()) return;
        g.setColour(highlighted ? juce::Colour(0xff345464) : juce::Colour(0xff1a2938));
        g.fillRoundedRectangle(juce::Rectangle<int>(2, 2, w - 5, h - 4).toFloat(), 5.0f);
        g.setColour(juce::Colour(0xffe3edf2));
        g.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
        g.drawFittedText(rows[(size_t)index].source,
                         {9, 4, w - 18, 18}, juce::Justification::centredLeft, 1, 0.75f);
        g.setColour(juce::Colour(0xffa8c7d4));
        g.setFont(juce::Font(juce::FontOptions(10.5f)));
        const auto& z = rows[(size_t)index].zone;
        const auto desc = z.file.getFileNameWithoutExtension() + " | N " +
            juce::String(z.lowNote) + "-" + juce::String(z.highNote) + "  V " +
            juce::String(z.lowVelocity) + "-" + juce::String(z.highVelocity);
        g.drawFittedText(desc, {9, 25, w - 18, 17}, juce::Justification::centredLeft, 1, 0.75f);
    }
    void drawText(juce::Graphics& g, const juce::String& text,
                  juce::Rectangle<int> bounds, float size, bool bold) const {
        if (bounds.isEmpty()) return;
        g.setColour(bold ? juce::Colour(0xff6fe1d2) : juce::Colour(0xffbdd1db));
        g.setFont(juce::Font(juce::FontOptions(size,
            bold ? juce::Font::bold : juce::Font::plain)));
        g.drawFittedText(text, bounds, juce::Justification::centredLeft, 1, 0.75f);
    }
    int firstVisibleZone() const { return juce::jmax(0, selected - 3); }
    int visibleZoneCount(int first) const { return juce::jmin(8, (int)rows.size() - first); }

    void drawNoteMap(juce::Graphics& g) {
        if (mapArea.isEmpty()) return;
        g.setColour(juce::Colour(0xff1d3142));
        g.fillRoundedRectangle(mapArea.toFloat(), 4.0f);
        const int first = firstVisibleZone();
        const int count = visibleZoneCount(first);
        if (!count) return;
        const float rowHeight = (float)mapArea.getHeight() / (float)count;
        for (int i = 0; i < count; ++i) {
            const int index = i + first;
            const auto& zone = rows[(size_t)index].zone;
            const int low = index == selected ? (int)sliders[0].getValue() : zone.lowNote;
            const int high = index == selected ? (int)sliders[1].getValue() : zone.highNote;
            const int x1 = mapArea.getX() + mapArea.getWidth() * low / 128;
            const int x2 = mapArea.getX() + mapArea.getWidth() * (high + 1) / 128;
            const float y = (float)mapArea.getY() + rowHeight * (float)i + 1.0f;
            g.setColour(index == selected ? juce::Colour(0xff6ce7d2) : juce::Colour(0xff447eae));
            g.fillRoundedRectangle((float)x1, y, (float)juce::jmax(2, x2 - x1),
                                   juce::jmax(2.0f, rowHeight - 2.0f), 3.0f);
        }
        if (const auto* row = currentRow()) {
            const int marker = mapArea.getX() + mapArea.getWidth() * row->zone.rootNote / 128;
            g.setColour(juce::Colour(0xffffce75));
            g.drawVerticalLine(marker, (float)mapArea.getY(), (float)mapArea.getBottom());
        }
        g.setColour(juce::Colour(0x708babba));
        for (int note = 12; note < 128; note += 12) {
            const float x = (float)mapArea.getX() + (float)mapArea.getWidth() * (float)note / 128.0f;
            g.drawVerticalLine((int)x, (float)mapArea.getY(), (float)mapArea.getBottom());
        }
    }
    void drawVelocityLayers(juce::Graphics& g) {
        if (velocityArea.isEmpty()) return;
        g.setColour(juce::Colour(0xff1d3142));
        g.fillRoundedRectangle(velocityArea.toFloat(), 4.0f);
        const int first = firstVisibleZone(), count = visibleZoneCount(first);
        if (!count) return;
        for (int i = 0; i < count; ++i) {
            const int index = i + first;
            const auto& zone = rows[(size_t)index].zone;
            const int v1 = index == selected ? (int)sliders[2].getValue() : zone.lowVelocity;
            const int v2 = index == selected ? (int)sliders[3].getValue() : zone.highVelocity;
            const int x = velocityArea.getX() + i * velocityArea.getWidth() / count;
            const int width = juce::jmax(2, velocityArea.getWidth() / count - 2);
            const int top = velocityArea.getBottom() - v2 * velocityArea.getHeight() / 127;
            const int bottom = velocityArea.getBottom() - (v1 - 1) * velocityArea.getHeight() / 127;
            g.setColour(index == selected ? juce::Colour(0xff69e2ce) : juce::Colour(0xff537ab2));
            g.fillRect(x, top, width, juce::jmax(1, bottom - top));
        }
    }
    void drawWaveform(juce::Graphics& g) {
        if (waveArea.isEmpty()) return;
        g.setColour(juce::Colour(0xff183140));
        g.fillRoundedRectangle(waveArea.toFloat(), 4.0f);
        if (thumbnail.getNumChannels() > 0 && thumbnail.getTotalLength() > 0 &&
            sampleFrames > 0 && viewSpan > 0) {
            const juce::Graphics::ScopedSaveState save(g);
            g.reduceClipRegion(waveArea);
            const int last = juce::jmin(sampleFrames, viewStart + viewSpan);
            const double firstSecond = loadedSeconds * viewStart / sampleFrames;
            const double lastSecond = loadedSeconds * last / sampleFrames;
            g.setColour(juce::Colour(0xff70e4d1));
            thumbnail.drawChannels(g, waveArea.reduced(4, 5), firstSecond,
                                   juce::jmin(lastSecond, thumbnail.getTotalLength()), 0.80f);
            if (loopEnabled.getToggleState()) {
                const int loopStart = (int)sliders[4].getValue();
                const int loopEnd = (int)sliders[5].getValue();
                const int x1 = waveArea.getX() + liberty::wavezoom::pixelAt(
                    loopStart, waveArea.getWidth(), viewStart, viewSpan);
                const int x2 = waveArea.getX() + liberty::wavezoom::pixelAt(
                    loopEnd, waveArea.getWidth(), viewStart, viewSpan);
                const int left = juce::jlimit(waveArea.getX(), waveArea.getRight(), x1);
                const int right = juce::jlimit(waveArea.getX(), waveArea.getRight(), x2);
                if (right > left) {
                    g.setColour(juce::Colour(0x4469e5c5));
                    g.fillRect(left, waveArea.getY(), right - left, waveArea.getHeight());
                }
                const int fade = (int)sliders[6].getValue();
                if (fade > 0 && viewSpan > 0) {
                    const auto bands = liberty::loop_v015::crossfadeBands(
                        {loopStart, loopEnd, sampleFrames}, fade,
                        {viewStart, viewSpan, (double)waveArea.getX(),
                         (double)waveArea.getWidth()});
                    const auto paintBand = [&g, this](liberty::loop_v015::VisibleBand b,
                                                      juce::Colour tint) {
                        if (!b.visible) return;
                        g.setColour(tint);
                        g.fillRect((float)b.x, (float)waveArea.getY(), (float)b.width,
                                   (float)waveArea.getHeight());
                    };
                    paintBand(bands.tail, juce::Colour(0x6676b1e8));
                    paintBand(bands.head, juce::Colour(0x665dd6be));
                }
                g.setColour(juce::Colour(0xffe8d389));
                if (x1 >= waveArea.getX() && x1 <= waveArea.getRight())
                    g.drawVerticalLine(x1, (float)waveArea.getY(), (float)waveArea.getBottom());
                if (x2 >= waveArea.getX() && x2 <= waveArea.getRight())
                    g.drawVerticalLine(x2, (float)waveArea.getY(), (float)waveArea.getBottom());
            }
        } else {
            drawText(g, "Forme d'onde indisponible", waveArea.reduced(12), 11, false);
        }
    }

    // Shift-click on any existing +/- frame button moves the entire loop.
    // Keep start/end together and never let automatic zero snap break
    // the exact loop length. This change remains pending until APPLIQUER.
    void nudgeLoopPair(int delta) {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState()
            || sampleFrames < 12) return;
        const auto start = (int)sliders[4].getValue();
        const auto end = (int)sliders[5].getValue();
        const auto result = liberty::loop_v020::nudgePair(
            {start, end, sampleFrames}, delta);
        if (!result.valid) return;
        if (result.appliedFrames == 0) {
            status = "GLISSE : limite atteinte";
        } else {
            sliders[4].setValue((double)result.loop.begin,
                                juce::dontSendNotification);
            sliders[5].setValue((double)result.loop.end,
                                juce::dontSendNotification);
            // One reconciliation keeps the fade/undo flags synchronized.
            reconcile(4);
            status = juce::String("GLISSE ")
                     + (result.appliedFrames > 0 ? "+" : "")
                     + juce::String((int)result.appliedFrames) + " f";
        }
        updateFeedback();
        repaint();
    }

    void nudgeLoopMarker(bool startMarker, int delta) {
        if (!showLoops || !currentRow() || !loopEnabled.getToggleState() ||
            sampleFrames < 12) return;
        const auto start = (int)sliders[4].getValue();
        const auto end = (int)sliders[5].getValue();
        const liberty::loop_v015::Loop loop {start, end, sampleFrames};
        if (!liberty::loop_v015::isValid(loop, 11)) return;
        const auto adjusted = liberty::loop_v015::nudge(
            loop, startMarker ? liberty::loop_v015::Marker::begin
                              : liberty::loop_v015::Marker::end,
            delta, 11);
        sliders[(size_t)(startMarker ? 4 : 5)].setValue(
            startMarker ? adjusted.begin : adjusted.end);
        if (autoSnap.getToggleState())
            snapLoopMarker(startMarker);
    }

    void previewSelectedLoop(bool withoutFade, bool extendedContext) {
        const auto* row = currentRow();
        if (!showLoops || !row || !loopEnabled.getToggleState() ||
            sampleFrames < 12) return;
        float rawSeamStepDb = -120.0f;
        const auto result = processor.previewSampleZoneLoop(
            row->zone.file, (int)sliders[4].getValue(), (int)sliders[5].getValue(),
            withoutFade ? 0 : (int)sliders[6].getValue(), &rawSeamStepDb,
            extendedContext ? 250 : 60);
        // The number is the raw sample-amplitude step, not a click audibility score.
        status = result.wasOk()
            ? juce::String(withoutFade ? "A" : "B") +
              (extendedContext ? " 250 ms" : " 60 ms") +
              " | BRUT " + juce::String(rawSeamStepDb, 1) + " dB"
            : result.getErrorMessage();
        updateFeedback();
    }

    // Shift+Option+click on either existing ZERO button searches a shift of
    // BOTH loop markers. Length, crossfade and WAV remain unchanged. An editor
    // mutation is pending until the user chooses the existing APPLIQUER action.
    void optimizeLoopPair() {
        const auto* row = currentRow();
        if (!showLoops || !row || !loopEnabled.getToggleState()
            || sampleFrames < 12) return;

        const int start = (int)sliders[4].getValue();
        const int end = (int)sliders[5].getValue();
        if (start < 0 || end > sampleFrames || end - start < 11)
            return;

        constexpr int radius = 512;
        const int startFirst = juce::jmax(0, start - radius - 2);
        const int endFirst = juce::jmax(0, end - radius - 3);
        const int startLast = (int)juce::jmin(
            (juce::int64)sampleFrames, (juce::int64)start + radius + 3);
        const int endLast = (int)juce::jmin(
            (juce::int64)sampleFrames, (juce::int64)end + radius + 2);
        const int startCount = startLast - startFirst;
        const int endCount = endLast - endFirst;
        if (startCount < 2 || endCount < 2)
            return;

        auto reader = std::unique_ptr<juce::AudioFormatReader>(
            formats.createReaderFor(row->zone.file));
        if (!reader || reader->numChannels < 1 || reader->sampleRate <= 0.0) {
            status = "Source WAV indisponible";
            updateFeedback();
            return;
        }
        const int channels = juce::jlimit(1, 2, (int)reader->numChannels);
        juce::AudioBuffer<float> starts(channels, startCount);
        juce::AudioBuffer<float> ends(channels, endCount);
        if (!reader->read(&starts, 0, startCount, startFirst, true, true)
            || !reader->read(&ends, 0, endCount, endFirst, true, true)) {
            status = "Lecture WAV impossible";
            updateFeedback();
            return;
        }

        std::vector<std::vector<float>> head((size_t)channels);
        std::vector<std::vector<float>> tail((size_t)channels);
        for (int ch = 0; ch < channels; ++ch) {
            const auto* a = starts.getReadPointer(ch);
            const auto* b = ends.getReadPointer(ch);
            head[(size_t)ch].assign(a, a + startCount);
            tail[(size_t)ch].assign(b, b + endCount);
        }

        const auto suggestion = liberty::loop_v019::findBestPairedShift(
            head, startFirst, tail, endFirst, start, end, sampleFrames, radius);
        if (!suggestion.valid) {
            status = "Recherche PAIRE indisponible";
            updateFeedback();
            return;
        }

        if (suggestion.deltaFrames == 0) {
            status = "PAIRE : aucun gain";
        } else {
            // Set both markers without intermediate slider callbacks.
            // Reconcile exactly once so undo/cancel/dirty/fade stay in sync.
            sliders[4].setValue(suggestion.startFrame, juce::dontSendNotification);
            sliders[5].setValue(suggestion.endFrame, juce::dontSendNotification);
            reconcile(4);
            status = juce::String("PAIRE ")
                   + juce::String(suggestion.deltaFrames > 0 ? "+" : "")
                   + juce::String(suggestion.deltaFrames) + "f | "
                   + juce::String(end - start) + "f";
        }
        updateFeedback();
        feedback.setTooltip(
            "PAIRE : longueur conservee. Saut brut avant "
            + juce::String(suggestion.previousPeakStep, 4) + " / apres "
            + juce::String(suggestion.bestPeakStep, 4)
            + ". Comparer A/B, puis APPLIQUER ou ANNULER.");
        repaint();
    }

    // Option+click on the existing ZERO buttons searches a small neighborhood
    // for the least *raw* stereo amplitude step. No sample, manifest, or DAW
    // state is modified until the existing APPLIQUER action is clicked.
    void optimizeLoopMarker(bool startMarker) {
        const auto* row = currentRow();
        if (!showLoops || !row || !loopEnabled.getToggleState()
            || sampleFrames < 12) return;
        const auto start = (int)sliders[4].getValue();
        const auto end = (int)sliders[5].getValue();
        if (start < 0 || end > sampleFrames || end - start < 11) return;
        const auto current = startMarker ? start : end;
        constexpr int radius = 512;
        const auto low = startMarker ? 0 : start + 11;
        const auto high = startMarker ? end - 11 : sampleFrames;
        const auto first = juce::jmax(0, current - radius - 2);
        const auto lastExclusive = juce::jmin(sampleFrames, current + radius + 3);
        const auto count = lastExclusive - first;
        if (count < 2) return;

        auto reader = std::unique_ptr<juce::AudioFormatReader>(
            formats.createReaderFor(row->zone.file));
        if (!reader || reader->numChannels < 1 || reader->sampleRate <= 0.0) {
            status = "Source WAV indisponible";
            updateFeedback();
            return;
        }
        const auto referenceFirst = startMarker ? end - 2 : start;
        if (referenceFirst < 0 || referenceFirst + 1 >= sampleFrames) return;
        const auto channels = juce::jlimit(1, 2, (int)reader->numChannels);
        juce::AudioBuffer<float> movingBuffer(channels, count);
        juce::AudioBuffer<float> referenceBuffer(channels, 2);
        if (!reader->read(&movingBuffer, 0, count, first, true, true)
            || !reader->read(&referenceBuffer, 0, 2,
                             referenceFirst, true, true)) {
            status = "Lecture WAV impossible";
            updateFeedback();
            return;
        }

        std::vector<std::vector<float>> neighborhood((size_t)channels);
        std::vector<std::array<float, 2>> fixed((size_t)channels);
        for (int ch = 0; ch < channels; ++ch) {
            const auto* moving = movingBuffer.getReadPointer(ch);
            const auto* edge = referenceBuffer.getReadPointer(ch);
            neighborhood[(size_t)ch].assign(moving, moving + count);
            fixed[(size_t)ch] = {edge[0], edge[1]};
        }

        const auto best = liberty::loop_v018::findLowestRawStep(
            neighborhood, first, fixed, startMarker, current, low, high, radius);
        if (!best.valid) {
            status = "Raccord non analysable";
            updateFeedback();
            return;
        }

        if (best.marker == current) {
            status = "SAUT MIN : aucun meilleur point";
        } else {
            sliders[(size_t)(startMarker ? 4 : 5)].setValue(best.marker);
            // Keep the change pending and reversible: do NOT call APPLIQUER
            // or AUTO ZERO here, which would move the chosen point again.
            status = juce::String("SAUT MIN ") + (startMarker ? "D " : "F ")
                     + juce::String(best.marker) + " ("
                     + juce::String(best.marker - current) + ")";
        }
        updateFeedback();
        feedback.setTooltip(
            "Recherche non destructive dans 512 frames : saut brut "
            + juce::String(best.previousStep, 4) + " -> "
            + juce::String(best.bestStep, 4)
            + ". Comparer A/B avant d'appliquer.");
        repaint();
    }

    void snapLoopMarker(bool startMarker) {
        const auto* row = currentRow();
        if (!row || !loopEnabled.getToggleState() || sampleFrames < 12) return;
        const auto& file = row->zone.file;
        auto reader = std::unique_ptr<juce::AudioFormatReader>(formats.createReaderFor(file));
        if (!reader || reader->numChannels < 1) {
            status = "Source WAV indisponible";
            updateFeedback();
            return;
        }
        constexpr int radius = 512;
        const int current = (int)sliders[(size_t)(startMarker ? 4 : 5)].getValue();
        // Exclusive loop end may equal sampleFrames; reading stays within the file.
        const int first = juce::jmax(0, current - radius - 2);
        const int lastExclusive = juce::jmin(sampleFrames, current + radius + 3);
        const int count = lastExclusive - first;
        if (count < 3) return;
        const int channels = juce::jlimit(1, 2, (int)reader->numChannels);
        juce::AudioBuffer<float> preview(channels, count);
        preview.clear();
        if (!reader->read(&preview, 0, count, first, true, true)) {
            status = "Lecture WAV impossible";
            updateFeedback();
            return;
        }
        const int minValid = startMarker ? 0 : ((int)sliders[4].getValue() + 11);
        const int maxValid = startMarker ? ((int)sliders[5].getValue() - 11) : sampleFrames;
        const int local = liberty::zerosnap::closest(
            preview.getReadPointer(0), channels > 1 ? preview.getReadPointer(1) : nullptr,
            count, current - first, radius, minValid - first, maxValid - first);
        if (local < 0) {
            status = juce::String::fromUTF8(u8"Aucun passage par z\u00e9ro dans 512 frames");
            updateFeedback();
            return;
        }
        const int snapped = first + local;
        sliders[(size_t)(startMarker ? 4 : 5)].setValue(snapped);
        status = (startMarker ? "Debut : " : "Fin : ") + juce::String(snapped) + " frames";
        updateFeedback();
        repaint();
    }

    void applyEdits() {
        const auto* row = currentRow();
        if (!row) return;
        LibertyRomplerAudioProcessor::SampleZoneEdit edit;
        edit.bankId = row->bankId;
        edit.presetId = row->presetId;
        edit.zoneIndex = row->index;
        edit.lowNote = (int)sliders[0].getValue();
        edit.highNote = (int)sliders[1].getValue();
        edit.lowVelocity = (int)sliders[2].getValue();
        edit.highVelocity = (int)sliders[3].getValue();
        if (loopEnabled.getToggleState()) {
            edit.loopStart = (int)sliders[4].getValue();
            edit.loopEnd = (int)sliders[5].getValue();
            edit.loopCrossfadeFrames = (int)sliders[6].getValue();
        }
        const auto result = processor.setSampleZoneEdit(edit);
        if (result.wasOk()) refresh();
        status = result.wasOk() ? juce::String::fromUTF8(u8"Modifications appliqu\u00e9es au projet") : result.getErrorMessage();
        updateFeedback();
        repaint();
    }

    void reconcile(int changed) {
        if (loading) return;
        loading = true;
        auto clampPair = [this, changed](int lo, int hi) {
            if (sliders[(size_t)lo].getValue() > sliders[(size_t)hi].getValue()) {
                if (changed == lo) sliders[(size_t)hi].setValue(sliders[(size_t)lo].getValue(),
                                                                juce::dontSendNotification);
                else sliders[(size_t)lo].setValue(sliders[(size_t)hi].getValue(),
                                                 juce::dontSendNotification);
            }
        };
        if (changed < 2) clampPair(0, 1);
        else if (changed < 4) clampPair(2, 3);
        else if (sampleFrames >= 12 && sliders[4].getValue() + 11 >= sliders[5].getValue()) {
            if (changed == 4) sliders[5].setValue(sliders[4].getValue() + 11,
                                                   juce::dontSendNotification);
            else sliders[4].setValue(sliders[5].getValue() - 11,
                                     juce::dontSendNotification);
        }
        // Recompute the legal fade range whenever a loop handle moves.
        const int maxFade = liberty::loopfade::maximum((int)sliders[4].getValue(),
                                                       (int)sliders[5].getValue());
        sliders[6].setRange(0, maxFade, 1);
        if (sliders[6].getValue() > maxFade)
            sliders[6].setValue(maxFade, juce::dontSendNotification);
        loading = false;
        dirty = true;
        cancelButton.setEnabled(showLoops && currentRow() != nullptr);
        updateFeedback();
        repaint();
    }
    void updateFeedback() {
        const auto pendingMessage = juce::String::fromUTF8(u8"NON APPLIQU\u00c9");
        const auto message = status.isNotEmpty()
            ? (dirty ? pendingMessage + " | " + status : status)
            : (dirty ? pendingMessage : juce::String("Session DAW / banques intactes"));
        feedback.setText(message, juce::dontSendNotification);
        feedback.setTooltip(message);
        // Full frame and crossfade timing belongs in a tooltip so the
        // existing single-line UI and all previous labels remain intact.
        const auto timing = loopTiming();
        const auto ratio = fadeRatio();
        const auto coverage = loopCoverage();
        const auto segments = loopSegments();
        if (timing.valid && ratio.valid && coverage.valid && segments.valid)
            loopEnabled.setTooltip(juce::String("BOUCLE ")
                + juce::String(timing.durationFrames) + " frames | "
                + juce::String(timing.durationMs, 3) + " ms"
                + " | FONDU " + juce::String(timing.fadeFrames)
                + " frames | " + juce::String(timing.fadeMs, 3) + " ms"
                + " | " + juce::String(ratio.percentage, 2) + " %"
                + " | PART DU WAV " + juce::String(coverage.percentage, 2)
                + " %"
                + "\nAVANT LA BOUCLE " + juce::String(segments.beforePercentage, 2)
                + juce::String::fromUTF8(u8" % | APR\u00c8S LA BOUCLE ")
                + juce::String(segments.afterPercentage, 2) + " %");
        else
            loopEnabled.setTooltip(juce::String::fromUTF8(
                u8"Dur\u00e9e de boucle indisponible"));

        // v0.31: retain every pre-existing gesture in the two ZERO tooltips.
        // Append read-only source-relative times; rebuild on each edit so
        // values never accumulate or outlive a sample switch.
        const auto markerStartHelp = juce::String::fromUTF8(
            u8"Clic : z\u00e9ro. Option : SAUT MIN. Maj + Option : PAIRE (dur\u00e9e fixe)."
            u8" Commande + clic : centrer D\u00c9BUT. Commande + Option : ZOOM D\u00c9TAIL.");
        const auto markerEndHelp = juce::String::fromUTF8(
            u8"Clic : z\u00e9ro. Option : SAUT MIN. Maj + Option : PAIRE (dur\u00e9e fixe)."
            u8" Commande + clic : centrer FIN. Commande + Option : ZOOM D\u00c9TAIL.");
        const auto positions = markerTimes();
        if (positions.valid) {
            snapStartButton.setTooltip(markerStartHelp
                + juce::String::fromUTF8(u8" | D\u00c9BUT ")
                + juce::String(positions.beginFrame) + " frames | "
                + juce::String(positions.beginMs, 3) + " ms");
            snapEndButton.setTooltip(markerEndHelp
                + " | FIN " + juce::String(positions.endFrame)
                + " frames | " + juce::String(positions.endMs, 3) + " ms");
        } else {
            snapStartButton.setTooltip(markerStartHelp);
            snapEndButton.setTooltip(markerEndHelp);
        }
    }
    void updateLayout() {
        const auto b = getLocalBounds().reduced(12, 10);
        const int leftWidth = juce::jlimit(235, 410, (int)(b.getWidth() * 0.40f));
        feedback.setBounds(b.getRight() - juce::jmin(360, b.getWidth() / 2), b.getY() + 8,
                           juce::jmax(0, juce::jmin(350, b.getWidth() / 2 - 12)), 23);
        list.setBounds(b.getX() + 4, b.getY() + 47,
                       juce::jmin(leftWidth, juce::jmax(0, b.getWidth() - 22)),
                       juce::jmax(0, b.getHeight() - 58));
        const auto lb = list.getBounds();
        auto panel = rightArea();
        const int rx = panel.getX(), ry = panel.getY();
        const int rw = panel.getWidth(), rh = panel.getHeight();
        const int half = juce::jmax(0, (rw - 32) / 2);
        mappingMode.setBounds(rx + 10, ry + 4, half, 25);
        loopMode.setBounds(rx + 22 + half, ry + 4, half, 25);
        mapArea = {rx + 13, ry + 58, juce::jmax(0, rw - 26), 74};
        velocityArea = {rx + 13, ry + 152, juce::jmax(0, rw - 26), 48};
        waveArea = {rx + 13, ry + 59, juce::jmax(0, rw - 26), juce::jmax(35, rh - 310)};
        // Dedicated zoom/navigation row between waveform caption and marker tools.
        const int zy = ry + rh - 226;
        const int zx = rx + 12;
        const int zoomWidth = juce::jlimit(70, 116, (rw - 24) / 4);
        const int navigationWidth = juce::jmax(45, rw - 24 - 42 - zoomWidth - 55 - 54 - 20);
        zoomCaption.setBounds(zx, zy, 42, 25);
        zoomSlider.setBounds(zx + 47, zy, zoomWidth, 25);
        positionCaption.setBounds(zx + 52 + zoomWidth, zy, 55, 25);
        positionSlider.setBounds(zx + 112 + zoomWidth, zy, navigationWidth, 25);
        fitWaveButton.setBounds(zx + 117 + zoomWidth + navigationWidth, zy, 54, 25);
        for (auto* component : {static_cast<juce::Component*>(&zoomCaption),
                                static_cast<juce::Component*>(&zoomSlider),
                                static_cast<juce::Component*>(&positionCaption),
                                static_cast<juce::Component*>(&positionSlider),
                                static_cast<juce::Component*>(&fitWaveButton)})
            component->setVisible(showLoops);
        zoomSlider.setEnabled(showLoops && sampleFrames > 0);
        syncPositionControl();
        const int controlWidth = juce::jmax(65, (rw - 57) / 4);
        for (int i = 0; i < 4; ++i) {
            juce::Component* button = i == 0 ? static_cast<juce::Component*>(&snapStartButton) :
                                      i == 1 ? static_cast<juce::Component*>(&snapEndButton) :
                                      i == 2 ? static_cast<juce::Component*>(&cancelButton) :
                                               static_cast<juce::Component*>(&autoSnap);
            button->setVisible(showLoops);
            button->setBounds(rx + 12 + i * (controlWidth + 5), ry + rh - 187,
                              controlWidth, 25);
            button->setEnabled(currentRow() != nullptr && sampleFrames >= 12 &&
                               (i == 2 ? dirty : loopEnabled.getToggleState()));
        }
        const int cellWidth = (rw - 34) / 2;
        for (int i = 0; i < 6; ++i) {
            const bool visible = showLoops ? i >= 4 : i < 4;
            sliders[(size_t)i].setVisible(visible);
            labels[(size_t)i].setVisible(visible);
            if (!visible) continue;
            const int col = i % 2, row = showLoops ? 0 : i / 2;
            const int cy = showLoops ? (ry + rh - 112) : (ry + 210 + row * 47);
            const int cx = rx + 12 + col * (cellWidth + 10);
            labels[(size_t)i].setBounds(cx, cy, cellWidth, 15);
            sliders[(size_t)i].setBounds(cx, cy + 15, cellWidth, 27);
        }
        loopEnabled.setVisible(showLoops);
        loopEnabled.setEnabled(sampleFrames >= 12 && currentRow() != nullptr);
        loopEnabled.setBounds(rx + 12, ry + rh - 145, half, 25);
        labels[6].setVisible(showLoops);
        sliders[6].setVisible(showLoops);
        labels[6].setBounds(rx + 22 + half, ry + rh - 155, half, 15);
        sliders[6].setBounds(rx + 22 + half, ry + rh - 138, half, 25);
        // Frame nudge buttons fit in the 27px gap between existing frame
        // sliders and the bottom action row; no overlapping controls.
        const bool loopControlsFit = showLoops && rw >= 355 && rh >= 340;
        const int nudgeWidth = juce::jmax(24, (cellWidth - 4) / 2);
        for (int i = 0; i < 4; ++i) {
            auto& button = frameNudges[(size_t)i];
            const int col = i / 2, side = i % 2;
            const int nx = rx + 12 + col * (cellWidth + 10)
                              + side * (nudgeWidth + 4);
            button.setBounds(nx, ry + rh - 69, nudgeWidth, 24);
            button.setVisible(loopControlsFit);
            button.setEnabled(currentRow() != nullptr &&
                              loopEnabled.getToggleState() && sampleFrames >= 12);
        }
        previewButton.setVisible(loopControlsFit);
        previewButton.setEnabled(currentRow() != nullptr &&
                                 loopEnabled.getToggleState() && sampleFrames >= 12);
        if (loopControlsFit) {
            const int gap = 6;
            const int actionWidth = juce::jmax(0, (rw - 24 - 2 * gap) / 3);
            previewButton.setBounds(rx + 12, ry + rh - 38, actionWidth, 29);
            apply.setBounds(rx + 12 + actionWidth + gap, ry + rh - 38, actionWidth, 29);
            reset.setBounds(rx + 12 + (actionWidth + gap) * 2, ry + rh - 38,
                            actionWidth, 29);
        } else {
            apply.setBounds(rx + 12, ry + rh - 38, juce::jmax(72, half), 29);
            reset.setBounds(rx + 22 + half, ry + rh - 38, juce::jmax(72, half), 29);
        }
        // At small sizes, hide controls rather than allow an overlap. Mapping
        // remains accessible after enlarging the panel.
        const bool enoughHeight = rh >= 340;
        for (int i = 0; i < 7; ++i) {
            const bool visible = enoughHeight && (showLoops ? i >= 4 : i < 4);
            if (!visible) { sliders[(size_t)i].setVisible(false);
                            labels[(size_t)i].setVisible(false); }
            sliders[(size_t)i].setEnabled(visible && currentRow() != nullptr &&
                                        (i < 4 || sampleFrames >= 12));
        }
        if (!loopControlsFit) {
            for (auto* component : {static_cast<juce::Component*>(&zoomCaption),
                                    static_cast<juce::Component*>(&zoomSlider),
                                    static_cast<juce::Component*>(&positionCaption),
                                    static_cast<juce::Component*>(&positionSlider),
                                    static_cast<juce::Component*>(&fitWaveButton),
                                    static_cast<juce::Component*>(&snapStartButton),
                                    static_cast<juce::Component*>(&snapEndButton),
                                    static_cast<juce::Component*>(&cancelButton),
                                    static_cast<juce::Component*>(&autoSnap),
                                    static_cast<juce::Component*>(&loopEnabled)})
                component->setVisible(false);
        }
        apply.setVisible(rw >= 185 && rh >= 82);
        reset.setVisible(rw >= 185 && rh >= 82);
        apply.setEnabled(currentRow() != nullptr);
        reset.setEnabled(currentRow() != nullptr);
    }
    juce::Rectangle<int> rightArea() const {
        const auto box = getLocalBounds().reduced(12, 10);
        const auto left = list.getBounds();
        return {left.getRight() + 14, left.getY(),
                juce::jmax(0, box.getRight() - left.getRight() - 18),
                juce::jmax(0, left.getHeight())};
    }
};
