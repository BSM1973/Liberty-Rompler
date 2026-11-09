#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ ! -d "Banks/Factory-Starter" || ! -d "Banks/Expansion-Demo" ]]; then
  echo "Erreur : banques de demonstration absentes du dossier."
  exit 1
fi
mkdir -p "$HOME/Applications" "$HOME/Library/Audio/Plug-Ins/VST3" "$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks"
ditto "Liberty Rompler.app" "$HOME/Applications/Liberty Rompler.app"
ditto "Liberty Rompler.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/Liberty Rompler.vst3"
ditto "Liberty Rompler.component" "$HOME/Library/Audio/Plug-Ins/Components/Liberty Rompler.component"
ditto "Banks/Factory-Starter" "$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks/Factory-Starter"
ditto "Banks/Expansion-Demo" "$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks/Expansion-Demo"
echo ""
echo "Liberty Rompler installe pour cet utilisateur."
echo "Application : ~/Applications/Liberty Rompler.app"
echo "Dans le DAW : recharger les plugins AU / VST3."
echo "ATTENTION : prototype signe ad hoc, non notarise par Apple."
