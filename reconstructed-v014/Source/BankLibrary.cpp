#include "BankLibrary.h"
#include <cstdlib>

namespace liberty {
namespace {
int integer(juce::DynamicObject* o, const char* key, int fallback) {
    return o->hasProperty(key) ? (int)o->getProperty(key) : fallback;
}
float decimal(juce::DynamicObject* o, const char* key, float fallback) {
    return o->hasProperty(key) ? (float)(double)o->getProperty(key) : fallback;
}
juce::String string(juce::DynamicObject* o, const char* key) {
    return o->getProperty(key).toString().trim();
}
bool safeId(const juce::String& id) {
    if (id.isEmpty() || id.length() > 80) return false;
    for (juce::juce_wchar c : id)
        if (!(juce::CharacterFunctions::isLetterOrDigit(c) || c == '-' || c == '_')) return false;
    return true;
}
bool inside(const juce::File& root, const juce::File& candidate) {
    return candidate.isAChildOf(root) && candidate.existsAsFile();
}
}

juce::File BankLibrary::defaultDirectory() {
    if (const auto* overridePath = std::getenv("LIBERTY_ROMPLER_BANK_DIR"))
        if (*overridePath) return juce::File(juce::String::fromUTF8(overridePath));
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Liberty Music Software")
        .getChildFile("Liberty Rompler")
        .getChildFile("Banks");
}

BankLibrary::BankLibrary(juce::File directory) : bankRoot(std::move(directory)) { rescan(); }

void BankLibrary::rescan() {
    banks.clear(); warnings.clear();
    if (!bankRoot.exists()) { warnings.add("Dossier de banques absent : " + bankRoot.getFullPathName()); return; }
    juce::Array<juce::File> folders;
    bankRoot.findChildFiles(folders, juce::File::findDirectories, false);
    if (folders.size() > 128) { warnings.add("Limite de 128 banques atteinte."); return; }
    for (const auto& folder : folders) {
        const auto manifestFile = folder.getChildFile("manifest.json");
        if (!manifestFile.existsAsFile()) continue;
        const auto json = juce::JSON::parse(manifestFile);
        auto* object = json.getDynamicObject();
        if (!object || string(object, "format") != "liberty-rompler-bank-v1") {
            warnings.add("Manifest incorrect : " + folder.getFileName()); continue;
        }
        BankDescriptor bank;
        bank.id = string(object, "id");
        bank.name = string(object, "name");
        bank.version = string(object, "version");
        bank.folder = folder;
        if (!safeId(bank.id) || bank.name.isEmpty()) {
            warnings.add("Identifiant de banque invalide : " + folder.getFileName()); continue;
        }
        bool duplicate = false;
        for (const auto& previous : banks) duplicate |= previous.id == bank.id;
        if (duplicate) { warnings.add("Identifiant de banque duplique : " + bank.id); continue; }
        auto* presets = object->getProperty("presets").getArray();
        if (!presets || presets->size() > 256) { warnings.add("Presets invalides : " + bank.name); continue; }
        for (const auto& presetValue : *presets) {
            auto* p = presetValue.getDynamicObject();
            if (!p) continue;
            PresetDescriptor preset;
            preset.id = string(p, "id"); preset.name = string(p, "name");
            preset.category = string(p, "category");
            preset.bankId = bank.id; preset.bankName = bank.name;
            auto* zones = p->getProperty("zones").getArray();
            auto* layerDefinitions = p->getProperty("layers").getArray();
            if (!safeId(preset.id) || preset.name.isEmpty() ||
                (!zones && !layerDefinitions) || (zones && zones->size() > 32) ||
                (layerDefinitions && layerDefinitions->size() > 4)) continue;
            if (layerDefinitions != nullptr) for (const auto& value : *layerDefinitions) {
                auto* o = value.getDynamicObject();
                if (!o) continue;
                LayerPresetDefinition layer;
                layer.bankId = string(o, "source_bank");
                if (layer.bankId.isEmpty()) layer.bankId = bank.id;
                layer.presetId = string(o, "source_preset");
                layer.enabled = o->hasProperty("enabled") ? (bool)o->getProperty("enabled") : true;
                layer.gain = decimal(o,"gain",0.6f);
                layer.pan = decimal(o,"pan",0.0f);
                layer.cutoffHz = decimal(o,"cutoff",20000.0f);
                layer.transpose = integer(o,"transpose",0);
                layer.lowNote = integer(o,"low_note",0);
                layer.highNote = integer(o,"high_note",127);
                layer.lowVelocity = integer(o,"low_velocity",1);
                layer.highVelocity = integer(o,"high_velocity",127);
                if (!safeId(layer.bankId) || !safeId(layer.presetId) ||
                    layer.gain < 0.0f || layer.gain > 1.5f ||
                    layer.pan < -1.0f || layer.pan > 1.0f ||
                    layer.cutoffHz < 80.0f || layer.cutoffHz > 20000.0f ||
                    layer.transpose < -24 || layer.transpose > 24 ||
                    layer.lowNote < 0 || layer.highNote > 127 || layer.lowNote > layer.highNote ||
                    layer.lowVelocity < 1 || layer.highVelocity > 127 ||
                    layer.lowVelocity > layer.highVelocity) continue;
                preset.layers.push_back(std::move(layer));
            }
            if (zones != nullptr) for (const auto& zoneValue : *zones) {
                auto* z = zoneValue.getDynamicObject();
                if (!z) continue;
                const auto path = string(z, "file");
                if (path.isEmpty() || juce::File::isAbsolutePath(path) || path.contains("..") || path.containsChar('\\')) continue;
                ZoneDescriptor zone;
                zone.file = folder.getChildFile(path);
                zone.rootNote = integer(z, "root_note", 60);
                zone.lowNote = integer(z, "low_note", 0);
                zone.highNote = integer(z, "high_note", 127);
                zone.lowVelocity = integer(z, "low_velocity", 1);
                zone.highVelocity = integer(z, "high_velocity", 127);
                zone.loopStart = integer(z, "loop_start", -1);
                zone.loopEnd = integer(z, "loop_end", -1);
                zone.gain = decimal(z, "gain", 1.0f);
                if (!inside(folder, zone.file) || zone.rootNote < 0 || zone.rootNote > 127 ||
                    zone.lowNote < 0 || zone.highNote > 127 || zone.lowNote > zone.highNote ||
                    zone.lowVelocity < 1 || zone.highVelocity > 127 || zone.lowVelocity > zone.highVelocity ||
                    zone.gain < 0 || zone.gain > 4) continue;
                preset.zones.push_back(zone);
            }
            if (!preset.zones.empty() || !preset.layers.empty()) bank.presets.push_back(std::move(preset));
        }
        if (!bank.presets.empty()) banks.push_back(std::move(bank));
        else warnings.add("Aucun preset exploitable : " + folder.getFileName());
    }
}

const PresetDescriptor* BankLibrary::find(const juce::String& bank, const juce::String& preset) const {
    for (const auto& b : banks)
        if (b.id == bank)
            for (const auto& p : b.presets) if (p.id == preset) return &p;
    return nullptr;
}
}
