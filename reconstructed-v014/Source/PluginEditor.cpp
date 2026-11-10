#include "PluginEditor.h"
#include <cmath>

namespace {
const juce::Colour bg           {0xff0c111b};
const juce::Colour chrome       {0xff101826};
const juce::Colour panel        {0xff151e2b};
const juce::Colour module       {0xff1a2635};
const juce::Colour selected     {0xff243a4b};
const juce::Colour stroke       {0xff304050};
const juce::Colour white        {0xfff2f4f6};
const juce::Colour secondary    {0xffa8b7c6};
const juce::Colour quiet        {0xff718396};
const juce::Colour cyan         {0xff68d9cc};
const juce::Colour gold         {0xffe9ba74};
const juce::Colour green        {0xff77c9a2};
const juce::Colour warm         {0xffe79a72};
const juce::Colour purple       {0xff988ae5};

juce::String utf(const char* chars) { return juce::String::fromUTF8(chars); }

void paintTruncated(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> area,
                    int fontSize, juce::Colour colour, bool bold = false,
                    juce::Justification justification = juce::Justification::centredLeft) {
    if (area.isEmpty()) return;
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions((float)fontSize,
        bold ? juce::Font::bold : juce::Font::plain)));
    g.drawFittedText(s, area.reduced(2, 0), justification, 1, 0.82f);
}

void drawStar(juce::Graphics& g, juce::Point<float> c, float outer, juce::Colour colour, bool filled) {
    juce::Path star;
    for (int i = 0; i < 10; ++i) {
        const auto a = (float)(-juce::MathConstants<double>::halfPi + i * juce::MathConstants<double>::pi / 5.0);
        const auto r = (i % 2 == 0) ? outer : outer * 0.46f;
        const auto x = c.x + r * std::cos(a);
        const auto y = c.y + r * std::sin(a);
        if (i == 0) star.startNewSubPath(x, y); else star.lineTo(x, y);
    }
    star.closeSubPath();
    g.setColour(colour);
    if (filled) g.fillPath(star); else g.strokePath(star, juce::PathStrokeType(1.4f));
}
}

void LibertyRomplerAudioProcessorEditor::LibertyTheme::drawButtonBackground(
    juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hovered, bool pressed) {
    auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    const bool toggled = b.getToggleState();
    g.setColour(toggled ? selected : hovered ? module.brighter(0.11f) : module);
    if (pressed) g.setColour(selected.brighter(0.1f));
    g.fillRoundedRectangle(r, 7.0f);
    g.setColour(toggled ? cyan.withAlpha(0.64f) : stroke);
    g.drawRoundedRectangle(r, 7.0f, 1.0f);
}
void LibertyRomplerAudioProcessorEditor::LibertyTheme::drawButtonText(
    juce::Graphics& g, juce::TextButton& b, bool, bool) {
    paintTruncated(g, b.getButtonText(), b.getLocalBounds().reduced(6, 2), 12,
                   b.getToggleState() ? cyan : white, b.getToggleState(), juce::Justification::centred);
}
void LibertyRomplerAudioProcessorEditor::LibertyTheme::drawRotarySlider(
    juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) {
    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h);
    const float radius = juce::jmax(8.0f, juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.41f);
    const auto centre = bounds.getCentre();
    const auto path = juce::Rectangle<float>(centre.x-radius, centre.y-radius, radius*2, radius*2);
    g.setColour(chrome); g.fillEllipse(path);
    g.setColour(stroke); g.drawEllipse(path, 1.4f);
    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                      start, start+(end-start)*pos, true);
    g.setColour(cyan); g.strokePath(arc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved,
                                                             juce::PathStrokeType::rounded));
    const float a = start + pos*(end-start);
    juce::Path needle;
    needle.startNewSubPath(centre);
    needle.lineTo(centre.x + std::sin(a)*(radius-9.0f), centre.y-std::cos(a)*(radius-9.0f));
    g.setColour(white); g.strokePath(needle, juce::PathStrokeType(2.0f));
}

void LibertyRomplerAudioProcessorEditor::setupLabel(
    juce::Label& label, const juce::String& text, float size, juce::Colour colour, bool bold) {
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)));
    label.setColour(juce::Label::textColourId, colour);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setMinimumHorizontalScale(0.75f);
}
void LibertyRomplerAudioProcessorEditor::setupSlider(
    juce::Slider& slider, juce::Label& label, const juce::String& name,
    double min, double max, double initial, bool logarithmic) {
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 20);
    slider.setRange(min, max, 0.001);
    if (logarithmic) slider.setSkewFactorFromMidPoint(1800.0);
    slider.setValue(initial, juce::dontSendNotification);
    slider.setColour(juce::Slider::textBoxTextColourId, white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, chrome);
    slider.setColour(juce::Slider::textBoxOutlineColourId, stroke);
    setupLabel(label, name, 10.0f, secondary, true);
    label.setJustificationType(juce::Justification::centred);
}

LibertyRomplerAudioProcessorEditor::LibertyRomplerAudioProcessorEditor(LibertyRomplerAudioProcessor& p)
  : juce::AudioProcessorEditor(p), processor(p),
    keyboard(p.getKeyboard(), juce::MidiKeyboardComponent::horizontalKeyboard), routingGraph(p),
    arpGrid(p.getArpControls()), sampleView(p) {
    setLookAndFeel(&theme);
    setOpaque(true);
    setSize(1360, 850);
    setResizable(true, true);
    setResizeLimits(1120, 750, 1920, 1120);

    setupLabel(brandLabel, "LIBERTY  ROMPLER", 22, white, true);
    setupLabel(brandSubLabel, "MUSIC SOFTWARE  /  INSTRUMENT MULTISAMPLE", 9, gold, true);
    setupLabel(headingLabel, utf(u8"Aucun pr\u00e9set charg\u00e9"), 20, white, true);
    setupLabel(subtitleLabel, "Prototype 0.14  |  AU / VST3 / STANDALONE", 10, quiet);
    setupLabel(resultCountLabel, "", 11, quiet);
    setupLabel(loadedPresetLabel, "", 15, gold, true);
    setupLabel(statusLabel, "", 11, secondary);
    for (auto* label : {&brandLabel,&brandSubLabel,&headingLabel,&subtitleLabel,
                         &resultCountLabel,&loadedPresetLabel,&statusLabel})
        addAndMakeVisible(*label);

    searchField.setTextToShowWhenEmpty(utf(u8"Rechercher un pr\u00e9set..."), quiet);
    searchField.setFont(juce::Font(juce::FontOptions(14.0f)));
    searchField.setColour(juce::TextEditor::backgroundColourId, chrome);
    searchField.setColour(juce::TextEditor::textColourId, white);
    searchField.setColour(juce::TextEditor::outlineColourId, stroke);
    searchField.setColour(juce::TextEditor::focusedOutlineColourId, cyan);
    searchField.setColour(juce::TextEditor::highlightColourId, selected);
    searchField.onTextChange = [this] { rebuildPresets(); };
    addAndMakeVisible(searchField);

    bankList.setModel(&bankModel);
    bankList.setRowHeight(65);
    bankList.setColour(juce::ListBox::backgroundColourId, panel);
    bankList.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(bankList);
    presetList.setModel(this);
    presetList.setRowHeight(48);
    presetList.setColour(juce::ListBox::backgroundColourId, panel);
    presetList.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(presetList);
    categoryViewport.setViewedComponent(&categoryContent, false);
    categoryViewport.setScrollBarsShown(false, true);
    categoryViewport.setScrollBarThickness(4);
    categoryViewport.setColour(juce::ScrollBar::thumbColourId, stroke);
    addAndMakeVisible(categoryViewport);

    favouriteFilter.setClickingTogglesState(false);
    favouriteFilter.onClick = [this] { setFavouriteFilter(!favouritesOnly); };
    favouriteToggle.onClick = [this] {
        const auto& banks = processor.getLibrary().getBanks();
        const auto bank = processor.getSelectedBankId(), preset = processor.getSelectedPresetId();
        for (size_t b = 0; b < banks.size(); ++b)
            for (size_t i = 0; i < banks[b].presets.size(); ++i)
                if (banks[b].presets[i].bankId == bank && banks[b].presets[i].id == preset) {
                    toggleFavourite((int)b, (int)i); return;
                }
    };
    rescanButton.onClick = [this] { processor.getLibrary().rescan(); rebuildBanks(); };
    folderButton.onClick = [this] {
        auto dir = processor.getLibrary().getDirectory();
        dir.createDirectory(); dir.revealToUser();
    };
    previousButton.onClick = [this] { chooseRelativePreset(-1); };
    nextButton.onClick = [this] { chooseRelativePreset(1); };
    for (auto* b : {&favouriteFilter,&favouriteToggle,&rescanButton,&folderButton,
                    &previousButton,&nextButton}) addAndMakeVisible(*b);

    auto& ctrl = p.getControls();
    setupSlider(cutoffSlider,cutoffLabel,"COUPURE",80.0,20000.0,ctrl.cutoffHz.load(),true);
    setupSlider(resonanceSlider,resonanceLabel,utf(u8"R\u00c9SONANCE"),0.5,4.0,ctrl.resonance.load());
    setupSlider(gainSlider,gainLabel,"VOLUME",0.0,1.5,ctrl.masterGain.load());
    setupSlider(attackSlider,attackLabel,"ATTAQUE",0.0,2.0,ctrl.attackSeconds.load());
    setupSlider(releaseSlider,releaseLabel,utf(u8"REL\u00c2CHEMENT"),0.02,5.0,ctrl.releaseSeconds.load());
    cutoffSlider.setTextValueSuffix(" Hz");
    for (auto* s : {&cutoffSlider,&resonanceSlider,&gainSlider,&attackSlider,&releaseSlider}) addAndMakeVisible(*s);
    for (auto* label : {&cutoffLabel,&resonanceLabel,&gainLabel,&attackLabel,&releaseLabel}) addAndMakeVisible(*label);
    cutoffSlider.onValueChange = [this] { processor.getControls().cutoffHz.store((float)cutoffSlider.getValue()); repaint(filterModule()); };
    resonanceSlider.onValueChange = [this] { processor.getControls().resonance.store((float)resonanceSlider.getValue()); repaint(filterModule()); };
    gainSlider.onValueChange = [this] { processor.getControls().masterGain.store((float)gainSlider.getValue()); };
    attackSlider.onValueChange = [this] {
        processor.getControls().attackSeconds.store((float)attackSlider.getValue());
        ampEnvSliders[0].setValue(attackSlider.getValue(), juce::dontSendNotification);
        repaint(ampModule());
    };
    releaseSlider.onValueChange = [this] {
        processor.getControls().releaseSeconds.store((float)releaseSlider.getValue());
        ampEnvSliders[3].setValue(releaseSlider.getValue(), juce::dontSendNotification);
        repaint(ampModule());
    };

    keyboard.setAvailableRange(24, 107); // Seven complete octaves
    keyboard.setLowestVisibleKey(24);
    keyboard.setScrollButtonsVisible(false);
    keyboard.setKeyWidth(26.0f);
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, cyan.withAlpha(0.6f));
    addAndMakeVisible(keyboard);
    // Navigate among functional pages without changing the MIDI audio engine.
    const auto makeTab = [this](juce::TextButton& button, Page page) {
        addAndMakeVisible(button);
        button.onClick = [this, page] { selectPage(page); };
    };
    makeTab(presetsTab, Page::Presets);
    makeTab(layersTab, Page::Layers);
    makeTab(routingTab, Page::Routing);
    makeTab(effectsTab, Page::Effects);
    makeTab(matrixTab, Page::Matrix);
    makeTab(modulatorsTab, Page::Modulators);
    makeTab(arpTab, Page::Arpeggiator);
    arpTab.setButtonText("ARP / STEP");
    makeTab(samplesTab, Page::Samples);
    makeTab(settingsTab, Page::Settings);
    addChildComponent(routingGraph);
    addChildComponent(arpGrid);
    addChildComponent(sampleView);
    auto& arp = processor.getArpControls();
    arpEnabled.setClickingTogglesState(true);
    arpEnabled.setToggleState(arp.enabled.load(), juce::dontSendNotification);
    arpEnabled.setButtonText(arp.enabled.load() ? "ARP ON" : "ARP OFF");
    arpEnabled.onClick = [this] {
        const bool value = arpEnabled.getToggleState();
        processor.getArpControls().enabled.store(value);
        arpEnabled.setButtonText(value ? "ARP ON" : "ARP OFF");
    };
    addChildComponent(arpEnabled);
    arpMode.addItem("UP",1); arpMode.addItem("DOWN",2);
    arpMode.addItem("RANDOM",3); arpMode.addItem("CHORD",4);
    arpMode.setSelectedId(arp.mode.load() + 1, juce::dontSendNotification);
    arpMode.onChange = [this] { processor.getArpControls().mode.store(juce::jlimit(0,3,arpMode.getSelectedId()-1)); };
    arpDivision.addItem("1/4",1); arpDivision.addItem("1/8",2);
    arpDivision.addItem("1/16",3); arpDivision.addItem("1/32",4);
    arpDivision.setSelectedId(arp.division.load()+1, juce::dontSendNotification);
    arpDivision.onChange = [this] { processor.getArpControls().division.store(juce::jlimit(0,3,arpDivision.getSelectedId()-1)); };
    addChildComponent(arpMode);
    addChildComponent(arpDivision);
    const char* knobTitles[4] = {"OCTAVES", "PAS", "SWING", "GATE"};
    const double knobMin[4] = {1,1,0,5}, knobMax[4] = {4,32,75,98};
    const double knobValue[4] = {(double)arp.octaves.load(),(double)arp.steps.load(),
                                 100.0*arp.swing.load(),100.0*arp.gate.load()};
    for(int i=0;i<4;++i) {
        auto& k=arpKnobs[(size_t)i];
        k.setSliderStyle(juce::Slider::LinearHorizontal);
        k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,70,18);
        k.setRange(knobMin[i],knobMax[i],1.0);
        k.setValue(knobValue[i],juce::dontSendNotification);
        k.setColour(juce::Slider::textBoxTextColourId,white);
        k.setColour(juce::Slider::textBoxBackgroundColourId,chrome);
        k.setColour(juce::Slider::textBoxOutlineColourId,stroke);
        setupLabel(arpLabels[(size_t)i+2],knobTitles[i],10,secondary,true);
        addChildComponent(k);
        addChildComponent(arpLabels[(size_t)i+2]);
    }
    setupLabel(arpLabels[0],"MODE",10,secondary,true);
    setupLabel(arpLabels[1],"DIVISION",10,secondary,true);
    addChildComponent(arpLabels[0]); addChildComponent(arpLabels[1]);
    arpKnobs[0].onValueChange = [this] { processor.getArpControls().octaves.store((int)arpKnobs[0].getValue()); };
    arpKnobs[1].onValueChange = [this] { processor.getArpControls().steps.store((int)arpKnobs[1].getValue()); arpGrid.repaint(); };
    arpKnobs[2].onValueChange = [this] { processor.getArpControls().swing.store((float)arpKnobs[2].getValue()/100.0f); };
    arpKnobs[3].onValueChange = [this] { processor.getArpControls().gate.store((float)arpKnobs[3].getValue()/100.0f); };
    settingsTab.setButtonText(utf(u8"R\u00c9GLAGES"));
    auto& performance = processor.getControls();
    const juce::String effectNames[18] = {
        "VITESSE", "PROFONDEUR", "MIX",
        "TAILLE", "AMORTI", "MIX",
        "TEMPS", "FEEDBACK", "MIX",
        "DRIVE", utf(u8"TONALIT\u00c9"), "MIX",
        "GRAVE", "MEDIUM", "AIGU",
        "SEUIL", "RATIO", "GAIN"
    };
    const double minimum[18] = {0.05,0,0, 0,0,0, 60,0,0, 0,500,0, -12,-12,-12, -48,1,0};
    const double maximum[18] = {8,1,1, 1,1,1, 1500,0.85,1, 30,18000,1, 12,12,12, 0,12,18};
    const double initial[18] = {performance.chorusRate.load(),performance.chorusDepth.load(),
        performance.chorusMix.load(),performance.reverbRoom.load(),performance.reverbDamping.load(),
        performance.reverbMix.load(),performance.delayTimeMs.load(),performance.delayFeedback.load(),
        performance.delayMix.load(),performance.distortionDriveDb.load(),performance.distortionToneHz.load(),
        performance.distortionMix.load(),performance.eqLowDb.load(),performance.eqMidDb.load(),
        performance.eqHighDb.load(),performance.compThresholdDb.load(),performance.compRatio.load(),
        performance.compMakeupDb.load()};
    for (int i=0; i<18; ++i) {
        setupSlider(effectSliders[(size_t)i], effectLabels[(size_t)i],
                    effectNames[i], minimum[i], maximum[i], initial[i]);
        addChildComponent(effectSliders[(size_t)i]);
        addChildComponent(effectLabels[(size_t)i]);
    }
    effectSliders[0].onValueChange = [this]{ processor.getControls().chorusRate = (float)effectSliders[0].getValue(); };
    effectSliders[1].onValueChange = [this]{ processor.getControls().chorusDepth = (float)effectSliders[1].getValue(); };
    effectSliders[2].onValueChange = [this]{ processor.getControls().chorusMix = (float)effectSliders[2].getValue(); };
    effectSliders[3].onValueChange = [this]{ processor.getControls().reverbRoom = (float)effectSliders[3].getValue(); };
    effectSliders[4].onValueChange = [this]{ processor.getControls().reverbDamping = (float)effectSliders[4].getValue(); };
    effectSliders[5].onValueChange = [this]{ processor.getControls().reverbMix = (float)effectSliders[5].getValue(); };
    effectSliders[6].setTextValueSuffix(" ms");
    effectSliders[9].setTextValueSuffix(" dB");
    effectSliders[10].setTextValueSuffix(" Hz");
    effectSliders[10].setSkewFactorFromMidPoint(4000.0);
    effectSliders[6].onValueChange = [this]{ processor.getControls().delayTimeMs = (float)effectSliders[6].getValue(); };
    effectSliders[7].onValueChange = [this]{ processor.getControls().delayFeedback = (float)effectSliders[7].getValue(); };
    effectSliders[8].onValueChange = [this]{ processor.getControls().delayMix = (float)effectSliders[8].getValue(); };
    effectSliders[9].onValueChange = [this]{ processor.getControls().distortionDriveDb = (float)effectSliders[9].getValue(); };
    effectSliders[10].onValueChange = [this]{ processor.getControls().distortionToneHz = (float)effectSliders[10].getValue(); };
    effectSliders[11].onValueChange = [this]{ processor.getControls().distortionMix = (float)effectSliders[11].getValue(); };
    for (int i=12; i<=14; ++i) effectSliders[(size_t)i].setTextValueSuffix(" dB");
    effectSliders[15].setTextValueSuffix(" dB");
    effectSliders[17].setTextValueSuffix(" dB");
    effectSliders[12].onValueChange = [this]{ processor.getControls().eqLowDb = (float)effectSliders[12].getValue(); };
    effectSliders[13].onValueChange = [this]{ processor.getControls().eqMidDb = (float)effectSliders[13].getValue(); };
    effectSliders[14].onValueChange = [this]{ processor.getControls().eqHighDb = (float)effectSliders[14].getValue(); };
    effectSliders[15].onValueChange = [this]{ processor.getControls().compThresholdDb = (float)effectSliders[15].getValue(); };
    effectSliders[16].onValueChange = [this]{ processor.getControls().compRatio = (float)effectSliders[16].getValue(); };
    effectSliders[17].onValueChange = [this]{ processor.getControls().compMakeupDb = (float)effectSliders[17].getValue(); };

    for (int i=0; i<4; ++i) {
        auto& source = sourceChoices[(size_t)i];
        auto& target = targetChoices[(size_t)i];
        source.addItem("Aucune", 1);
        source.addItem("LFO 1", 2);
        source.addItem("LFO 2", 3);
        source.addItem("Molette MIDI", 4);
        target.addItem("Aucune", 1);
        target.addItem("Coupure filtre", 2);
        target.addItem("Volume", 3);
        source.setSelectedId(performance.matrix[(size_t)i].source.load()+1, juce::dontSendNotification);
        target.setSelectedId(performance.matrix[(size_t)i].target.load()+1, juce::dontSendNotification);
        for (auto* cb : {&source,&target}) {
            cb->setColour(juce::ComboBox::backgroundColourId, chrome);
            cb->setColour(juce::ComboBox::textColourId, white);
            cb->setColour(juce::ComboBox::outlineColourId, stroke);
            addChildComponent(*cb);
        }
        source.onChange = [this, i] { processor.getControls().matrix[(size_t)i].source = sourceChoices[(size_t)i].getSelectedId()-1; };
        target.onChange = [this, i] { processor.getControls().matrix[(size_t)i].target = targetChoices[(size_t)i].getSelectedId()-1; };
        auto& amount = amountSliders[(size_t)i];
        amount.setSliderStyle(juce::Slider::LinearHorizontal);
        amount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 54, 22);
        amount.setRange(-100.0, 100.0, 1.0);
        amount.setTextValueSuffix(" %");
        amount.setValue(performance.matrix[(size_t)i].amount.load()*100.0f, juce::dontSendNotification);
        amount.setColour(juce::Slider::textBoxTextColourId, white);
        amount.setColour(juce::Slider::textBoxBackgroundColourId, chrome);
        amount.setColour(juce::Slider::textBoxOutlineColourId, stroke);
        amount.onValueChange = [this, i] {
            processor.getControls().matrix[(size_t)i].amount = (float)amountSliders[(size_t)i].getValue()/100.0f;
        };
        addChildComponent(amount);
    }
    setupSlider(lfoSliders[0], lfoLabels[0], "LFO 1 / VITESSE", 0.05, 12.0, performance.lfoOneHz.load());
    setupSlider(lfoSliders[1], lfoLabels[1], "LFO 2 / VITESSE", 0.05, 12.0, performance.lfoTwoHz.load());
    for (int i=0; i<2; ++i) {
        lfoSliders[(size_t)i].setSliderStyle(juce::Slider::LinearHorizontal);
        lfoSliders[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 22);
        lfoSliders[(size_t)i].setTextValueSuffix(" Hz");
        addChildComponent(lfoLabels[(size_t)i]);
        addChildComponent(lfoSliders[(size_t)i]);
    }
    lfoSliders[0].onValueChange = [this] { processor.getControls().lfoOneHz = (float)lfoSliders[0].getValue(); };
    lfoSliders[1].onValueChange = [this] { processor.getControls().lfoTwoHz = (float)lfoSliders[1].getValue(); };

    // A separate Modulateurs page owns two full ADSRs and waveform controls.
    const std::array<juce::String,4> envNames {{"ATTAQUE", utf(u8"D\u00c9CLIN"), "MAINTIEN", utf(u8"REL\u00c2CHEMENT")}};
    const double ampMinimum[4] = {0.0, 0.01, 0.0, 0.02};
    const double ampMaximum[4] = {2.0, 5.0, 1.0, 5.0};
    const double ampInitial[4] = {performance.attackSeconds.load(), performance.decaySeconds.load(),
                                  performance.sustainLevel.load(), performance.releaseSeconds.load()};
    const double filtMinimum[4] = {0.001, 0.01, 0.0, 0.01};
    const double filtMaximum[4] = {5.0, 5.0, 1.0, 8.0};
    const double filtInitial[4] = {performance.filterAttack.load(), performance.filterDecay.load(),
                                   performance.filterSustain.load(), performance.filterRelease.load()};
    for (int i=0; i<4; ++i) {
        setupSlider(ampEnvSliders[(size_t)i],ampEnvLabels[(size_t)i],envNames[(size_t)i],
                    ampMinimum[i],ampMaximum[i],ampInitial[i]);
        setupSlider(filterEnvSliders[(size_t)i],filterEnvLabels[(size_t)i],envNames[(size_t)i],
                    filtMinimum[i],filtMaximum[i],filtInitial[i]);
        ampEnvSliders[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,61,18);
        filterEnvSliders[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,61,18);
        for (auto* component : {static_cast<juce::Component*>(&ampEnvSliders[(size_t)i]),
                                static_cast<juce::Component*>(&ampEnvLabels[(size_t)i]),
                                static_cast<juce::Component*>(&filterEnvSliders[(size_t)i]),
                                static_cast<juce::Component*>(&filterEnvLabels[(size_t)i])})
            addChildComponent(*component);
    }
    setupSlider(filterEnvDepthSlider,filterEnvDepthLabel,"PROFONDEUR",-100.0,100.0,
                performance.filterEnvDepth.load()*100.0f);
    filterEnvDepthSlider.setTextValueSuffix(" %");
    filterEnvDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,61,18);
    addChildComponent(filterEnvDepthSlider);
    addChildComponent(filterEnvDepthLabel);
    ampEnvSliders[0].onValueChange = [this] {
        processor.getControls().attackSeconds = (float)ampEnvSliders[0].getValue();
        attackSlider.setValue(ampEnvSliders[0].getValue(),juce::dontSendNotification);
    };
    ampEnvSliders[1].onValueChange = [this] { processor.getControls().decaySeconds = (float)ampEnvSliders[1].getValue(); };
    ampEnvSliders[2].onValueChange = [this] { processor.getControls().sustainLevel = (float)ampEnvSliders[2].getValue(); };
    ampEnvSliders[3].onValueChange = [this] {
        processor.getControls().releaseSeconds = (float)ampEnvSliders[3].getValue();
        releaseSlider.setValue(ampEnvSliders[3].getValue(),juce::dontSendNotification);
    };
    filterEnvSliders[0].onValueChange = [this] {processor.getControls().filterAttack = (float)filterEnvSliders[0].getValue();};
    filterEnvSliders[1].onValueChange = [this] {processor.getControls().filterDecay = (float)filterEnvSliders[1].getValue();};
    filterEnvSliders[2].onValueChange = [this] {processor.getControls().filterSustain = (float)filterEnvSliders[2].getValue();};
    filterEnvSliders[3].onValueChange = [this] {processor.getControls().filterRelease = (float)filterEnvSliders[3].getValue();};
    filterEnvDepthSlider.onValueChange = [this] {
        processor.getControls().filterEnvDepth = (float)filterEnvDepthSlider.getValue()/100.0f;
    };
    for (int i=0; i<2; ++i) {
        auto& box=lfoShapeChoices[(size_t)i];
        box.addItem(utf(u8"Sinuso\u00efdale"),1);
        box.addItem("Triangle",2);
        box.addItem("Dent de scie",3);
        box.addItem(utf(u8"Carr\u00e9e"),4);
        box.setSelectedId((i==0 ? performance.lfoOneWave.load() : performance.lfoTwoWave.load())+1,
                          juce::dontSendNotification);
        box.setColour(juce::ComboBox::backgroundColourId,chrome);
        box.setColour(juce::ComboBox::textColourId,white);
        box.setColour(juce::ComboBox::outlineColourId,stroke);
        box.onChange = [this,i] {
            const int choice = juce::jlimit(0,3,lfoShapeChoices[(size_t)i].getSelectedId()-1);
            if(i==0) processor.getControls().lfoOneWave.store(choice);
            else processor.getControls().lfoTwoWave.store(choice);
            repaint(modulatorCards()[(size_t)i+2]);
        };
        addChildComponent(box);
    }

    // Four genuinely independent multi-sample layers. All controls directly drive voices.
    const std::array<juce::String, 8> names {{
        "VOLUME", "PAN", "DEMI-TONS", "COUPURE",
        "NOTE MIN", "NOTE MAX", utf(u8"V\u00c9L. MIN"), utf(u8"V\u00c9L. MAX")
    }};
    const std::array<double, 8> low {{0.0, -100.0, -24.0, 80.0, 0.0, 0.0, 1.0, 1.0}};
    const std::array<double, 8> high {{150.0, 100.0, 24.0, 20000.0, 127.0, 127.0, 127.0, 127.0}};
    for (int i=0; i<4; ++i) {
        auto& select = layerSources[(size_t)i];
        select.setColour(juce::ComboBox::backgroundColourId, chrome);
        select.setColour(juce::ComboBox::textColourId, white);
        select.setColour(juce::ComboBox::outlineColourId, stroke);
        addChildComponent(select);
        select.onChange = [this, i] {
            const int id = layerSources[(size_t)i].getSelectedId();
            juce::Result result = juce::Result::ok();
            if (id <= 1) result = processor.setLayerSource(i, {}, {});
            else if ((size_t)(id-2) < sourceOptions.size()) {
                const auto& ref = sourceOptions[(size_t)(id-2)];
                result = processor.setLayerSource(i, ref.first, ref.second);
            }
            if (result.failed()) {
                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                    "Liberty Rompler", result.getErrorMessage());
            }
            refreshLayerControls();
        };
        auto& toggle = layerEnabled[(size_t)i];
        toggle.setButtonText("ACTIF");
        toggle.setClickingTogglesState(true);
        toggle.onClick = [this, i] {
            const auto src = processor.getLayerSource(i);
            auto enabled = layerEnabled[(size_t)i].getToggleState() && src.presetId.isNotEmpty();
            processor.getControls().layers[(size_t)i].enabled.store(enabled);
            layerEnabled[(size_t)i].setToggleState(enabled, juce::dontSendNotification);
            repaint(layerCards()[(size_t)i]);
        };
        addChildComponent(toggle);
        auto& layer = performance.layers[(size_t)i];
        const std::array<double, 8> values {{
            (double)layer.gain.load()*100.0, (double)layer.pan.load()*100.0,
            (double)layer.transpose.load(), (double)layer.cutoffHz.load(),
            (double)layer.lowNote.load(), (double)layer.highNote.load(),
            (double)layer.lowVelocity.load(), (double)layer.highVelocity.load()
        }};
        for (int j=0; j<8; ++j) {
            auto& label = layerLabels[(size_t)i][(size_t)j];
            auto& slider = layerSliders[(size_t)i][(size_t)j];
            setupLabel(label,names[(size_t)j],10.0f,secondary,true);
            label.setJustificationType(juce::Justification::centredLeft);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight,false,53,18);
            slider.setRange(low[(size_t)j],high[(size_t)j],j==3 ? 1.0 : 1.0);
            slider.setValue(values[(size_t)j],juce::dontSendNotification);
            slider.setColour(juce::Slider::textBoxTextColourId,white);
            slider.setColour(juce::Slider::textBoxBackgroundColourId,chrome);
            slider.setColour(juce::Slider::textBoxOutlineColourId,stroke);
            if (j==3) {slider.setSkewFactorFromMidPoint(1800.0);slider.setTextValueSuffix(" Hz");}
            if (j==0 || j==1) slider.setTextValueSuffix(" %");
            slider.onValueChange = [this,i,j] {
                auto& ctrl = processor.getControls().layers[(size_t)i];
                const auto v = (float)layerSliders[(size_t)i][(size_t)j].getValue();
                switch (j) {
                    case 0: ctrl.gain.store(v/100.0f); break;
                    case 1: ctrl.pan.store(v/100.0f); break;
                    case 2: ctrl.transpose.store((int)v); break;
                    case 3: ctrl.cutoffHz.store(v); break;
                    case 4:
                        ctrl.lowNote.store((int)v);
                        if ((int)v > ctrl.highNote.load()) {
                            ctrl.highNote.store((int)v);
                            layerSliders[(size_t)i][5].setValue(v,juce::dontSendNotification);
                        } break;
                    case 5:
                        ctrl.highNote.store((int)v);
                        if ((int)v < ctrl.lowNote.load()) {
                            ctrl.lowNote.store((int)v);
                            layerSliders[(size_t)i][4].setValue(v,juce::dontSendNotification);
                        } break;
                    case 6:
                        ctrl.lowVelocity.store((int)v);
                        if ((int)v > ctrl.highVelocity.load()) {
                            ctrl.highVelocity.store((int)v);
                            layerSliders[(size_t)i][7].setValue(v,juce::dontSendNotification);
                        } break;
                    case 7:
                        ctrl.highVelocity.store((int)v);
                        if ((int)v < ctrl.lowVelocity.load()) {
                            ctrl.lowVelocity.store((int)v);
                            layerSliders[(size_t)i][6].setValue(v,juce::dontSendNotification);
                        } break;
                }
            };
            addChildComponent(label);
            addChildComponent(slider);
        }
    }
    settingsOpen.onClick = [this] { auto dir=processor.getLibrary().getDirectory(); dir.createDirectory(); dir.revealToUser(); };
    settingsScan.onClick = [this] { processor.getLibrary().rescan(); rebuildBanks(); };
    addChildComponent(settingsOpen);
    addChildComponent(settingsScan);
    selectPage(Page::Presets);
    loadFavourites();
    rebuildBanks();
}
LibertyRomplerAudioProcessorEditor::~LibertyRomplerAudioProcessorEditor() {
    categoryViewport.setViewedComponent(nullptr, false);
    categoryButtons.clear();
    presetList.setModel(nullptr);
    bankList.setModel(nullptr);
    setLookAndFeel(nullptr);
}

LibertyRomplerAudioProcessorEditor::Panels LibertyRomplerAudioProcessorEditor::panels() const {
    const int margin=14, gap=12, top=125, bottom=146;
    const int w = getWidth(), mainH = juce::jmax(0, getHeight()-top-bottom);
    const int side = juce::jlimit(210, 260, (int)(w*0.188));
    const int right = juce::jlimit(298, 370, (int)(w*0.258));
    const int centre = juce::jmax(0, w-2*margin-2*gap-side-right);
    Panels p;
    p.sidebar={margin,top,side,mainH};
    p.browser={margin+side+gap,top,centre,mainH};
    p.modifiers={margin+side+gap+centre+gap,top,right,mainH};
    p.footer={0,getHeight()-129,w,116};
    return p;
}
juce::Rectangle<int> LibertyRomplerAudioProcessorEditor::filterModule() const {
    auto r=panels().modifiers;
    return {r.getX()+11,r.getY()+43,r.getWidth()-22,166};
}
juce::Rectangle<int> LibertyRomplerAudioProcessorEditor::ampModule() const {
    auto r=filterModule(); return r.translated(0,176);
}
juce::Rectangle<int> LibertyRomplerAudioProcessorEditor::infoModule() const {
    const auto r=panels().modifiers, a=ampModule();
    return {r.getX()+11,a.getBottom()+10,r.getWidth()-22,juce::jmax(0,r.getBottom()-12-a.getBottom()-10)};
}
juce::Rectangle<int> LibertyRomplerAudioProcessorEditor::workArea() const {
    const auto p = panels();
    return {p.browser.getX(), p.browser.getY(), p.modifiers.getRight()-p.browser.getX(), p.browser.getHeight()};
}
std::array<juce::Rectangle<int>, 6>
LibertyRomplerAudioProcessorEditor::effectCards() const {
    const auto bounds=workArea().reduced(18,16);
    const int gap=12, width=(bounds.getWidth()-2*gap)/3;
    const int height=juce::jmax(170,juce::jmin(275,(bounds.getHeight()-48-gap-28)/2));
    const int top=bounds.getY()+48;
    return {{
        {bounds.getX(),top,width,height},
        {bounds.getX()+width+gap,top,width,height},
        {bounds.getX()+2*(width+gap),top,width,height},
        {bounds.getX(),top+height+gap,width,height},
        {bounds.getX()+width+gap,top+height+gap,width,height},
        {bounds.getX()+2*(width+gap),top+height+gap,width,height}
    }};
}
std::array<juce::Rectangle<int>,4>
LibertyRomplerAudioProcessorEditor::modulatorCards() const {
    const auto bounds=workArea().reduced(18,16);
    const int gap=14, width=(bounds.getWidth()-gap)/2;
    const int height=juce::jmax(180,juce::jmin(285,(bounds.getHeight()-48-gap-28)/2));
    const int top=bounds.getY()+48;
    return {{
        {bounds.getX(),top,width,height},
        {bounds.getX()+width+gap,top,width,height},
        {bounds.getX(),top+height+gap,width,height},
        {bounds.getX()+width+gap,top+height+gap,width,height}
    }};
}
std::array<juce::Rectangle<int>, 4> LibertyRomplerAudioProcessorEditor::layerCards() const {
    return modulatorCards();
}
void LibertyRomplerAudioProcessorEditor::selectPage(Page next) {
    activePage=next;
    presetsTab.setToggleState(next==Page::Presets,juce::dontSendNotification);
    layersTab.setToggleState(next==Page::Layers,juce::dontSendNotification);
    routingTab.setToggleState(next==Page::Routing,juce::dontSendNotification);
    arpTab.setToggleState(next==Page::Arpeggiator,juce::dontSendNotification);
    arpGrid.setVisible(next==Page::Arpeggiator);
    arpEnabled.setVisible(next==Page::Arpeggiator);
    arpMode.setVisible(next==Page::Arpeggiator);
    arpDivision.setVisible(next==Page::Arpeggiator);
    for(auto& k:arpKnobs)k.setVisible(next==Page::Arpeggiator);
    for(auto& l:arpLabels)l.setVisible(next==Page::Arpeggiator);
    routingGraph.setVisible(next==Page::Routing);
    samplesTab.setToggleState(next==Page::Samples,juce::dontSendNotification);
    sampleView.setVisible(next==Page::Samples);
    if (next==Page::Samples) sampleView.refresh();
    effectsTab.setToggleState(next==Page::Effects,juce::dontSendNotification);
    matrixTab.setToggleState(next==Page::Matrix,juce::dontSendNotification);
    modulatorsTab.setToggleState(next==Page::Modulators,juce::dontSendNotification);
    settingsTab.setToggleState(next==Page::Settings,juce::dontSendNotification);
    const bool presets=next==Page::Presets;
    for (juce::Component* c : {static_cast<juce::Component*>(&searchField),
             static_cast<juce::Component*>(&presetList),
             static_cast<juce::Component*>(&categoryViewport),
             static_cast<juce::Component*>(&favouriteToggle),
             static_cast<juce::Component*>(&resultCountLabel),
             static_cast<juce::Component*>(&loadedPresetLabel),
             static_cast<juce::Component*>(&statusLabel),
             static_cast<juce::Component*>(&cutoffSlider),
             static_cast<juce::Component*>(&resonanceSlider),
             static_cast<juce::Component*>(&gainSlider),
             static_cast<juce::Component*>(&attackSlider),
             static_cast<juce::Component*>(&releaseSlider),
             static_cast<juce::Component*>(&cutoffLabel),
             static_cast<juce::Component*>(&resonanceLabel),
             static_cast<juce::Component*>(&gainLabel),
             static_cast<juce::Component*>(&attackLabel),
             static_cast<juce::Component*>(&releaseLabel)}) c->setVisible(presets);
    for (int i=0; i<4; ++i) {
        layerSources[(size_t)i].setVisible(next==Page::Layers);
        layerEnabled[(size_t)i].setVisible(next==Page::Layers);
        for (int j=0; j<8; ++j) {
            layerSliders[(size_t)i][(size_t)j].setVisible(next==Page::Layers);
            layerLabels[(size_t)i][(size_t)j].setVisible(next==Page::Layers);
        }
    }
    for (size_t i=0;i<18;++i) {
        effectSliders[i].setVisible(next==Page::Effects);
        effectLabels[i].setVisible(next==Page::Effects);
    }
    for (size_t i=0; i<4; ++i) {
        ampEnvSliders[i].setVisible(next==Page::Modulators);
        ampEnvLabels[i].setVisible(next==Page::Modulators);
        filterEnvSliders[i].setVisible(next==Page::Modulators);
        filterEnvLabels[i].setVisible(next==Page::Modulators);
    }
    filterEnvDepthSlider.setVisible(next==Page::Modulators);
    filterEnvDepthLabel.setVisible(next==Page::Modulators);
    for (auto& c : lfoShapeChoices) c.setVisible(next==Page::Modulators);
    for (size_t i=0;i<4;++i) {
        sourceChoices[i].setVisible(next==Page::Matrix);
        targetChoices[i].setVisible(next==Page::Matrix);
        amountSliders[i].setVisible(next==Page::Matrix);
    }
    for (size_t i=0;i<2;++i) {
        lfoSliders[i].setVisible(next==Page::Modulators);
        lfoLabels[i].setVisible(next==Page::Modulators);
    }
    settingsOpen.setVisible(next==Page::Settings);
    settingsScan.setVisible(next==Page::Settings);
    resized();
    repaint();
}
void LibertyRomplerAudioProcessorEditor::paintPanel(juce::Graphics& g, juce::Rectangle<int> area, juce::Colour colour) const {
    if (area.isEmpty()) return;
    g.setColour(colour);
    g.fillRoundedRectangle(area.toFloat(),10.0f);
    g.setColour(stroke.withAlpha(0.65f));
    g.drawRoundedRectangle(area.toFloat().reduced(0.5f),10.0f,1.0f);
}
void LibertyRomplerAudioProcessorEditor::paintFilterGraph(juce::Graphics& g, juce::Rectangle<float> rect) const {
    if (rect.isEmpty()) return;
    g.setColour(chrome);g.fillRoundedRectangle(rect,5.0f);
    g.setColour(stroke.withAlpha(0.5f));
    for(int i=1;i<5;++i) {
        float x=rect.getX()+rect.getWidth()*(float)i/5.0f;
        g.drawVerticalLine((int)x,rect.getY()+5.0f,rect.getBottom()-5.0f);
    }
    const auto cutoff=juce::jlimit(80.0,20000.0,(double)cutoffSlider.getValue());
    const float normalized=(float)(std::log(cutoff/80.0)/std::log(20000.0/80.0));
    const float cx=rect.getX()+rect.getWidth()*(0.05f+normalized*0.89f);
    juce::Path curve;
    const auto yTop=rect.getY()+rect.getHeight()*0.32f;
    for(int i=0;i<=70;++i) {
        const float x=rect.getX()+rect.getWidth()*(float)i/70.0f;
        const float norm=(x-cx)/juce::jmax(5.0f,rect.getWidth()*0.06f);
        const float roll=1.0f/(1.0f+std::exp(-norm));
        const float y=yTop+roll*rect.getHeight()*0.58f;
        if (i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);
    }
    g.setColour(cyan);g.strokePath(curve,juce::PathStrokeType(2.0f));
    g.setColour(cyan.withAlpha(0.45f));g.fillEllipse(cx-2.5f,yTop-2.5f,5.0f,5.0f);
}
void LibertyRomplerAudioProcessorEditor::paintAmpGraph(juce::Graphics& g, juce::Rectangle<float> rect) const {
    if (rect.isEmpty()) return;
    g.setColour(chrome);g.fillRoundedRectangle(rect,5.0f);
    const float a=juce::jlimit(0.04f,0.35f,0.06f+((float)attackSlider.getValue()/2.0f)*0.29f);
    const float r=juce::jlimit(0.06f,0.32f,0.07f+((float)releaseSlider.getValue()/5.0f)*0.25f);
    auto p=[rect](float x,float level){return juce::Point<float>(rect.getX()+rect.getWidth()*x,
                            rect.getBottom()-7.0f-level*(rect.getHeight()-14.0f));};
    juce::Path path;
    const auto p0=p(0.02f,0.01f), p1=p(a,0.95f), p2=p(a+0.12f,0.67f), p3=p(0.85f-r,0.67f),p4=p(0.95f,0.01f);
    path.startNewSubPath(p0);path.lineTo(p1);path.lineTo(p2);path.lineTo(p3);path.lineTo(p4);
    g.setColour(warm);g.strokePath(path,juce::PathStrokeType(2.0f));
}
void LibertyRomplerAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(bg);
    const auto p=panels();
    g.setColour(chrome);g.fillRect(0,0,getWidth(),119);
    g.setColour(cyan.withAlpha(0.72f));g.fillRect(0,69,getWidth(),2);
    g.setColour(stroke.withAlpha(0.8f));g.drawHorizontalLine(118,0.0f,(float)getWidth());
    paintPanel(g,p.sidebar,panel);
    if (activePage==Page::Presets) {
        paintPanel(g,p.browser,panel);paintPanel(g,p.modifiers,panel);
    } else paintPanel(g,workArea(),panel);
    paintPanel(g,p.footer,chrome);
    paintTruncated(g,utf(u8"BIBLIOTH\u00c8QUES"),{p.sidebar.getX()+15,p.sidebar.getY()+10,p.sidebar.getWidth()-30,23},13,white,true);
    paintTruncated(g,"BANQUES ET EXTENSIONS",{p.sidebar.getX()+15,p.sidebar.getY()+51,p.sidebar.getWidth()-30,18},10,quiet,true);
    if (activePage==Page::Presets) {
    paintTruncated(g,"EXPLORER LES PRESETS",{p.browser.getX()+16,p.browser.getY()+12,p.browser.getWidth()-172,25},14,white,true);
    paintTruncated(g,utf(u8"CAT\u00c9GORIES"),{p.browser.getX()+16,p.browser.getY()+50,p.browser.getWidth()-30,18},10,quiet,true);
    const int bodyW=p.browser.getWidth()-24-19-36;
    const int x=p.browser.getX()+25;
    const int nameW=bodyW*44/100,catW=bodyW*21/100;
    paintTruncated(g,"NOM",{x,p.browser.getY()+181,nameW-6,18},10,secondary,true);
    paintTruncated(g,"TYPE",{x+nameW,p.browser.getY()+181,catW-6,18},10,secondary,true);
    paintTruncated(g,"BANQUE",{x+nameW+catW,p.browser.getY()+181,bodyW-nameW-catW-6,18},10,secondary,true);
    g.setColour(stroke);g.drawHorizontalLine(p.browser.getY()+204,(float)x,(float)p.browser.getRight()-17);
    paintTruncated(g,"MODIFICATEURS",{p.modifiers.getX()+14,p.modifiers.getY()+12,p.modifiers.getWidth()-28,23},14,white,true);
    paintPanel(g,filterModule(),module);paintPanel(g,ampModule(),module);paintPanel(g,infoModule(),module);
    auto f=filterModule(),a=ampModule(),i=infoModule();
    paintTruncated(g,"FILTRE  |  LOW PASS",{f.getX()+13,f.getY()+10,f.getWidth()-26,20},11,cyan,true);
    paintTruncated(g,"AMPLITUDE  |  ADSR",{a.getX()+13,a.getY()+10,a.getWidth()-26,20},11,warm,true);
    paintFilterGraph(g,juce::Rectangle<float>((float)f.getX()+13,(float)f.getY()+34,(float)f.getWidth()-26,52));
    paintAmpGraph(g,juce::Rectangle<float>((float)a.getX()+13,(float)a.getY()+34,(float)a.getWidth()-26,52));
    if(i.getHeight()>50) {
        paintTruncated(g,utf(u8"D\u00c9TAILS DU SON"),{i.getX()+13,i.getY()+11,i.getWidth()-26,19},11,gold,true);
        auto* pr=processor.getLibrary().find(processor.getSelectedBankId(),processor.getSelectedPresetId());
        if(pr) {
            paintTruncated(g,pr->bankName,{i.getX()+13,i.getY()+42,i.getWidth()-26,21},14,white,true);
            paintTruncated(g,utf(u8"Cat\u00e9gorie : ")+pr->category,{i.getX()+13,i.getY()+65,i.getWidth()-26,17},11,secondary);
            if(i.getHeight()>110)
                paintTruncated(g,pr->layers.empty() ? juce::String((int)pr->zones.size())+" zones multisamples" : juce::String((int)pr->layers.size())+" couches sonores",{i.getX()+13,i.getY()+87,i.getWidth()-26,18},11,green);
        } else paintTruncated(g,utf(u8"S\u00e9lectionnez un pr\u00e9set."),{i.getX()+13,i.getY()+45,i.getWidth()-26,24},12,secondary);
    }
    } else {
        const auto work=workArea();
        const auto heading=activePage==Page::Layers ? "COUCHES MULTISAMPLES" :
                           activePage==Page::Routing ? "ROUTAGE AUDIO" :
                           activePage==Page::Arpeggiator ? "ARP / STEP  /  32 PAS" :
                           activePage==Page::Samples ? "SAMPLES  /  MULTISAMPLES" :
                           activePage==Page::Effects ? "EFFETS AUDIO" :
                           activePage==Page::Matrix ? "MATRICE DE MODULATION" :
                           activePage==Page::Modulators ? "MODULATEURS" :
                           utf(u8"R\u00c9GLAGES DU ROMPLER");
        paintTruncated(g,heading,{work.getX()+22,work.getY()+15,work.getWidth()-44,27},18,white,true);
        if (activePage==Page::Layers) {
            const auto cards = layerCards();
            const std::array<juce::Colour, 4> accents {{cyan,gold,purple,green}};
            for (int i=0; i<4; ++i) {
                const auto card=cards[(size_t)i];
                paintPanel(g,card,module);
                paintTruncated(g,"COUCHE "+juce::String(i+1),
                    {card.getX()+15,card.getY()+8,card.getWidth()-122,24},13,accents[(size_t)i],true);
            }
            paintTruncated(g,utf(u8"Combinez jusqu'\u00e0 quatre sources et ajustez leur clavier et v\u00e9locit\u00e9."),
                {work.getX()+22,work.getBottom()-34,work.getWidth()-44,23},11,secondary);
        } else if (activePage==Page::Routing) {
            // Graph and interaction labels are drawn by the RoutingGraph child.
        } else if (activePage==Page::Arpeggiator) {
            const auto bounds = work.reduced(17,8).withTrimmedTop(41).withTrimmedBottom(9);
            g.setColour(module);
            g.fillRoundedRectangle(bounds.toFloat(),9.0f);
            paintTruncated(g,utf(u8"32 PAS  /  V\u00c9LOCIT\u00c9  /  TRANSPOSITION"),
                {bounds.getX()+14,bounds.getY()+106,bounds.getWidth()-28,22},12,cyan,true);
            paintTruncated(g,utf(u8"Glisser : v\u00e9locit\u00e9     Maj + glisser : transposition     Clic droit : pause du pas"),
                {bounds.getX()+14,bounds.getBottom()-39,bounds.getWidth()-28,22},11,secondary);
            paintTruncated(g,utf(u8"Synchronis\u00e9 au tempo DAW (PPQ si disponible)  /  Horloge de secours 120 BPM"),
                {bounds.getX()+14,bounds.getBottom()-20,bounds.getWidth()-28,16},10,quiet);
        } else if (activePage==Page::Samples) {
            // Dedicated SampleZoneView draws the true WAV zones and waveform.
        } else if (activePage==Page::Effects) {
            const auto cards=effectCards();
            for (const auto& card : cards) paintPanel(g,card,module);
            const std::array<juce::String,6> titles {{
                utf(u8"CHORUS ST\u00c9R\u00c9O"),
                utf(u8"R\u00c9VERB\u00c9RATION"),
                utf(u8"DELAY ST\u00c9R\u00c9O"),
                "DISTORSION", "EQ 3 BANDES", "COMPRESSEUR"
            }};
            const std::array<juce::String,6> descriptions {{
                "Mouvement et largeur sonore",
                "Taille de salle et ambiance",
                utf(u8"R\u00e9p\u00e9titions et feedback"),
                utf(u8"Saturation et tonalit\u00e9"),
                utf(u8"Graves, m\u00e9diums, aigus"),
                "Seuil, ratio et gain de sortie"
            }};
            const std::array<juce::Colour,6> accents {{cyan,gold,purple,warm,green,cyan}};
            for (size_t i=0; i<cards.size(); ++i) {
                const auto& card=cards[i];
                paintTruncated(g,titles[i],
                    {card.getX()+16,card.getY()+11,card.getWidth()-32,24},14,accents[i],true);
                paintTruncated(g,descriptions[i],
                    {card.getX()+16,card.getY()+43,card.getWidth()-32,22},11,secondary);
            }
            paintTruncated(g,utf(u8"Effets actifs sur le signal et m\u00e9moris\u00e9s dans les projets DAW."),
               {work.getX()+22,work.getBottom()-38,work.getWidth()-44,24},11,secondary);
        } else if (activePage==Page::Modulators) {
            const auto cards=modulatorCards();
            const std::array<juce::String,4> names {{
                "ENVELOPPE AMPLITUDE", "ENVELOPPE FILTRE", "LFO 1", "LFO 2"
            }};
            for (size_t i=0; i<cards.size(); ++i) {
                paintPanel(g,cards[i],module);
                paintTruncated(g,names[i],{cards[i].getX()+16,cards[i].getY()+10,
                    cards[i].getWidth()-32,24},13,i<2?gold:cyan,true);
                const auto graph=juce::Rectangle<float>((float)cards[i].getX()+15,
                    (float)cards[i].getY()+38,(float)cards[i].getWidth()-30,55);
                g.setColour(chrome); g.fillRoundedRectangle(graph,5.0f);
                juce::Path plot;
                for (int step=0; step<=96; ++step) {
                    const float t=(float)step/96.0f;
                    float val=0.0f;
                    if(i<2) {
                        const auto& envKnobs = i==0 ? ampEnvSliders : filterEnvSliders;
                        const float att=juce::jlimit(0.03f,0.28f,0.03f+(float)envKnobs[0].getValue()/5.0f);
                        const float dec=juce::jlimit(0.04f,0.20f,(float)envKnobs[1].getValue()/12.0f);
                        const float sustain=(float)envKnobs[2].getValue();
                        const float rel=juce::jlimit(0.07f,0.27f,(float)envKnobs[3].getValue()/12.0f);
                        if(t<att) val=t/att;
                        else if(t<att+dec) val=1.0f-(1.0f-sustain)*(t-att)/dec;
                        else if(t<1.0f-rel) val=sustain;
                        else val=sustain*juce::jlimit(0.0f,1.0f,(1.0f-t)/rel);
                    } else {
                        const auto wave=lfoShapeChoices[i-2].getSelectedId()-1;
                        const float phase=std::fmod(t*3.0f,1.0f);
                        switch(wave) {
                            case 1: val=1.0f-4.0f*std::abs(phase-0.5f); break;
                            case 2: val=2.0f*phase-1.0f; break;
                            case 3: val=phase<0.5f?1.0f:-1.0f; break;
                            default: val=std::sin(juce::MathConstants<float>::twoPi*phase); break;
                        }
                        val=val*0.48f+0.5f;
                    }
                    const float x=graph.getX()+5.0f+t*(graph.getWidth()-10.0f);
                    const float y=graph.getBottom()-5.0f-val*(graph.getHeight()-10.0f);
                    if(step==0) plot.startNewSubPath(x,y); else plot.lineTo(x,y);
                }
                g.setColour(i<2?gold:cyan);g.strokePath(plot,juce::PathStrokeType(1.7f));
                if(i>=2) paintTruncated(g,"FORME D'ONDE",{cards[i].getCentreX()+12,
                    cards[i].getY()+100,cards[i].getWidth()/2-28,18},10,secondary,true);
            }
            paintTruncated(g,utf(u8"Enveloppes et LFO enregistr\u00e9s avec le projet DAW."),
                {work.getX()+22,work.getBottom()-34,work.getWidth()-44,23},11,secondary);
        } else if (activePage==Page::Matrix) {
            const int left=work.getX()+24,width=work.getWidth()-48;
            const int sourceW=width*30/100,targetW=width*30/100,amountW=width-sourceW-targetW-24;
            paintTruncated(g,"SOURCE",{left,work.getY()+62,sourceW,20},11,secondary,true);
            paintTruncated(g,"DESTINATION",{left+sourceW+12,work.getY()+62,targetW,20},11,secondary,true);
            paintTruncated(g,utf(u8"QUANTIT\u00c9"),
               {left+sourceW+targetW+24,work.getY()+62,amountW,20},11,secondary,true);
            for (int i=0;i<4;++i) {
                auto row=juce::Rectangle<int>(left,work.getY()+88+i*65,width,55);
                g.setColour(i%2?module:chrome);
                g.fillRoundedRectangle(row.toFloat(),6.0f);
            }
            paintTruncated(g,utf(u8"LFO 1, LFO 2 et molette MIDI (CC1) : 4 routages ind\u00e9pendants."),
               {left,work.getBottom()-28,width,22},11,secondary);
        } else {
            const auto area=work.reduced(22,65);
            const auto card=juce::Rectangle<int>(area.getX(),area.getY(),area.getWidth(),
                                                juce::jmin(205,area.getHeight()/2));
            paintPanel(g,card,module);
            paintTruncated(g,utf(u8"BIBLIOTH\u00c8QUES ET EXTENSIONS"),
                {card.getX()+18,card.getY()+18,card.getWidth()-36,26},14,cyan,true);
            paintTruncated(g,utf(u8"Les banques sont charg\u00e9es depuis le dossier de l'utilisateur."),
                {card.getX()+18,card.getY()+52,card.getWidth()-36,24},12,secondary);
            paintTruncated(g,utf(u8"Actualisez la biblioth\u00e8que apr\u00e8s l'installation d'une extension."),
                {card.getX()+18,card.getY()+78,card.getWidth()-36,24},12,secondary);
            paintTruncated(g,"AU / VST3 / STANDALONE  -  VERSION DE TEST",
                {area.getX()+18,area.getBottom()-75,area.getWidth()-36,22},12,gold,true);
        }
    }
    paintTruncated(g,"CLAVIER MIDI  /  PERFORMANCE",{p.footer.getX()+16,p.footer.getY()+10,juce::jmin(300,p.footer.getWidth()/2),18},11,quiet,true);
    paintTruncated(g,"LIBERTY MUSIC SOFTWARE",{p.footer.getRight()-220,p.footer.getY()+10,202,18},10,gold,true,juce::Justification::centredRight);
}
void LibertyRomplerAudioProcessorEditor::resized() {
    const auto p=panels();
    // Keep all nine buttons inside the title row at the 1120 px minimum.
    const int tabGap=4;
    const int tabWidth=juce::jlimit(80,126,(getWidth()-294-24-8*tabGap)/9);
    int tabX=294;
    for (auto* tab : {&presetsTab,&layersTab,&routingTab,&effectsTab,&matrixTab,&modulatorsTab,&arpTab,&samplesTab,&settingsTab}) {
        tab->setBounds(tabX,79,tabWidth,31);
        tabX+=tabWidth+tabGap;
    }
    brandLabel.setBounds(20,10,260,31);
    brandSubLabel.setBounds(22,43,260,16);
    headingLabel.setBounds(293,8,juce::jmax(80,getWidth()-293-160),34);
    subtitleLabel.setBounds(295,43,juce::jmax(80,getWidth()-295-160),18);
    previousButton.setBounds(getWidth()-123,17,43,39);
    nextButton.setBounds(getWidth()-71,17,43,39);

    const int sx=p.sidebar.getX(),sy=p.sidebar.getY(),sw=p.sidebar.getWidth(),sh=p.sidebar.getHeight();
    bankList.setBounds(sx+12,sy+77,sw-24,juce::jmax(80,sh-224));
    favouriteFilter.setBounds(sx+13,p.sidebar.getBottom()-131,sw-26,34);
    rescanButton.setBounds(sx+13,p.sidebar.getBottom()-85,(sw-34)/2,32);
    folderButton.setBounds(sx+21+(sw-34)/2,p.sidebar.getBottom()-85,(sw-34)/2,32);

    const int bx=p.browser.getX(),by=p.browser.getY(),bw=p.browser.getWidth(),bh=p.browser.getHeight();
    resultCountLabel.setBounds(bx+bw-143,by+13,125,24);
    resultCountLabel.setJustificationType(juce::Justification::centredRight);
    categoryViewport.setBounds(bx+15,by+76,bw-30,39);
    searchField.setBounds(bx+16,by+126,bw-32,39);
    presetList.setBounds(bx+12,by+208,bw-24,juce::jmax(54,bh-280));
    loadedPresetLabel.setBounds(bx+17,p.browser.getBottom()-59,bw-207,24);
    statusLabel.setBounds(bx+17,p.browser.getBottom()-34,bw-34,21);
    favouriteToggle.setBounds(p.browser.getRight()-176,p.browser.getBottom()-67,160,31);

    // Both parameter modules have the exact same geometry. Widgets cannot share bounds.
    const auto f=filterModule(),a=ampModule();
    auto placeKnob=[&](juce::Slider& slider,juce::Label& label,juce::Rectangle<int> box,
                       int index,int count) {
        const int usable=box.getWidth()-28, col=usable/count;
        const int x=box.getX()+14+index*col;
        label.setBounds(x,box.getY()+89,col,16);
        slider.setBounds(x+3,box.getY()+105,col-6,58);
    };
    placeKnob(cutoffSlider,cutoffLabel,f,0,2);
    placeKnob(resonanceSlider,resonanceLabel,f,1,2);
    placeKnob(gainSlider,gainLabel,a,0,3);
    placeKnob(attackSlider,attackLabel,a,1,3);
    placeKnob(releaseSlider,releaseLabel,a,2,3);

    auto foot=p.footer;
    // Full width keyboard: 7 C-B octaves contain exactly 49 white keys.
    keyboard.setBounds(0,foot.getY()+32,getWidth(),78);
    keyboard.setKeyWidth((float)getWidth()/49.0f);
    keyboard.setLowestVisibleKey(24);
    const auto layers=layerCards();
    for (int i=0; i<4; ++i) {
        const auto card=layers[(size_t)i];
        layerEnabled[(size_t)i].setBounds(card.getRight()-90,card.getY()+7,77,26);
        layerSources[(size_t)i].setBounds(card.getX()+12,card.getY()+36,card.getWidth()-24,31);
        const int col=(card.getWidth()-24)/2;
        for (int j=0; j<8; ++j) {
            const int x=card.getX()+12+(j%2)*col;
            const int y=card.getY()+74+(j/2)*24;
            layerLabels[(size_t)i][(size_t)j].setBounds(x,y,62,20);
            layerSliders[(size_t)i][(size_t)j].setBounds(x+65,y,col-71,22);
        }
    }
    const auto cards=effectCards();
    for (int i=0;i<18;++i) {
        const auto& card=cards[(size_t)i/3];
        const int index=i%3,inner=card.getWidth()-30,col=inner/3;
        effectLabels[(size_t)i].setBounds(card.getX()+15+index*col,card.getY()+75,col,17);
        effectSliders[(size_t)i].setBounds(card.getX()+17+index*col,card.getY()+95,col-4,72);
    }
    const auto work=workArea();
    routingGraph.setBounds(work.reduced(16,8).withTrimmedTop(40).withTrimmedBottom(8));
    sampleView.setBounds(work.reduced(16,8).withTrimmedTop(40).withTrimmedBottom(8));
    // ARP: all seven controls occupy non-overlapping columns. The step grid
    // uses two clipped rows of 16 cells and never crosses the footer.
    const auto arpBounds = work.reduced(17,8).withTrimmedTop(41).withTrimmedBottom(9);
    const int rowX=arpBounds.getX()+12, rowW=arpBounds.getWidth()-24, colW=rowW/7;
    arpEnabled.setBounds(rowX+4,arpBounds.getY()+45,colW-8,37);
    arpLabels[0].setBounds(rowX+colW+4,arpBounds.getY()+28,colW-8,15);
    arpMode.setBounds(rowX+colW+4,arpBounds.getY()+47,colW-8,33);
    arpLabels[1].setBounds(rowX+2*colW+4,arpBounds.getY()+28,colW-8,15);
    arpDivision.setBounds(rowX+2*colW+4,arpBounds.getY()+47,colW-8,33);
    for(int i=0;i<4;++i) {
        const int x=rowX+(i+3)*colW+4;
        arpLabels[(size_t)i+2].setBounds(x,arpBounds.getY()+25,colW-8,17);
        arpKnobs[(size_t)i].setBounds(x,arpBounds.getY()+45,colW-8,56);
    }
    arpGrid.setBounds(arpBounds.getX()+10,arpBounds.getY()+133,
                      arpBounds.getWidth()-20,juce::jmax(100,arpBounds.getHeight()-176));
    const int left=work.getX()+24,width=work.getWidth()-48;
    const int sourceW=width*30/100,targetW=width*30/100,amountW=width-sourceW-targetW-24;
    for (int i=0;i<4;++i) {
        const int y=work.getY()+94+i*65;
        sourceChoices[(size_t)i].setBounds(left+8,y,sourceW-16,38);
        targetChoices[(size_t)i].setBounds(left+sourceW+12,y,targetW-16,38);
        amountSliders[(size_t)i].setBounds(left+sourceW+targetW+24,y,amountW-6,38);
    }
    const auto modCards=modulatorCards();
    for (int i=0; i<4; ++i) {
        const auto& card=modCards[0];
        const int col=(card.getWidth()-24)/4;
        ampEnvLabels[(size_t)i].setBounds(card.getX()+12+i*col,card.getY()+100,col,16);
        ampEnvSliders[(size_t)i].setBounds(card.getX()+14+i*col,card.getY()+117,col-4,57);
    }
    for (int i=0; i<5; ++i) {
        const auto& card=modCards[1];
        const int col=(card.getWidth()-24)/5;
        auto& label = i==4 ? filterEnvDepthLabel : filterEnvLabels[(size_t)i];
        auto& slider = i==4 ? filterEnvDepthSlider : filterEnvSliders[(size_t)i];
        label.setBounds(card.getX()+12+i*col,card.getY()+100,col,16);
        slider.setBounds(card.getX()+14+i*col,card.getY()+117,col-4,57);
    }
    for (int i=0; i<2; ++i) {
        const auto& card=modCards[(size_t)i+2];
        const int half=(card.getWidth()-40)/2;
        lfoLabels[(size_t)i].setBounds(card.getX()+17,card.getY()+105,half,18);
        lfoSliders[(size_t)i].setBounds(card.getX()+17,card.getY()+128,half,38);
        lfoShapeChoices[(size_t)i].setBounds(card.getX()+22+half,card.getY()+128,half,38);
    }
    settingsOpen.setBounds(left+18,work.getY()+185,juce::jmin(240,width/2-30),39);
    settingsScan.setBounds(left+juce::jmin(240,width/2-30)+30,work.getY()+185,
                           juce::jmin(260,width/2-30),39);
    // Category content owns a horizontal strip; the viewport clips it safely.
    int x=0;
    for(auto& button:categoryButtons) {
        const int width=juce::jlimit(78,150,32+button->getButtonText().length()*8);
        button->setBounds(x,1,width,33);
        x+=width+6;
    }
    categoryContent.setSize(juce::jmax(categoryViewport.getWidth(),x),36);
}

int LibertyRomplerAudioProcessorEditor::BankModel::getNumRows() {
    return 1+(int)owner.processor.getLibrary().getBanks().size();
}
void LibertyRomplerAudioProcessorEditor::BankModel::paintListBoxItem(
    int row,juce::Graphics& g,int w,int h,bool s) { owner.paintBankRow(row,g,w,h,s); }
void LibertyRomplerAudioProcessorEditor::BankModel::selectedRowsChanged(int row) { owner.selectBank(row); }
void LibertyRomplerAudioProcessorEditor::paintBankRow(int row,juce::Graphics& g,int w,int h,bool highlighted) {
    const auto& banks=processor.getLibrary().getBanks();
    const bool active = row == bankIndex+1;
    auto area=juce::Rectangle<float>(3.0f,3.0f,(float)w-6,(float)h-6);
    g.setColour(active||highlighted?selected:chrome);g.fillRoundedRectangle(area,7.0f);
    if(active){g.setColour(cyan);g.fillRoundedRectangle(4.0f,8.0f,3.0f,(float)h-16,1.4f);}
    const auto accent=row==0?cyan:row==1?gold:purple;
    g.setColour(accent.withAlpha(0.17f));g.fillRoundedRectangle(14.0f,12.0f,35.0f,35.0f,7.0f);
    g.setColour(accent);g.drawRoundedRectangle(14.5f,12.5f,34.0f,34.0f,7.0f,1.0f);
    paintTruncated(g,row==0?"ALL":row==1?"F":"EX",{17,20,29,17},10,accent,true,juce::Justification::centred);
    if(row==0) {
        paintTruncated(g,utf(u8"Tous les pr\u00e9sets"),{58,10,juce::jmax(0,w-68),24},13,white,true);
        int count=0;for(auto& b:banks)count+=(int)b.presets.size();
        paintTruncated(g,juce::String(count)+utf(u8" sons disponibles"),{58,34,juce::jmax(0,w-68),16},10,quiet);
    }else if(row-1<(int)banks.size()) {
        auto& b=banks[(size_t)(row-1)];
        paintTruncated(g,b.name,{58,10,juce::jmax(0,w-68),24},13,white,true);
        paintTruncated(g,juce::String((int)b.presets.size())+" presets  /  "+b.version,
                       {58,34,juce::jmax(0,w-68),16},10,quiet);
    }
}
void LibertyRomplerAudioProcessorEditor::refreshLayerSources() {
    sourceOptions.clear();
    const auto& banks=processor.getLibrary().getBanks();
    for (const auto& bank : banks) for (const auto& preset : bank.presets) {
        // Layer-presets cannot recursively reference other layered patches.
        if (!preset.zones.empty() && preset.layers.empty() && sourceOptions.size() < 500)
            sourceOptions.emplace_back(bank.id, preset.id);
    }
    for (auto& combo : layerSources) {
        combo.clear(juce::dontSendNotification);
        combo.addItem("- AUCUNE SOURCE -",1);
        for (size_t j=0; j<sourceOptions.size(); ++j) {
            const auto& ref=sourceOptions[j];
            if (const auto* p=processor.getLibrary().find(ref.first,ref.second))
                combo.addItem(p->bankName+" / "+p->name,(int)j+2);
        }
    }
    refreshLayerControls();
}
void LibertyRomplerAudioProcessorEditor::refreshLayerControls() {
    const auto& controls=processor.getControls();
    for (int i=0;i<4;++i) {
        const auto& ctrl=controls.layers[(size_t)i];
        const auto source=processor.getLayerSource(i);
        const std::array<double, 8> values {{
            ctrl.gain.load()*100.0, ctrl.pan.load()*100.0,
            (double)ctrl.transpose.load(), (double)ctrl.cutoffHz.load(),
            (double)ctrl.lowNote.load(), (double)ctrl.highNote.load(),
            (double)ctrl.lowVelocity.load(), (double)ctrl.highVelocity.load()
        }};
        for (int j=0; j<8; ++j)
            layerSliders[(size_t)i][(size_t)j].setValue(values[(size_t)j],juce::dontSendNotification);
        layerEnabled[(size_t)i].setToggleState(ctrl.enabled.load(),juce::dontSendNotification);
        int selected=1;
        for (size_t j=0;j<sourceOptions.size();++j)
            if (sourceOptions[j].first==source.bankId && sourceOptions[j].second==source.presetId) {
                selected=(int)j+2;break;
            }
        layerSources[(size_t)i].setSelectedId(selected,juce::dontSendNotification);
    }
}
void LibertyRomplerAudioProcessorEditor::selectBank(int row) {
    if(row<0)return;
    const int next=row-1;
    if(next==bankIndex)return;
    bankIndex=next;
    category="Toutes";
    rebuildCategories();rebuildPresets();
    bankList.repaint();
}
void LibertyRomplerAudioProcessorEditor::rebuildBanks() {
    bankList.updateContent();
    if(bankIndex >= (int)processor.getLibrary().getBanks().size())bankIndex=-1;
    bankList.selectRow(bankIndex+1);
    rebuildCategories();rebuildPresets();
    refreshLayerSources();
}
void LibertyRomplerAudioProcessorEditor::rebuildCategories() {
    categoryButtons.clear();
    juce::StringArray names; names.add("Toutes");
    const auto& banks=processor.getLibrary().getBanks();
    for(size_t b=0;b<banks.size();++b) {
        if(bankIndex>=0 && (int)b!=bankIndex) continue;
        for(const auto& p:banks[b].presets)names.addIfNotAlreadyThere(p.category);
    }
    for(const auto& cat:names) {
        auto button=std::make_unique<juce::TextButton>(cat);
        button->setClickingTogglesState(false);
        button->setToggleState(cat==category,juce::dontSendNotification);
        button->onClick=[this,cat] {
            category=cat;
            for(auto& b:categoryButtons)b->setToggleState(b->getButtonText()==category,juce::dontSendNotification);
            rebuildPresets();
        };
        categoryContent.addAndMakeVisible(*button);
        categoryButtons.push_back(std::move(button));
    }
    resized();
}
int LibertyRomplerAudioProcessorEditor::getNumRows() {return (int)visiblePresets.size();}
void LibertyRomplerAudioProcessorEditor::rebuildPresets() {
    visiblePresets.clear();
    const auto search=searchField.getText().trim();
    const auto& banks=processor.getLibrary().getBanks();
    for(size_t b=0;b<banks.size();++b) {
        if(bankIndex>=0 && (int)b!=bankIndex)continue;
        for(size_t i=0;i<banks[b].presets.size();++i) {
            const auto& p=banks[b].presets[i];
            if(category!="Toutes" && p.category!=category)continue;
            if(search.isNotEmpty() && !p.name.containsIgnoreCase(search)
                && !p.category.containsIgnoreCase(search) && !p.bankName.containsIgnoreCase(search))continue;
            if(favouritesOnly && !isFavourite(p.bankId,p.id))continue;
            visiblePresets.push_back({(int)b,(int)i});
        }
    }
    resultCountLabel.setText(juce::String((int)visiblePresets.size())+utf(u8" pr\u00e9sets"),juce::dontSendNotification);
    presetList.updateContent();presetList.repaint();
    updatePresetInfo();
}
void LibertyRomplerAudioProcessorEditor::paintListBoxItem(
    int row,juce::Graphics& g,int width,int height,bool highlighted) {
    if(row<0||row>=(int)visiblePresets.size())return;
    const auto ref=visiblePresets[(size_t)row];
    const auto& p=processor.getLibrary().getBanks()[(size_t)ref.bank].presets[(size_t)ref.preset];
    const bool playing = p.bankId==processor.getSelectedBankId()&&p.id==processor.getSelectedPresetId();
    if(highlighted||playing){
        g.setColour(selected);g.fillRoundedRectangle(juce::Rectangle<float>(3.0f,2.0f,(float)width-6,(float)height-4),5.0f);
        if(playing){g.setColour(cyan);g.fillRoundedRectangle(3.0f,5.0f,3.0f,(float)height-10,1.0f);}
    }
    const int fav=36,content=width-19-fav,name=content*44/100,cat=content*21/100;
    int x=13;
    paintTruncated(g,p.name,{x,9,name-6,height-18},13,playing?white:secondary,true);x+=name;
    paintTruncated(g,p.category,{x,9,cat-6,height-18},11,quiet);x+=cat;
    paintTruncated(g,p.bankName,{x,9,content-name-cat-5,height-18},11,quiet);
    drawStar(g,{(float)width-22.0f,(float)height*0.5f},9.0f,
             isFavourite(p.bankId,p.id)?gold:quiet,isFavourite(p.bankId,p.id));
    g.setColour(stroke.withAlpha(0.45f));g.drawHorizontalLine(height-1,14.0f,(float)width-15);
}
void LibertyRomplerAudioProcessorEditor::selectedRowsChanged(int row){choosePreset(row);}
void LibertyRomplerAudioProcessorEditor::listBoxItemClicked(int row,const juce::MouseEvent& e) {
    if(e.x>presetList.getWidth()-49 && row>=0 && row<(int)visiblePresets.size()) {
        auto p=visiblePresets[(size_t)row];toggleFavourite(p.bank,p.preset);
    }
}
void LibertyRomplerAudioProcessorEditor::choosePreset(int row) {
    if(row<0||row>=(int)visiblePresets.size())return;
    const auto ref=visiblePresets[(size_t)row];
    const auto& p=processor.getLibrary().getBanks()[(size_t)ref.bank].presets[(size_t)ref.preset];
    const auto result=processor.selectPreset(p.bankId,p.id);
    if(result.failed())statusLabel.setText(result.getErrorMessage(),juce::dontSendNotification);
    else { updatePresetInfo(); refreshLayerControls(); }
    presetList.repaint();repaint(infoModule());
}
void LibertyRomplerAudioProcessorEditor::chooseRelativePreset(int offset) {
    if(visiblePresets.empty())return;
    int row=-1;
    const auto& banks=processor.getLibrary().getBanks();
    for(int i=0;i<(int)visiblePresets.size();++i) {
        const auto& ref=visiblePresets[(size_t)i];
        const auto& p=banks[(size_t)ref.bank].presets[(size_t)ref.preset];
        if(p.bankId==processor.getSelectedBankId()&&p.id==processor.getSelectedPresetId()){row=i;break;}
    }
    const int next=row<0?0:(row+(int)visiblePresets.size()+offset)%(int)visiblePresets.size();
    presetList.selectRow(next);
}
void LibertyRomplerAudioProcessorEditor::updatePresetInfo() {
    const auto name=processor.getSelectedPresetName();
    headingLabel.setText(name.isEmpty()?utf(u8"Aucun pr\u00e9set charg\u00e9"):name,juce::dontSendNotification);
    loadedPresetLabel.setText(name.isEmpty()?utf(u8"Aucun son s\u00e9lectionn\u00e9"):name,juce::dontSendNotification);
    const auto* p=processor.getLibrary().find(processor.getSelectedBankId(),processor.getSelectedPresetId());
    statusLabel.setText(p? p->bankName+"  /  "+p->category+"  /  "+juce::String((int)p->zones.size())+" zones"
         : utf(u8"Installez une banque pour commencer."),juce::dontSendNotification);
    const auto isFav=isFavourite(processor.getSelectedBankId(),processor.getSelectedPresetId());
    favouriteToggle.setButtonText(isFav ? utf(u8"Retirer des favoris") : utf(u8"Ajouter aux favoris"));
    favouriteToggle.setEnabled(p!=nullptr);
    favouriteToggle.setToggleState(isFav,juce::dontSendNotification);
    if (activePage==Page::Samples) sampleView.refresh();
    repaint(infoModule());
}
void LibertyRomplerAudioProcessorEditor::setFavouriteFilter(bool next) {
    favouritesOnly=next;
    favouriteFilter.setToggleState(next,juce::dontSendNotification);
    rebuildPresets();
}
bool LibertyRomplerAudioProcessorEditor::isFavourite(const juce::String& b,const juce::String& p) const {
    return favourites.contains(b+"|"+p);
}
void LibertyRomplerAudioProcessorEditor::toggleFavourite(int b,int i) {
    const auto& banks=processor.getLibrary().getBanks();
    if(b<0||b>=(int)banks.size()||i<0||i>=(int)banks[(size_t)b].presets.size())return;
    const auto& p=banks[(size_t)b].presets[(size_t)i];
    const auto key=p.bankId+"|"+p.id;
    if(favourites.contains(key))favourites.removeString(key);else favourites.addIfNotAlreadyThere(key);
    saveFavourites();
    if(favouritesOnly)rebuildPresets();else presetList.repaint();
    updatePresetInfo();
}
juce::File LibertyRomplerAudioProcessorEditor::favouritesFile() const {
    return processor.getLibrary().getDirectory().getParentDirectory().getChildFile("favourites.txt");
}
void LibertyRomplerAudioProcessorEditor::loadFavourites() {
    auto f=favouritesFile();
    if(f.existsAsFile()) {
        favourites.addLines(f.loadFileAsString());
        favourites.removeEmptyStrings();
        favourites.removeDuplicates(false);
    }
}
void LibertyRomplerAudioProcessorEditor::saveFavourites() const {
    const auto f=favouritesFile();
    if(f.getParentDirectory().createDirectory().wasOk())
        f.replaceWithText(favourites.joinIntoString("\n"));
}
