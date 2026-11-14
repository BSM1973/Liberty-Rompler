# Liberty ROMpler v0.51 — optimisation RAM et stabilité temps réel

## Référence v0.50
Banc réel JUCE hors périphérique audio, macOS Apple Silicon, 44,1 kHz / 512 frames : 0 sous-alimentation poly64, 1 sous-alimentation SLICE, 5 dépassements de budget poly64, 502 Mo RSS maximum pendant le changement de presets. Ces chiffres sont une référence, pas une garantie tous Macs/DAW.

## Changements
- **Cache de 32 pages PCM conservé** (4 096 frames par page, 4 workers, 96 requêtes et protection atomique contre les évictions). Un premier essai à 16 pages a réduit la RAM mais augmenté les sous-alimentations SLICE de 1 à 7, sans améliorer les 5 dépassements audio. Nous avons donc rétabli 32 pages pour préserver la lecture sur disque. Les attaques RAM 1,5 s et les têtes de boucle v0.50 restent inchangées.
- Réutilisation des **couches identiques** (ID banque + ID preset + nombre de zones inchangé). Les références JUCE sont retenues avant la purge des anciens sons, puis réattachées au synthétiseur : décodeur, cache, fenêtre de boucle et tables PCM ne sont pas dupliqués inutilement. Les voix sont toujours coupées comme avant et les sélecteurs RR/articulation réinitialisés. La table spectrale reste en place pour les couches réutilisées.
- **Sauvegardes DAW et modifications de zones** : rechargement forcé pour ne jamais réutiliser une zone périmée ; les presets, formats de banques et anciens paramètres sont inchangés.
- **Rapports audio** enrichis d'un relevé des pics de temps de callback et de la RAM résidente à chaque changement de preset (mesures *hors* callback).

## Résultat du premier banc v0.51 et décision corrective
La première compilation v0.51 a mesuré 233 Mo de pic mémoire lors des switches (contre 502 Mo en v0.50), 0 sous-alimentation poly64, mais **7 sous-alimentations SLICE contre 1 précédemment** et 6 dépassements poly64 contre 5. Le cache réduit n'est donc pas retenu. La diminution de RAM visée doit venir avant tout de la réutilisation des couches identiques. Un second benchmark macOS avec 32 pages permettra de qualifier ou non ce correctif ; ne pas recopier les métriques de la première exécution comme résultats finaux.

## Critères de qualification
Comparer v0.51 et v0.50 avec la même banque et les mêmes scénarios poly64, switch et SLICE. Viser une baisse nette de mémoire dans switch et aucun incident supplémentaire ; mesurer le CPU processus, les latences 99e percentile et le maximum du callback. En cas de réapparition d'underflows, conserver 32 pages et réexaminer la prélecture par couche avant d'autres réductions.

Les 5 dépassements du budget en v0.50 sont des durées murales de callback, potentiellement perturbées par l'ordonnancement du runner. L'objectif est de les réduire, mais leur origine ne peut pas être prouvée par un simple pourcentage CPU. Un test en AU/VST3 dans Ableton/Logic/Bitwig et avec **un support USB lent physique** reste obligatoire avant qualification « production ». Aucune manipulation de périphérique utilisateur n'a été effectuée depuis CI.
