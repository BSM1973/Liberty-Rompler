# Liberty ROMpler v0.16 — Préécoute des WAV volumineux

**Statut : lot de développement préparé sur une branche indépendante. La v0.15 verte publiée n'est pas modifiée. Aucun nouveau build macOS déclenché.**

## Problème corrigé

En v0.15, le bouton PRÉÉCOUTE lit tout le WAV et en crée une copie en mémoire avant de rendre seulement quelques millisecondes autour du raccord de boucle. Un multisample long peut donc provoquer une allocation mémoire et un délai inutiles.

## Changement

`Source/LoopPreviewWindowsV016.h` construit exactement le même signal de préécoute à partir de deux fenêtres : l'une immédiatement avant la fin exclusive de boucle, l'autre immédiatement après le début inclusif. La courbe de fondu reste identique à `LoopCrossfade::at()` de la v0.14.

`scripts/apply_v016_preview.sh` doit s'exécuter **après** `scripts/apply_v015_overlays.sh` et remplace la lecture intégrale par deux appels `reader->read()` bornés. Il ne touche pas au `processBlock()` audio, au DAW, aux presets ou aux fichiers WAV.

- Mémoire temporaire proportionnelle à la durée de préécoute, pas à la durée totale du sample.
- Jusqu'à 32 768 frames par fenêtre et deux canaux au maximum ; validation préalable des bornes.
- Le décodage et le calcul restent hors du callback audio.
- Aucune modification des libellés français ou de la géométrie de l'interface.
- Aucun changement du format d'état DAW et AUTO ZÉRO reste désactivé par défaut.

## Vérifications prévues

Le test C++ `tests/loop_preview_windows_v016_test.cpp` compare l'algorithme optimisé à la sortie v0.15 pour plusieurs tailles de WAV, positions, longueurs de contexte, fondus et deux canaux. `tests/test_v016_preview_windows.py` l'intègre à la suite Python existante.

Avant de publier une v0.16, exécuter ces tests, l'audit UTF-8 et les vérifications JUCE sur Linux. Puis effectuer **un seul build macOS** regroupé lorsque l'intégration des autres améliorations éventuelles sera validée.

## État des livraisons

- **v0.15** : build #22, tentative 2, vert ; artifact macOS disponible.
- **v0.16** : travail non fusionné, pas d'artifact de distribution.
