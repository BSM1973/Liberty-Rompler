#!/bin/bash
# Liberty Rompler v0.9 - one local macOS build, no paid GitHub Actions.
set -Eeuo pipefail
ROOT="$HOME/Liberty-Rompler"
WORK="$ROOT/.local-build"
SRC="$WORK/source"
JUCE="$WORK/JUCE"
BUILD="$WORK/build"
DIST="$WORK/dist"
trap 'echo "ERREUR : compilation interrompue. Voir $WORK/compilation.log" >&2' ERR
if [ "$(uname -s)" != Darwin ] || [ "$(uname -m)" != arm64 ]; then
  echo "Ce script exige un Mac Apple Silicon (arm64)." >&2; exit 1
fi
if [ "$#" -gt 1 ] || { [ "$#" -eq 1 ] && [ "$1" != '--install' ]; }; then
  echo "Usage : bash scripts/build_local_github_v09.command [--install]" >&2; exit 2
fi
for tool in git cmake xcodebuild python3 shasum patch xz tar base64 ditto hdiutil codesign; do
  command -v "$tool" >/dev/null || { echo "Outil manquant : $tool" >&2; exit 1; }
done
if [ ! -d "$ROOT/.git" ]; then
  if [ -e "$ROOT" ]; then echo "Dossier non Git deja present : $ROOT" >&2; exit 1; fi
  git clone https://github.com/BSM1973/Liberty-Rompler.git "$ROOT"
else
  git -C "$ROOT" pull --ff-only
fi
mkdir -p "$WORK" "$DIST"
rm -rf "$SRC"
mkdir -p "$SRC"
verify_sha() { printf '%s  %s\n' "$1" "$2" | shasum -a 256 -c -; }
verify_sha 0da09c77c326780c5f972a45984be0f345ff3a39639c340192e04c2a33476e9e "$ROOT/assets/rompler-source-v0.1.tar.xz"
tar -xJf "$ROOT/assets/rompler-source-v0.1.tar.xz" -C "$SRC"
cat "$ROOT"/assets/ui-v02/part-{1..5}.b64 | base64 -D > "$WORK/ui-v02.tar.xz"
verify_sha 3cfe2791987fd1a9e4b51abb60696b24b24da4ef09d45a9ce93e59652d9fe061 "$WORK/ui-v02.tar.xz"
tar -xJf "$WORK/ui-v02.tar.xz" -C "$SRC"
apply() {
  local version="$1" count="$2" hash="$3"
  local archive="$WORK/ui-$version.patch.xz"
  local i
  : > "$WORK/ui-$version.b64"
  for ((i=1; i<=count; ++i)); do cat "$ROOT/assets/ui-$version/part-$i.b64" >> "$WORK/ui-$version.b64"; done
  base64 -D < "$WORK/ui-$version.b64" > "$archive"
  verify_sha "$hash" "$archive"
  (cd "$SRC" && xz -dc "$archive" | patch --batch -p1)
}
apply v03 4 2048ee0ec90574a07c52fa738a907f7e1570cbf9f373e9305aa0ee6147041c94
apply v04 3 37fce78e5a9eb0dfdb4c6106b25540ad26f1e5f93276ed51d467f47e07950d1d
apply v05 5 103afbbefa19e00f88128e6f21d2980dd6cd7862f410ca09f4987d2cf683fc57
apply v06 6 fdea4e597422beba2f9dbf2a902e53a81e204d579dca786211d1583c8dfd1f1a
mkdir -p "$SRC/scripts"
cp "$ROOT/scripts/fix_v06_header.py" "$SRC/scripts/fix_v06_header.py"
python3 "$SRC/scripts/fix_v06_header.py"
apply v07 5 8dc6ce8ec8633453f4f06f27757f4613898a17c7f83166c6f3011cb5f4c61e8d
apply v08 6 8eaf16d5c4366d8442c555e00085dfdde3866353b55a7a96c9f3780f0ffd279a
apply v09 4 36d7ad16a83946851159444f5f874fcf336f7271ccb155349bd3a17f64660f7e
# All C++ MIDI tests run before downloading JUCE or building plugins.
if [ ! -x "$WORK/venv/bin/python" ]; then python3 -m venv "$WORK/venv"; fi
PYTHON="$WORK/venv/bin/python"
if ! "$PYTHON" -c 'import numpy' >/dev/null 2>&1; then "$PYTHON" -m pip install numpy; fi
"$PYTHON" "$SRC/tools/make_demo_banks.py"
"$PYTHON" "$SRC/tools/validate_banks.py"
"$PYTHON" "$SRC/tools/audit_ui_text.py"
(cd "$SRC" && "$PYTHON" -m unittest discover -s tests -p 'test_*.py' -v)
if [ ! -f "$JUCE/CMakeLists.txt" ]; then
  git clone --depth 1 --branch 8.0.10 https://github.com/juce-framework/JUCE.git "$JUCE"
fi
cmake -S "$SRC" -B "$BUILD" -G Xcode -DJUCE_DIR="$JUCE" -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build "$BUILD" --config Release --target LibertyRompler_AU LibertyRompler_VST3 LibertyRompler_Standalone --parallel 2 2>&1 | tee "$WORK/compilation.log"
PRODUCTS="$BUILD/LibertyRompler_artefacts"
APP="$(find "$PRODUCTS" -type d -name '*.app' -print -quit)"
VST="$(find "$PRODUCTS" -type d -name '*.vst3' -print -quit)"
AU="$(find "$PRODUCTS" -type d -name '*.component' -print -quit)"
[ -n "$APP" ] && [ -n "$VST" ] && [ -n "$AU" ] || { echo 'Plugin absent' >&2; exit 1; }
PKG="$DIST/Liberty-Rompler-macOS-arm64-v0.9-LOCAL"
rm -rf "$PKG"
mkdir -p "$PKG"
ditto "$APP" "$PKG/Liberty Rompler.app"
ditto "$VST" "$PKG/Liberty Rompler.vst3"
ditto "$AU" "$PKG/Liberty Rompler.component"
ditto "$SRC/Banks" "$PKG/Banks"
cp "$ROOT/scripts/install_mac.command" "$PKG/INSTALLER_MAC.command"
chmod +x "$PKG/INSTALLER_MAC.command"
for item in "$PKG/Liberty Rompler.app" "$PKG/Liberty Rompler.vst3" "$PKG/Liberty Rompler.component"; do
  codesign --force --deep --sign - "$item" && codesign --verify --deep "$item"
done
(cd "$DIST" && ditto -c -k --sequesterRsrc --keepParent "$(basename "$PKG")" "$(basename "$PKG").zip")
hdiutil create -volname 'Liberty Rompler v0.9' -srcfolder "$PKG" -ov -format UDZO "$PKG.dmg" >/dev/null
if [ "$#" -eq 1 ]; then
  data="$HOME/Library/Application Support/Liberty Music Software/Liberty Rompler"
  mkdir -p "$data/Backups"
  if [ -d "$data/Banks" ]; then
    stamp="$(date '+%Y%m%d-%H%M%S')"
    ditto "$data/Banks" "$data/Backups/Banks-$stamp"
  fi
  (cd "$PKG" && bash ./INSTALLER_MAC.command)
fi
echo "BUILD LOCAL v0.9 VERT : $PKG"
echo "DMG : $PKG.dmg"
echo "ZIP : $PKG.zip"
