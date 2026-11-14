# Liberty ROMpler v0.55 — SPECTRAL DNA (lot A : fondation)

## Statut
Branche de développement, **pas encore un effet audible dans AU/VST3**. Le moteur spectral v0.48 et ORBIT v0.54 restent inchangés. Aucun nouveau paramètre host, aucun changement de banque, aucune modification du thread audio et aucun build macOS déclenché par ce lot.

## Architecture
`Source/SpectralDNAV055.h` apporte une représentation de 128 bandes d'une empreinte harmonique par couche (A/B/C/D), la normalisation défensive des magnitudes, des poids XY bilinéaires, et une interpolation du profil cible à quatre couches. Elle propose un contrôle Wet et une priorité aux transitoires à l'attaque. `Settings.enabled` est **false** par défaut ; `wet` est **0** par défaut.

L'extraction FFT des profils et leur publication dans un cache immuable devront être réalisées hors du callback. Ce lot ne prétend pas faire la FFT, la resynthèse, la synchronisation du cache ou le traitement spectral en temps réel. Le branchement devra réutiliser le moteur spectral v0.48 plutôt qu'en créer un deuxième.

## Étapes suivantes
1. Examiner les interfaces du moteur v0.48 reconstruit, leurs coûts CPU et le protocole de publication des profils.
2. Extraire hors thread audio quatre profils de spectre depuis des fenêtres de samples; prévoir la régénération lors d'un changement de banque.
3. Raccorder les coefficients calculés à la resynthèse existante, avec dry/wet lissé et protection des attaques; aucun lock, FFT ni allocation dans processBlock.
4. Ajouter paramètres automatisables, stockage rétrocompatible et contrôles dans MORPH sans chevauchement. L'ancienne session doit garder un rendu bit-identique avec SPECTRAL DNA OFF.
5. Tester silence, NaN, basses fréquences, harmoniques, transitoires, quatre couches, streaming et SLICE; Linux d'abord, un seul build macOS après prévalidation et tests d'écoute réels.

## Tests de ce lot
`tests/spectral_dna_v055_test.cpp` vérifie les quatre coins, le centre XY, le bypass, la protection des transitoires et la robustesse des entrées invalides. La CI Linux compile en C++17 avec avertissements stricts et UBSan.
