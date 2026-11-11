# Liberty ROMpler v0.31 — Horodatage précis des marqueurs DÉBUT et FIN

**Statut : en développement sur `feature/v031-loop-marker-time-tooltips`. La v0.30 est la dernière version macOS confirmée tant que la v0.31 n'est pas validée.**

## Informations supplémentaires dans SAMPLES > BOUCLES

Au survol des deux boutons ZÉRO existants :

- **ZÉRO DÉBUT** : l'infobulle affiche la frame absolue du marqueur DÉBUT et sa position en millisecondes depuis le début du fichier WAV.
- **ZÉRO FIN** : l'infobulle affiche la frame absolue du marqueur FIN et sa position en millisecondes depuis le début du fichier WAV.

Exemple avec un WAV à 48 kHz : DÉBUT = 24000 frames correspond à 500.000 ms ; FIN = 48000 frames correspond à 1000.000 ms. FIN est le point **exclusif** de la boucle et peut correspondre à l'extrémité du fichier.

Les valeurs utilisent le **taux d'échantillonnage réel du WAV** et sont recalculées lors de chaque modification des marqueurs. Elles sont réinitialisées en changeant de zone, et absentes si la boucle n'est pas active ou si le WAV n'est pas valide.

## Fonctions historiques préservées

Le texte des infobulles continue d'expliquer les raccourcis :

- Clic : rechercher le passage par zéro.
- Option + clic : SAUT MIN.
- Maj + Option + clic : PAIRE, durée fixe.
- Commande + clic : centrer le marqueur (v0.28).
- Commande + Option + clic : zoom de précision (v0.29).

La v0.30 continue d'afficher la durée de boucle en frames/ms dans la ligne sous la forme d'onde, et la durée du FONDU dans l'infobulle « Boucle activée ».

## Sécurité et présentation

Le calculateur `Source/LoopMarkerTimesV031.h` est du C++17 pur, sans JUCE ni allocation. Il ne modifie **aucun marqueur, FONDU, WAV, preset, banque ou projet**, et n'ajoute aucune opération à `processBlock()`.

**Aucun bouton, ligne ou panneau ajouté** : les nouvelles valeurs apparaissent uniquement dans les deux infobulles existantes. La disposition et la visibilité des faders demeurent inchangées.

Les deux règles Liberty restent impératives : **aucun chevauchement** et **aucun symbole parasite à la place des caractères accentués français**.

## Vérification

Tests `tests/loop_marker_times_v031_test.cpp` et `tests/test_v031_marker_times.py` : taux 44,1 à 192 kHz, limites des WAV longs, marqueurs exclusifs, taux invalides, absence de mutation, Unicode et maintien des raccourcis.

Prévalidation Linux JUCE 8 VST3, puis un seul build macOS AU, VST3 et Standalone lorsque la prévalidation est entièrement verte. Guide de test manuel : `docs/GUIDE_TEST_BOUCLES_V031.md`.
