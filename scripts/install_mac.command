#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

app="Liberty Rompler.app"
vst="Liberty Rompler.vst3"
au="Liberty Rompler.component"
for item in "$app" "$vst" "$au"; do
  if [[ ! -d "$item" ]]; then
    printf 'ERREUR : composant absent : %s\n' "$item" >&2
    exit 1
  fi
done
for bank in Factory-Starter Expansion-Demo; do
  if [[ ! -f "Banks/$bank/manifest.json" ]]; then
    printf 'ERREUR : banque incomplete : %s\n' "$bank" >&2
    exit 1
  fi
done

app_dir="$HOME/Applications"
vst_dir="$HOME/Library/Audio/Plug-Ins/VST3"
au_dir="$HOME/Library/Audio/Plug-Ins/Components"
bank_dir="$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks"
mkdir -p "$app_dir" "$vst_dir" "$au_dir" "$bank_dir"

stamp="$(date +%Y%m%d-%H%M%S)-$$"

install_bundle() {
  local source="$1" destination_dir="$2" name="$3"
  local target="$destination_dir/$name"
  local stage="$destination_dir/.liberty-rompler-stage-$stamp-$RANDOM"
  local backup="$destination_dir/.$name.sauvegarde-$stamp"
  mkdir -p "$stage"
  if ! ditto "$source" "$stage/$name"; then
    rm -rf "$stage"
    printf 'ERREUR : copie impossible de %s\n' "$name" >&2
    return 1
  fi
  if [[ -e "$target" || -L "$target" ]]; then
    if ! mv "$target" "$backup"; then
      rm -rf "$stage"
      printf 'ERREUR : sauvegarde impossible de %s\n' "$name" >&2
      return 1
    fi
    printf 'Ancienne version conservee : %s\n' "$backup"
  fi
  if ! mv "$stage/$name" "$target"; then
    if [[ -e "$backup" || -L "$backup" ]]; then
      mv "$backup" "$target" || true
    fi
    rm -rf "$stage"
    printf 'ERREUR : installation impossible de %s\n' "$name" >&2
    return 1
  fi
  rmdir "$stage"
  printf 'Installe : %s\n' "$target"
}

printf '\nLIBERTY ROMPLER - Installation macOS par utilisateur\n'
printf 'Fermez vos logiciels audio avant cette installation.\n\n'
install_bundle "$app" "$app_dir" "$app"
install_bundle "$vst" "$vst_dir" "$vst"
install_bundle "$au" "$au_dir" "$au"

for bank in Factory-Starter Expansion-Demo; do
  if [[ -e "$bank_dir/$bank" ]]; then
    printf 'Banque existante preservee : %s\n' "$bank"
  else
    ditto "Banks/$bank" "$bank_dir/$bank"
    printf 'Banque installee : %s\n' "$bank"
  fi
done
printf '\nInstallation terminee. Relancez votre DAW et actualisez les plugins.\n'
printf 'Les banques personnelles et les anciennes versions sauvegardees n ont pas ete supprimees.\n'
printf 'Version de test signee ad hoc : non notarisee par Apple.\n'
