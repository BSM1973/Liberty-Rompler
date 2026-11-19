#!/bin/bash
# Liberty ROMpler v0.56: signed-ad-hoc AU, VST3, Standalone + Factory 100.
# Runs with the user's permissions; no admin rights, does not delete old banks.
set -euo pipefail
cd "$(dirname "$0")"
PACKAGE_DIR="$(pwd)"
BANK="Factory-Starter-v055-100"
SOURCE_BANK="$PACKAGE_DIR/Banks/$BANK"
BASE="$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler"
CONFIG="$BASE/library-location-v056.json"
ROOT="$BASE/Banks"
if [[ -f "$CONFIG" ]] && command -v python3 >/dev/null 2>&1; then
  CUSTOM="$(python3 - "$CONFIG" <<'PY'
import json,os,sys
try:
    obj=json.load(open(sys.argv[1],encoding='utf-8'))
    path=obj.get('bankDirectory','')
    if isinstance(path,str) and os.path.isabs(path) and os.path.isdir(path):
        print(path)
except (OSError,ValueError,TypeError):
    pass
PY
)"
  [[ -z "$CUSTOM" ]] || ROOT="$CUSTOM"
fi
if [[ ! -f "$SOURCE_BANK/manifest.json" ]]; then
  echo "ERREUR : banques et samples absents du paquet."
  read -r -p "Entrée pour fermer..." _
  exit 1
fi
if command -v python3 >/dev/null 2>&1; then
  python3 - "$SOURCE_BANK" <<'PY'
import json,os,sys
root=sys.argv[1]
m=json.load(open(os.path.join(root,'manifest.json'),encoding='utf-8'))
p=m['presets']
assert m['id']=='liberty-factory-starter-100-v055'
assert len(p)==100 and len({x['id'] for x in p})==100
assert len({x['category'] for x in p})==10
assert all(len(x['zones'])==4 for x in p)
assert all(os.path.isfile(os.path.join(root,z['file'])) for x in p for z in x['zones'])
print('FACTORY 100 : manifeste, 100 presets et 400 samples validés.')
PY
fi
for bundle in "Liberty Rompler.app" "Liberty Rompler.vst3" "Liberty Rompler.component"; do
  if [[ ! -d "$PACKAGE_DIR/$bundle" ]]; then
    echo "ERREUR : composant $bundle absent."
    exit 1
  fi
done
mkdir -p "$ROOT" "$HOME/Applications" "$HOME/Library/Audio/Plug-Ins/VST3" "$HOME/Library/Audio/Plug-Ins/Components"
STAMP="$(date +%Y%m%d-%H%M%S)-$$"
install_bundle() {
  local source="$1" directory="$2" name="$3"
  local destination="$directory/$name"
  local stage="$directory/.liberty-rompler-v056-$STAMP-$RANDOM"
  # Backups are OUTSIDE active AU/VST3 and BankLibrary scan directories.
  local backup_root="$BASE/Sauvegardes-v056"
  mkdir -p "$backup_root"
  local backup="$backup_root/$name.backup-v056-$STAMP"
  mkdir -p "$stage"
  if ! /usr/bin/ditto "$source" "$stage/$name"; then
    rm -rf "$stage"
    echo "ERREUR : copie de $name."
    exit 1
  fi
  if [[ -e "$destination" ]]; then mv "$destination" "$backup"; fi
  if ! mv "$stage/$name" "$destination"; then
    if [[ -e "$backup" ]]; then mv "$backup" "$destination"; fi
    rm -rf "$stage"
    echo "ERREUR : installation de $name."
    exit 1
  fi
  rmdir "$stage"
  echo "Installé : $destination"
}
echo
echo "LIBERTY ROMPLER v0.56 — INSTALLATION COMPLÈTE"
echo "Fermez vos applications audio avant de continuer."
echo "Dossier cible des 100 presets : $ROOT"
echo
install_bundle "$PACKAGE_DIR/Liberty Rompler.app" "$HOME/Applications" "Liberty Rompler.app"
install_bundle "$PACKAGE_DIR/Liberty Rompler.vst3" "$HOME/Library/Audio/Plug-Ins/VST3" "Liberty Rompler.vst3"
install_bundle "$PACKAGE_DIR/Liberty Rompler.component" "$HOME/Library/Audio/Plug-Ins/Components" "Liberty Rompler.component"
install_bundle "$SOURCE_BANK" "$ROOT" "$BANK"
# The installed manifest is at the same path the JUCE BankLibrary indexes.
test -f "$ROOT/$BANK/manifest.json"
printf '\nSUCCÈS : 100 presets Factory Starter installés et prêts à être scannés.\n'
printf 'Dans ROMpler : PRESETS > banque "Liberty Factory Starter — 100 Presets".\n'
printf 'RÉGLAGES > CHANGER LE DOSSIER permet de copier toute la bibliothèque.\n'
printf 'Anciennes versions et banques préservées (sauvegardes .backup-v056).\n'
printf 'Relancez Liberty ROMpler ou votre DAW.\n\n'
read -r -p "Entrée pour fermer..." _
