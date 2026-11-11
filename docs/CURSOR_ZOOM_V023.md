# Liberty ROMpler v0.23 — Zoom précis à l'endroit de la souris

**Statut : développement sur `feature/v023-pointer-anchored-waveform-zoom`. La v0.22 reste la dernière distribution macOS confirmée tant que la v0.23 n'est pas validée.**

## Nouveau geste dans SAMPLES > BOUCLES

- **Molette verticale normale** : zoom de forme d'onde autour du centre de la vue, comme dans les versions précédentes.
- **Option + molette verticale** : zoom autour du point de la forme d'onde situé **sous le curseur de la souris**. La frame visée reste sous le curseur lorsque l'espace disponible le permet.
- **Défilement horizontal** : déplacement de la zone visible, comportement historique inchangé.
- **Double-clic normal** : voir tout le WAV ; **Option + double-clic** : cadrer automatiquement la boucle active, comme en v0.22.

Une infobulle sur le libellé ZOOM documente le raccourci. Aucun bouton, panneau ni texte permanent supplémentaire n'est créé.

## Garde-fous

Le moteur C++17 `Source/LoopCursorZoomV023.h` calcule la nouvelle position de la fenêtre à partir de la position actuelle, de la largeur en pixels et de la frame ciblée. Il refuse les spans invalides, supporte les longs WAV, limite le curseur aux bords de l'affichage et empêche que la fenêtre déborde du fichier.

Le curseur de ZOOM et la barre POSITION restent synchronisés. Le zoom ne doit modifier **ni DÉBUT, ni FIN, ni FONDU**, ni le WAV, les banques ou les presets. Il n'exécute aucun code supplémentaire dans le callback audio `processBlock()`.

## Compatibilité

Les actions des v0.15 à v0.22 sont conservées : zoom classique, déplacement horizontal, cadrage Option + double-clic, Maj + glisser pour déplacer toute la boucle, ±1/±10, préécoute A/B, SAUT MIN/PAIRE, ANNULER/APPLIQUER.

Les deux règles Liberty restent obligatoires : **aucun chevauchement** (pas de nouveau composant), et **accents français corrects** (échappements Unicode vérifiés dans les sources C++).

## Validation

`tests/loop_cursor_zoom_v023_test.cpp` valide l'ancrage, les cas de débordement, les fichiers à une seule frame et les longueurs extrêmes. `tests/test_v023_cursor_zoom.py` contrôle les branchements de l'éditeur, l'absence de modifications audio et les échappements de texte. La prévalidation compile le VST3 JUCE 8 sous Linux avant un seul build macOS AU/VST3/Standalone.

Les vérifications manuelles macOS sont décrites dans `docs/GUIDE_TEST_BOUCLES_V023.md`.
