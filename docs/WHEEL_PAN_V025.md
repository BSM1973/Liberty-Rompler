# Liberty ROMpler v0.25 — Navigation horizontale à la molette

**Statut : en développement dans `feature/v025-shift-wheel-waveform-navigation`. La v0.24 reste la dernière distribution macOS confirmée tant que le build v0.25 n'est pas vert.**

## Nouvelle commande dans SAMPLES > BOUCLES

**Maj + molette verticale** permet de faire défiler horizontalement la forme d'onde, sans déplacer DÉBUT ou FIN.

- Molette vers le haut : naviguer vers les frames plus anciennes, au début du WAV.
- Molette vers le bas : naviguer vers les frames plus récentes, à la fin du WAV.
- Le déplacement correspond à environ **20 % de la fenêtre visible par unité de molette**. Les déplacements de précision d'un trackpad sont arrondis à la frame.
- En cas de valeurs de molette excessives, un événement est limité à 80 % de la fenêtre visible.
- Si le trackpad macOS transforme Maj + molette verticale en défilement horizontal, la nouvelle commande prend aussi en charge `deltaX`.
- La navigation reste bloquée aux limites du WAV ; si tout le sample est visible, elle ne déplace rien.
- La commande fonctionne avec une boucle désactivée, du moment qu'un WAV est sélectionné.

## Compatibilité

Les gestes existants restent inchangés :

- Molette verticale sans Maj : zoom classique autour du centre.
- Option + molette verticale : zoom centré sous le curseur (v0.23).
- Molette horizontale : navigation horizontale historique.
- Option + glisser : déplacer la fenêtre visible (v0.24).
- Maj + glisser : déplacer la boucle complète avec sa durée intacte (v0.21).
- Option + double-clic : cadrer la boucle (v0.22).
- ZÉRO, SAUT MIN, PAIRE, ±1/±10, préécoute A/B et ANNULER/APPLIQUER.

Aucun bouton ou panneau nouveau. Le moteur C++17 `Source/LoopWheelPanV025.h` reste indépendant de JUCE, ne modifie ni les samples WAV, ni les banques, les presets, l'état du DAW, ni le traitement dans `processBlock()`. Les sliders de ZOOM et POSITION restent synchronisés.

## Nos deux règles Liberty

**Aucun chevauchement de textes, boutons, curseurs ou menus**, y compris en fenêtre compacte sur écran 13 pouces.

**Aucun caractère erroné à la place des accents français**. Les chaînes C++ emploient les échappements Unicode et sont contrôlées lors de la prévalidation.

## Validation

- `tests/loop_wheel_pan_v025_test.cpp` : direction, amplitude, limites, grands entiers, trackpad et données invalides.
- `tests/test_v025_shift_wheel_pan.py` : compilation C++17, intégration JUCE, conservation des contrôles historiques et audit Unicode.
- Prévalidation Linux avec JUCE 8 : suite complète de tests et compilation VST3.
- **Un seul build macOS** après validation Linux, puis vérification réelle avec `docs/GUIDE_TEST_BOUCLES_V025.md`.

Les tests automatisés ne remplacent pas l'essai ergonomique réel sur Mac.
