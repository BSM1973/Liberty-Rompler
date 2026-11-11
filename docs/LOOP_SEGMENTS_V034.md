# Liberty ROMpler v0.34 — Répartition du WAV autour de la boucle

## Amélioration

Dans SAMPLES > BOUCLES, l'infobulle existante « Boucle activée » conserve PART DU WAV (v0.33) et ajoute **AVANT LA BOUCLE** et **APRÈS LA BOUCLE** sur une seconde ligne. Le calcul utilise uniquement les frames du fichier original, FIN exclusif, et ne dépend pas du tempo ni de la fréquence du DAW.

- AVANT = 100 × DÉBUT / frames totales.
- DANS = 100 × (FIN − DÉBUT) / frames totales (v0.33, inchangé).
- APRÈS = 100 × (frames totales − FIN) / frames totales.

Exemple : WAV de 96000 frames, DÉBUT à 24000, FIN à 48000 : 25,00 % avant, 25,00 % dans la boucle, 50,00 % après. Somme mathématique : 100 %. L'affichage à deux décimales peut présenter une différence de 0,01 % après arrondi.

## Compatibilité et sécurité

La fonction C++17 `Source/LoopSegmentsV034.h` réutilise la validation de v0.33. Pas de nouveaux composants JUCE, de marqueurs déplacés, de modification des samples/presets/banques, de traitement dans le callback audio ni de changement des anciennes commandes. L'infobulle est sur deux lignes pour éviter une seule ligne trop longue. Tous les accents sont encodés avec des séquences Unicode explicites.

Tests : `tests/loop_segments_v034_test.cpp` et `tests/test_v034_loop_segments.py` ; guide manuel : `docs/GUIDE_TEST_BOUCLES_V034.md`.

Statut : code à prévalider, puis compilation macOS unique AU/VST3/Standalone si les tests sont satisfaisants.
