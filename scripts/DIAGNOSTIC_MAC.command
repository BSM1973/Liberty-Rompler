#!/bin/bash
set -euo pipefail

printf '\nLIBERTY ROMPLER - Diagnostic macOS\n'
printf 'Systeme : %s\n' "$(sw_vers -productVersion 2>/dev/null || echo inconnu)"
printf 'Architecture : %s\n' "$(uname -m)"
if [[ "$(uname -m)" != arm64 ]]; then
  printf 'ATTENTION : cette distribution cible macOS Apple Silicon (arm64).\n'
fi

check_bundle() {
  local label="$1" path="$2"
  if [[ ! -d "$path" ]]; then
    printf '[ABSENT] %s : %s\n' "$label" "$path"
    return
  fi
  printf '[PRESENT] %s : %s\n' "$label" "$path"
  if command -v codesign >/dev/null 2>&1; then
    if codesign --verify --deep "$path" >/dev/null 2>&1; then
      printf '[OK] Signature : %s\n' "$label"
    else
      printf '[A VERIFIER] Signature : %s\n' "$label"
    fi
  fi
}

check_bundle 'Standalone' "$HOME/Applications/Liberty Rompler.app"
check_bundle 'VST3' "$HOME/Library/Audio/Plug-Ins/VST3/Liberty Rompler.vst3"
check_bundle 'Audio Unit' "$HOME/Library/Audio/Plug-Ins/Components/Liberty Rompler.component"

bank_dir="$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks"
for bank in Factory-Starter Expansion-Demo; do
  if [[ -f "$bank_dir/$bank/manifest.json" ]]; then
    printf '[OK] Banque : %s\n' "$bank"
  else
    printf '[ABSENTE] Banque : %s\n' "$bank"
  fi
done

for plugin in '/Library/Audio/Plug-Ins/VST3/Liberty Rompler.vst3' '/Library/Audio/Plug-Ins/Components/Liberty Rompler.component'; do
  if [[ -d "$plugin" ]]; then
    printf '[INFO] Autre installation systeme detectee : %s\n' "$plugin"
  fi
done
printf '\nSi le plugin est absent du DAW : fermez le DAW, relancez-le puis effectuez un nouveau scan des plugins.\n'
printf 'Aucune configuration ni aucun projet n a ete modifie.\n'
