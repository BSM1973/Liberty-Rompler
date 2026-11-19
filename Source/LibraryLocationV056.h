#pragma once
#include <JuceHeader.h>

// v0.56: per-user library root. Never touches audio callbacks.
// A complete bank directory contains manifest.json and relative Samples paths.
namespace liberty::factory_v056 {
inline juce::File settingsDirectory() {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Liberty Music Software").getChildFile("Liberty Rompler");
}
inline juce::File settingsFile() {
    return settingsDirectory().getChildFile("library-location-v056.json");
}
inline juce::File legacyBankDirectory() {
    return settingsDirectory().getChildFile("Banks");
}
inline juce::File configuredBankDirectory() {
    const auto file=settingsFile();
    if (file.existsAsFile()) {
        const auto json=juce::JSON::parse(file);
        if (auto* object=json.getDynamicObject()) {
            auto path=object->getProperty("bankDirectory").toString().trim();
            if (juce::File::isAbsolutePath(path)) {
                juce::File target(path);
                if (target.isDirectory()) return target;
            }
        }
    }
    return legacyBankDirectory();
}
inline juce::Result saveBankDirectory(const juce::File& folder) {
    const auto dir=settingsDirectory();
    if (dir.createDirectory().failed())
        return juce::Result::fail("Impossible de créer les préférences ROMpler.");
    auto* obj=new juce::DynamicObject();
    obj->setProperty("version", "0.56");
    obj->setProperty("bankDirectory", folder.getFullPathName());
    const juce::String json=juce::JSON::toString(juce::var(obj), true);
    juce::TemporaryFile tmp(settingsFile());
    if (!tmp.getFile().replaceWithText(json) || !tmp.overwriteTargetFileWithTemporary())
        return juce::Result::fail("Impossible de mémoriser le dossier de bibliothèque.");
    return juce::Result::ok();
}
inline juce::Result copyWholeLibrary(const juce::File& source,
                                     const juce::File& selectedParent,
                                     juce::File& copiedRoot) {
    if (!selectedParent.isDirectory())
        return juce::Result::fail("Choisissez un dossier existant et accessible.");
    if (!source.isDirectory())
        return juce::Result::fail("Bibliothèque source absente. Réinstallez Factory Starter 100.");
    if (selectedParent==source || selectedParent.isAChildOf(source) ||
        source.isAChildOf(selectedParent))
        return juce::Result::fail("Choisissez un autre emplacement, hors du dossier actuel.");
    const auto target=selectedParent.getChildFile("Liberty ROMpler Banks");
    if (target.exists())
        return juce::Result::fail("Un dossier Liberty ROMpler Banks existe déjà ici : aucun fichier n'a été écrasé.");
    const auto staging=selectedParent.getNonexistentChildFile(
        ".Liberty-ROMpler-v056-copie-", "", false);
    if (!source.copyDirectoryTo(staging)) {
        staging.deleteRecursively();
        return juce::Result::fail("Copie interrompue. Le dossier original est intact.");
    }
    // Byte-count and relative-path validation, including all 400 factory WAVs.
    juce::Array<juce::File> files;
    source.findChildFiles(files, juce::File::findFiles, true);
    for (const auto& src : files) {
        if (src.isSymbolicLink()) {
            staging.deleteRecursively();
            return juce::Result::fail("Lien symbolique non autorisé dans la bibliothèque source.");
        }
        const auto rel=src.getRelativePathFrom(source);
        const auto dst=staging.getChildFile(rel);
        if (!dst.existsAsFile() || dst.getSize()!=src.getSize()) {
            staging.deleteRecursively();
            return juce::Result::fail("Copie incomplète : un sample ou manifeste est manquant.");
        }
    }
    if (!staging.moveFileTo(target)) {
        staging.deleteRecursively();
        return juce::Result::fail("Impossible de terminer la copie. Original inchangé.");
    }
    copiedRoot=target;
    return juce::Result::ok();
}
} // namespace liberty::factory_v056
