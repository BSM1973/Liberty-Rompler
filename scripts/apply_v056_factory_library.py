#!/usr/bin/env python3
"""v0.56 JUCE factory 100 and persistent library folder relocation.

Apply after v0.55 spectral overlays, before configuring CMake.
All changes are restricted to bank loading, UI preferences, initial preset
selection and plugin version. Never patch audio-thread rendering.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
def once(path,old,new):
    source=path.read_text(encoding="utf-8")
    count=source.count(old)
    if count!=1:
        raise SystemExit(f"v0.56 anchor mismatch in {path.name}: {count}, {old[:90]!r}")
    path.write_text(source.replace(old,new,1),encoding="utf-8")

b_h=root/"Source/BankLibrary.h"
b_cpp=root/"Source/BankLibrary.cpp"
p_h=root/"Source/PluginProcessor.h"
p_cpp=root/"Source/PluginProcessor.cpp"
e_h=root/"Source/PluginEditor.h"
e_cpp=root/"Source/PluginEditor.cpp"
cmake=root/"CMakeLists.txt"

once(b_h,
    '    void rescan();',
    '    void rescan();\n    void setDirectory(const juce::File& root) { bankRoot=root; rescan(); }')
once(b_cpp,
    '#include "BankLibrary.h"',
    '#include "BankLibrary.h"\n#include "LibraryLocationV056.h"')
once(b_cpp,
    '''    for (const auto& folder : folders) {
        const auto manifestFile = folder.getChildFile("manifest.json");''',
    '''    for (const auto& folder : folders) {
        // Avoid duplicate old bank IDs in hidden installer backup folders.
        if (folder.getFileName().startsWith(".")) continue;
        const auto manifestFile = folder.getChildFile("manifest.json");''')
once(b_cpp,
    '''    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Liberty Music Software")
        .getChildFile("Liberty Rompler")
        .getChildFile("Banks");''',
    '''    return liberty::factory_v056::configuredBankDirectory();''')

once(p_h,
    '    juce::Result importBankFolder(const juce::File& sourceFolder);',
    '    juce::Result importBankFolder(const juce::File& sourceFolder);\n'
    '    juce::Result relocateBankLibrary(const juce::File& parent);')
once(p_cpp,
    '#include "PluginProcessor.h"',
    '#include "PluginProcessor.h"\n#include "LibraryLocationV056.h"')
once(p_cpp,
    '''    if (!library.getBanks().empty() && !library.getBanks()[0].presets.empty()) {
        const auto& bank = library.getBanks()[0];
        selectPreset(bank.id, bank.presets[0].id);
    }''',
    '''    // v0.56: prefer the installed 100-preset Factory Starter on first open.
    // Host state restoration may still choose any saved bank/preset afterwards.
    const auto* factory=library.find("liberty-factory-starter-100-v055", "submarine");
    if (factory) {
        selectPreset(factory->bankId, factory->id);
    } else if (!library.getBanks().empty() && !library.getBanks()[0].presets.empty()) {
        const auto& bank=library.getBanks()[0];
        selectPreset(bank.id, bank.presets[0].id);
    }''')
once(p_cpp,
    'juce::Result LibertyRomplerAudioProcessor::importBankFolder(const juce::File& sourceFolder) {',
    '''juce::Result LibertyRomplerAudioProcessor::relocateBankLibrary(const juce::File& parent) {
    const auto previous=library.getDirectory();
    juce::File copied;
    const auto transfer=liberty::factory_v056::copyWholeLibrary(previous,parent,copied);
    if (transfer.failed()) return transfer;
    // Validate every original bank and every preset before switching folders.
    liberty::BankLibrary candidate(copied);
    const auto& before=library.getBanks();
    const auto& after=candidate.getBanks();
    bool valid=before.size()==after.size();
    for (const auto& b:before) {
        const auto match=std::find_if(after.begin(),after.end(),
            [&](const liberty::BankDescriptor& x) { return x.id==b.id; });
        if (match==after.end() || match->presets.size()!=b.presets.size()) {
            valid=false;
            break;
        }
        for (const auto& p:b.presets) {
            const auto found=candidate.find(b.id,p.id);
            if (!found || found->zones.size()!=p.zones.size() ||
                found->layers.size()!=p.layers.size()) { valid=false; break; }
        }
        if (!valid) break;
    }
    if (!valid) {
        copied.deleteRecursively(); // Original data remains untouched.
        return juce::Result::fail("Vérification échouée : banques ou échantillons incomplets.");
    }
    const auto saved=liberty::factory_v056::saveBankDirectory(copied);
    if (saved.failed()) return saved; // Copy preserved for recovery.
    const auto oldBank=currentBankId,oldPreset=currentPresetId;
    library.setDirectory(copied);
    if (library.find(oldBank,oldPreset)) {
        // Rebind streaming readers to the files in the new library root.
        const auto reloaded=selectPreset(oldBank,oldPreset,true);
        if (reloaded.failed()) return reloaded;
    }
    // Deliberately retain the old files: an audio host may still be streaming
    // from them. User may delete the backup only after closing all DAWs.
    return juce::Result::ok();
}

juce::Result LibertyRomplerAudioProcessor::importBankFolder(const juce::File& sourceFolder) {''')

once(e_h,
    '    juce::TextButton settingsScan {"ACTUALISER LES BANQUES"};',
    '    juce::TextButton settingsScan {"ACTUALISER LES BANQUES"};\n'
    '    juce::TextButton settingsMove {"CHANGER LE DOSSIER PRESETS + SAMPLES"};\n'
    '    juce::Label settingsLocation;\n'
    '    std::unique_ptr<juce::FileChooser> libraryChooser;')
once(e_cpp,
    '''    settingsOpen.onClick = [this] { auto dir=processor.getLibrary().getDirectory(); dir.createDirectory(); dir.revealToUser(); };''',
    '''    settingsOpen.onClick = [this] { auto dir=processor.getLibrary().getDirectory(); dir.createDirectory(); dir.revealToUser(); };
    setupLabel(settingsLocation,"",11,secondary);
    settingsLocation.setText(processor.getLibrary().getDirectory().getFullPathName(),
                             juce::dontSendNotification);
    addChildComponent(settingsLocation);
    settingsMove.onClick = [this] {
        libraryChooser=std::make_unique<juce::FileChooser>(
            juce::String::fromUTF8(u8"Choisir le dossier parent de la bibliothèque ROMpler"),
            juce::File::getSpecialLocation(juce::File::userHomeDirectory));
        juce::Component::SafePointer<LibertyRomplerAudioProcessorEditor> safe(this);
        libraryChooser->launchAsync(
            juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectDirectories,
            [safe](const juce::FileChooser& chooser) {
                if (safe==nullptr) return;
                const auto folder=chooser.getResult();
                if (folder==juce::File()) return;
                safe->settingsLocation.setText(
                    juce::String::fromUTF8(u8"Copie complète de la bibliothèque en cours..."),
                    juce::dontSendNotification);
                const auto result=safe->processor.relocateBankLibrary(folder);
                safe->settingsLocation.setText(
                    result.wasOk()
                        ? safe->processor.getLibrary().getDirectory().getFullPathName()
                        : result.getErrorMessage(),juce::dontSendNotification);
                if (result.wasOk()) {
                    safe->bankIndex=-1;
                    safe->category="Toutes";
                    safe->rebuildBanks();
                    safe->statusLabel.setText(juce::String::fromUTF8(
                        u8"Nouvel emplacement actif. Ancien dossier préservé par sécurité."),
                        juce::dontSendNotification);
                }
            });
    };
    addChildComponent(settingsMove);''')
once(e_cpp,
    '    settingsScan.setVisible(next==Page::Settings);',
    '    settingsScan.setVisible(next==Page::Settings);\n'
    '    settingsMove.setVisible(next==Page::Settings);\n'
    '    settingsLocation.setVisible(next==Page::Settings);')
once(e_cpp,
    '''            const auto card=juce::Rectangle<int>(area.getX(),area.getY(),area.getWidth(),
                                                juce::jmin(205,area.getHeight()/2));''',
    '''            const auto card=juce::Rectangle<int>(area.getX(),area.getY(),area.getWidth(),
                                                juce::jmin(272,area.getHeight()-20));''')
once(e_cpp,
    r'''            paintTruncated(g,utf(u8"Les banques sont charg\u00e9es depuis le dossier de l'utilisateur."),
                {card.getX()+18,card.getY()+52,card.getWidth()-36,24},12,secondary);
            paintTruncated(g,utf(u8"Actualisez la biblioth\u00e8que apr\u00e8s l'installation d'une extension."),
                {card.getX()+18,card.getY()+78,card.getWidth()-36,24},12,secondary);''',
    r'''            paintTruncated(g,utf(u8"Factory Starter : 100 presets et leurs samples install\u00e9s ensemble."),
                {card.getX()+18,card.getY()+52,card.getWidth()-36,24},12,secondary);
            paintTruncated(g,utf(u8"Choisissez un autre disque pour copier toute la biblioth\u00e8que."),
                {card.getX()+18,card.getY()+78,card.getWidth()-36,24},12,secondary);''')
once(e_cpp,
    '''    streamingDiagnostics.setBounds(work.getX()+24, work.getY()+282,
                                   work.getWidth()-48, juce::jmax(165, work.getHeight()-300));''',
    '''    settingsMove.setBounds(left+18,work.getY()+232,juce::jmin(370,width-36),36);
    settingsLocation.setBounds(left+18,work.getY()+272,width-36,27);
    streamingDiagnostics.setBounds(work.getX()+24,work.getY()+346,
                                   work.getWidth()-48,juce::jmax(110,work.getHeight()-366));''')
# The upstream release overlay may change the subtitle version string.
# Replace only the exact setupLabel(subtitleLabel) declaration line.
source=e_cpp.read_text(encoding="utf-8")
lines=[line for line in source.splitlines(keepends=True)
       if "setupLabel(subtitleLabel," in line]
if len(lines)!=1: raise SystemExit("v0.56 subtitle label anchor missing or repeated")
source=source.replace(lines[0],
    '    setupLabel(subtitleLabel, "LIBERTY ROMPLER v0.56  |  AU / VST3 / STANDALONE", 10, quiet);\n',1)
e_cpp.write_text(source,encoding="utf-8")
once(cmake,
    'project(LibertyRompler VERSION 0.1.0 LANGUAGES C CXX)',
    'project(LibertyRompler VERSION 0.56.0 LANGUAGES C CXX)')
# Keep C++ files ASCII-only while preserving the French accents at runtime.
# This project audits every C++ source and requires Unicode escape sequences.
for generated in (p_cpp,e_cpp):
    source=generated.read_text(encoding="utf-8")
    source="".join(c if ord(c)<128 else "\\u"+format(ord(c),"04x") for c in source)
    generated.write_text(source,encoding="utf-8")
print("v0.56: library root preference, full-folder relocation, factory boot preset, UI and version patched.")
