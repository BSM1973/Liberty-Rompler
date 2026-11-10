#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RoutingGraph.h"
#include "ArpStepGrid.h"
#include "SampleZoneView.h"
#include <memory>
#include <array>
#include <utility>
#include <vector>

// The entire UI is drawn with real JUCE components, not a static backdrop.
// All visible labels use explicit UTF-8; geometry is shared by paint/resized.
class LibertyRomplerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                 private juce::ListBoxModel {
public:
    explicit LibertyRomplerAudioProcessorEditor(LibertyRomplerAudioProcessor&);
    ~LibertyRomplerAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct LibertyTheme final : juce::LookAndFeel_V4 {
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float,
                              juce::Slider&) override;
    };
    struct BankModel final : juce::ListBoxModel {
        explicit BankModel(LibertyRomplerAudioProcessorEditor& editor) : owner(editor) {}
        int getNumRows() override;
        void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
        void selectedRowsChanged(int) override;
        LibertyRomplerAudioProcessorEditor& owner;
    };
    struct PresetRef { int bank = 0, preset = 0; };
    struct Panels {
        juce::Rectangle<int> sidebar, browser, modifiers, footer;
    };

    LibertyRomplerAudioProcessor& processor;
    enum class Page { Presets, Layers, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings };
    Page activePage = Page::Presets;
    juce::TextButton presetsTab {"PRESETS"}, layersTab {"COUCHES"}, routingTab {"ROUTAGE"}, arpTab {"ARP"}, effectsTab {"EFFETS"};
    juce::TextButton matrixTab {"MODULATION"}, modulatorsTab {"MODULATEURS"};
    juce::TextButton samplesTab {"SAMPLES"};
    juce::TextButton settingsTab {"REGLAGES"};
    LibertyRoutingGraph routingGraph;
    LibertyArpStepGrid arpGrid;
    LibertySampleZoneView sampleView;
    juce::TextButton arpEnabled {"ARP OFF"};
    juce::ComboBox arpMode, arpDivision;
    std::array<juce::Slider,4> arpKnobs;
    std::array<juce::Label,6> arpLabels;
    std::array<juce::ComboBox, 4> layerSources;
    std::array<juce::TextButton, 4> layerEnabled;
    std::array<std::array<juce::Slider, 8>, 4> layerSliders;
    std::array<std::array<juce::Label, 8>, 4> layerLabels;
    std::vector<std::pair<juce::String, juce::String>> sourceOptions;
    std::array<juce::Slider, 18> effectSliders;
    std::array<juce::Label, 18> effectLabels;
    std::array<juce::Slider, 4> ampEnvSliders, filterEnvSliders;
    std::array<juce::Label, 4> ampEnvLabels, filterEnvLabels;
    juce::Slider filterEnvDepthSlider;
    juce::Label filterEnvDepthLabel;
    std::array<juce::ComboBox, 2> lfoShapeChoices;
    std::array<juce::ComboBox, 4> sourceChoices, targetChoices;
    std::array<juce::Slider, 4> amountSliders;
    std::array<juce::Slider, 2> lfoSliders;
    std::array<juce::Label, 2> lfoLabels;
    juce::TextButton settingsOpen {"OUVRIR LES BANQUES"};
    juce::TextButton settingsScan {"ACTUALISER LES BANQUES"};
    LibertyTheme theme;
    BankModel bankModel { *this };
    juce::ListBox bankList, presetList;
    juce::TextEditor searchField;
    juce::Viewport categoryViewport;
    juce::Component categoryContent;
    std::vector<std::unique_ptr<juce::TextButton>> categoryButtons;
    juce::TextButton favouriteFilter {"FAVORIS"}, favouriteToggle {"Ajouter aux favoris"};
    juce::TextButton rescanButton {"Actualiser"}, folderButton {"Dossier"};
    juce::TextButton previousButton {"<"}, nextButton {">"};
    juce::Label brandLabel, brandSubLabel, headingLabel, subtitleLabel;
    juce::Label resultCountLabel, loadedPresetLabel, statusLabel;
    juce::Label cutoffLabel, resonanceLabel, gainLabel, attackLabel, releaseLabel;
    juce::Slider cutoffSlider, resonanceSlider, gainSlider, attackSlider, releaseSlider;
    juce::MidiKeyboardComponent keyboard;
    std::vector<PresetRef> visiblePresets;
    juce::StringArray favourites;
    juce::String category = "Toutes";
    int bankIndex = -1; // -1 = all banks, otherwise an index into BankLibrary
    bool favouritesOnly = false;

    Panels panels() const;
    juce::Rectangle<int> workArea() const;
    std::array<juce::Rectangle<int>, 4> layerCards() const;
    std::array<juce::Rectangle<int>, 6> effectCards() const;
    std::array<juce::Rectangle<int>, 4> modulatorCards() const;
    void refreshLayerSources();
    void refreshLayerControls();
    void selectPage(Page);

    juce::Rectangle<int> filterModule() const;
    juce::Rectangle<int> ampModule() const;
    juce::Rectangle<int> infoModule() const;
    void paintPanel(juce::Graphics&, juce::Rectangle<int>, juce::Colour) const;
    void paintFilterGraph(juce::Graphics&, juce::Rectangle<float>) const;
    void paintAmpGraph(juce::Graphics&, juce::Rectangle<float>) const;
    void paintBankRow(int, juce::Graphics&, int, int, bool);
    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemClicked(int, const juce::MouseEvent&) override;
    void selectBank(int);
    void rebuildBanks();
    void rebuildCategories();
    void rebuildPresets();
    void updatePresetInfo();
    void chooseRelativePreset(int);
    void choosePreset(int);
    void setFavouriteFilter(bool);
    void toggleFavourite(int, int);
    bool isFavourite(const juce::String&, const juce::String&) const;
    void loadFavourites();
    void saveFavourites() const;
    juce::File favouritesFile() const;
    static void setupLabel(juce::Label&, const juce::String&, float, juce::Colour, bool bold = false);
    static void setupSlider(juce::Slider&, juce::Label&, const juce::String&, double, double,
                            double, bool logarithmic = false);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibertyRomplerAudioProcessorEditor)
};
