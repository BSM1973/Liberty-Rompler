# Liberty ROMpler v0.35 — Durées avant et après la boucle

La v0.35 complète la répartition en pourcentage de v0.34 en ajoutant, sur une troisième ligne de l'infobulle BOUCLE activée, les durées **AVANT** et **APRÈS** en frames et en millisecondes.

Calcul sur le WAV source (FIN exclusif) :

- Frames AVANT = DÉBUT.
- Frames APRÈS = nombre de frames du WAV − FIN.
- Durée AVANT (ms) = 1000 × DÉBUT / fréquence du WAV.
- Durée APRÈS (ms) = 1000 × (frames totales − FIN) / fréquence du WAV.

Exemple : WAV de 96000 frames, 48 kHz ; DÉBUT 24000, FIN 48000 : AVANT 24000 frames, 500,000 ms ; APRÈS 48000 frames, 1000,000 ms.

Moteur pur C++17 : `Source/LoopSegmentTimesV035.h`. Validation préalable par v0.34, puis contrôle de la fréquence réelle du fichier et des résultats non finis. Aucun nouveau bouton, libellé ou panneau ; aucun changement du callback audio, WAV, banques, marqueurs, presets ou projets. Les valeurs sont recalculées avec l'infobulle, sans accumulation d'anciennes mesures.

Tests : `tests/loop_segment_times_v035_test.cpp` et `tests/test_v035_loop_segment_times.py`. Guide macOS : `docs/GUIDE_TEST_BOUCLES_V035.md`.

Prévalidation Linux du code d'abord, puis une unique compilation macOS après validation. La v0.34 reste la base en attendant son build définitif.
