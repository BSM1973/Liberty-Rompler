#!/bin/bash
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
source_dir="$here/Factory-Starter-v055-100"
if [[ -d "$here/Banks/Factory-Starter-v055-100" ]]; then
  source_dir="$here/Banks/Factory-Starter-v055-100"
fi
bank_root="$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler/Banks"
target="$bank_root/Factory-Starter-v055-100"
if [[ ! -f "$source_dir/manifest.json" ]]; then
  echo "ERREUR : la banque Factory Starter 100 est introuvable."
  read -r -p "Appuyez sur Entrée pour fermer..." _
  exit 1
fi
if ! /usr/bin/python3 - "$source_dir/manifest.json" "$source_dir" <<'PY'
import json,sys,os
m=json.load(open(sys.argv[1],encoding='utf-8'))
assert m['id']=='liberty-factory-starter-100-v055' and len(m['presets'])==100
assert len({p['id'] for p in m['presets']})==100
assert all(os.path.isfile(os.path.join(sys.argv[2],z['file'])) for p in m['presets'] for z in p['zones'])
print('100 presets et échantillons vérifiés.')
PY
then
  echo "ERREUR : fichiers de la banque incomplets ou invalides."
  read -r -p "Appuyez sur Entrée pour fermer..." _
  exit 1
fi
mkdir -p "$bank_root"
backup=""
if [[ -e "$target" ]]; then
  backup="$bank_root/.Factory-Starter-v055-100-sauvegarde-$(date +%Y%m%d-%H%M%S)-$$"
  mv "$target" "$backup"
fi
if ! /usr/bin/ditto "$source_dir" "$target"; then
  echo "ERREUR : impossible d'installer la banque."
  rm -rf "$target"
  [[ -z "$backup" ]] || mv "$backup" "$target"
  read -r -p "Appuyez sur Entrée pour fermer..." _
  exit 1
fi
printf '\nINSTALLATION REUSSIE : 100 presets Factory Starter.\n'
printf 'Dossier : %s\n' "$target"
[[ -z "$backup" ]] || printf 'Ancienne banque de même version sauvegardée : %s\n' "$backup"
printf 'Les autres banques restent intactes. Fermez et relancez Liberty ROMpler.\n\n'
read -r -p "Appuyez sur Entrée pour fermer..." _
