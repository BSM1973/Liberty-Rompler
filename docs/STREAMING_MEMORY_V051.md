# Liberty ROMpler v0.51 — optimisation RAM et stabilité temps réel

## Référence v0.50
Banc réel JUCE hors périphérique audio, macOS Apple Silicon, 44,1 kHz / 512 frames : 0 sous-alimentation poly64, 1 sous-alimentation SLICE, 5 dépassements de budget poly64, 502 Mo RSS maximum pendant le changement de presets. Ces chiffres sont une référence, pas une garantie tous Macs/DAW.

## Changements
- Réduction de **32 à 16 pages PCM de 4 096 frames par zone WAV longue**, conservant 4 workers, 96 demandes en attente et le protocole atomique anti-éviction. Mémoire théorique des caches pour 64 zones : 64 Mio → 32 Mio ; pour 128 zones simultanément vivantes pendant une transition : 128 Mio → 64 Mio. Les attaques RAM 1,5 s et les têtes de boucles v0.50 ne sont pas modifiées.
- Réutilisation des **couches identiques** (ID banque + ID preset + nombre de zones inchangé). Les références JUCE sont retenues avant la purge des anciens sons, puis réattachées au synthétiseur : décodeur, cache, fenêtre de boucle et tables PCM ne sont pas dupliqués inutilement. Les voix sont toujours coupées comme avant et les sélecteurs RR/articulation réinitialisés. La table spectrale reste en place pour les couches réutilisées.
- **Sauvegardes DAW et modifications de zones** : rechargement forcé pour ne jamais réutiliser une zone périmée ; les presets, formats de banques et anciens paramètres sont inchangés.
- **Rapports audio** enrichis d'un relevé des pics de temps de callback et de la RAM résidente à chaque changement de preset (mesures *hors* callback).

## Critères de qualification
Comparer v0.51 et v0.50 avec la même banque et les mêmes scénarios poly64, switch et SLICE. Viser une baisse nette de mémoire dans switch et aucun incident supplémentaire ; mesurer le CPU processus, les latences 99e percentile et le maximum du callback. En cas de réapparition d'underflows, retirer ou ajuster la réduction de cache avant fusion.

Les 5 dépassements du budget en v0.50 sont des durées murales de callback, potentiellement perturbées par l'ordonnancement du runner. L'objectif est de les réduire, mais leur origine ne peut pas être prouvée par un simple pourcentage CPU. Un test en AU/VST3 dans Ableton/Logic/Bitwig et avec **un support USB lent physique** reste obligatoire avant qualification « production ». Aucune manipulation de périphérique utilisateur n'a été effectuée depuis CI.
