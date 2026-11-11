# Liberty ROMpler v0.29 — Zoom de précision sur les marqueurs de boucle

**Statut : développement sur `feature/v029-precision-zoom-loop-markers`. La v0.28 est la dernière distribution macOS validée tant que le build v0.29 n'est pas vert.**

## Nouvelle commande dans SAMPLES > BOUCLES

- **Commande (⌘) + Option + clic sur ZÉRO DÉBUT** : agrandir et centrer le passage au voisinage du marqueur DÉBUT.
- **Commande (⌘) + Option + clic sur ZÉRO FIN** : agrandir et centrer le passage au voisinage du marqueur FIN.

Le zoom de précision vise une fenêtre d'environ **512 frames**, mais respecte le zoom maximal de l'application ; sur les WAV très longs, la fenêtre visible peut donc être plus large. Le geste ne diminue jamais le zoom si celui-ci est déjà plus précis que le zoom cible.

La position est bornée aux deux extrémités du fichier ; un FIN égal à la fin exclusive du WAV est accepté. La commande ne fait rien si aucune boucle valide n'est sélectionnée.

Le cadrage avant zoom est enregistré par l'historique de la v0.27 : **Maj + double-clic** permet de revenir au zoom et à la position précédents, puis de basculer entre les deux.

## Toutes les commandes antérieures restent disponibles

| Geste sur ZÉRO DÉBUT / FIN | Action |
|---|---|
| Clic simple | Calage historique au passage par zéro |
| Option + clic | SAUT MIN pour un seul marqueur |
| Maj + Option + clic | Optimisation PAIRE, durée constante |
| Commande + clic | Centrer le marqueur sans modifier le zoom (v0.28) |
| **Commande + Option + clic** | **Zoom de précision sur le marqueur (v0.29)** |

**Maj + Option reste prioritaire**, même si Commande est également maintenue. Tous les gestes de navigation dans la forme d'onde (molette, glissement, double-clics) continuent de fonctionner.

## Intégrité, interface et compilation

Le calculateur `Source/LoopMarkerDetailV029.h` est du C++17 sans JUCE ni allocation. L'éditeur `overlays/v015/SampleZoneView.h` applique simultanément le zoom et la position, en réutilisant le cadrage borné de la v0.28.

Cette nouveauté est **strictement visuelle** : ni DÉBUT, ni FIN, ni FONDU, ni WAV, banque, preset ou état du DAW ne sont modifiés. Aucun changement du moteur audio ou du callback `processBlock()`, aucun nouveau bouton et aucun chevauchement de composants.

Les deux règles Liberty restent obligatoires : **aucun chevauchement** graphique, notamment sur Mac 13 pouces, et **aucun caractère erroné** à la place des accents français.

Tests C++ : `tests/loop_marker_detail_v029_test.cpp`. Régressions JUCE et Unicode : `tests/test_v029_marker_detail.py`. Prévalidation Linux VST3 JUCE 8 complète, puis un seul build macOS AU, VST3 et Standalone, avec DMG, ZIP et sources.

La vérification manuelle est décrite dans `docs/GUIDE_TEST_BOUCLES_V029.md`.
