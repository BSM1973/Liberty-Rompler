# Liberty ROMpler v0.26 — Recentrer la forme d'onde sans changer le zoom

**Statut : développement sur `feature/v026-command-doubleclick-center-view`. La v0.25 demeure la dernière version macOS confirmée tant que le build v0.26 n'est pas vert.**

## Nouvelle commande de navigation dans SAMPLES > BOUCLES

**Commande (⌘) + double-clic sur la forme d'onde** place la frame choisie au centre de la fenêtre visible lorsque les limites du WAV le permettent, **sans changer le niveau de zoom**.

- Le repère visé est calculé directement en frames depuis la position actuelle et la coordonnée de la souris.
- Près du début et de la fin du WAV, le cadrage est limité aux bornes du fichier.
- À zoom 1× (tout le fichier visible), le geste ne déplace rien.
- Le raccourci est utilisable même quand la boucle est désactivée, à condition d'avoir sélectionné un sample.
- Le curseur POSITION reste synchronisé ; le curseur ZOOM conserve exactement sa valeur.

## Raccourcis existants inchangés

- Double-clic normal : afficher tout le fichier WAV.
- Option + double-clic : cadrer automatiquement la boucle (v0.22), prioritaire même si Commande est aussi maintenue.
- Option + molette : zoomer autour du curseur (v0.23).
- Option + glisser : déplacer la vue (v0.24).
- Maj + molette : naviguer horizontalement (v0.25).
- Maj + glisser : déplacer les deux marqueurs ensemble (v0.21).
- Déplacement des marqueurs, ±1/±10, SAUT MIN, PAIRE, préécoute A/B, ANNULER et APPLIQUER conservés.

Le calculateur `Source/LoopCenterViewV026.h` est indépendant de JUCE (C++17, calculs intermédiaires 64 bits, sans allocation). Le branchement graphique utilise le composant existant `overlays/v015/SampleZoneView.h` : **aucun nouveau bouton, panneau ou redimensionnement**.

Le geste ne modifie ni DÉBUT, ni FIN, ni FONDU, ni les WAV, banques, presets ou l'état du DAW ; aucun traitement audio temps réel supplémentaire.

## Nos deux règles Liberty

**Zéro chevauchement** des boutons, menus et textes, notamment sur les écrans Mac 13 pouces.

**Aucun symbole erroné à la place des caractères français** : les libellés C++ restent protégés par les échappements Unicode et l'audit des tests.

## Validation

- `tests/loop_center_view_v026_test.cpp` : points de clic, conservation du zoom, limites WAV, fichiers extrêmement longs ou courts, données invalides.
- `tests/test_v026_center_view.py` : compilation du calculateur, cohérence des raccourcis et absence de modification audio.
- Prévalidation GitHub Linux : suite complète de tests et compilation JUCE 8 VST3.
- **Un seul build macOS AU, VST3 et Standalone** après Linux vert, avec DMG, ZIP et sources.
- Guide de recette : `docs/GUIDE_TEST_BOUCLES_V026.md`.

Le contrôle final du comportement réel de la souris et de la lisibilité de l'interface se fait après compilation macOS.
