# Liberty ROMpler v0.22 — Vérification sur macOS

Cette procédure concerne la v0.22 après compilation et installation. Aucun fichier WAV ou preset ne doit être modifié par le cadrage.

## Essai du nouveau cadrage

1. Dans Standalone, ouvrir **SAMPLES > BOUCLES**, charger un WAV et activer une boucle avec DÉBUT et FIN valides.
2. Relever les positions DÉBUT, FIN et FONDU ainsi que le zoom initial.
3. Faire **Option + double-clic sur la forme d'onde** : la boucle doit apparaître entière, au centre quand la place le permet, avec une marge autour de ses deux extrémités.
4. Vérifier que le curseur ZOOM et la barre POSITION reflètent le nouveau cadrage.
5. Faire **double-clic normal** sur la forme d'onde : retour à l'affichage complet du WAV.
6. Refaire avec une boucle très courte, une boucle presque égale à la durée du WAV, une boucle au début et une boucle en fin de fichier.
7. Vérifier que DÉBUT, FIN et FONDU ne changent jamais après ces commandes et que le bouton ANNULER n'est pas activé par un simple cadrage.

## Régressions

- **Maj + glisser** à l'intérieur de la boucle déplace toujours DÉBUT et FIN ensemble, sans changer sa durée.
- La souris déplace toujours les marqueurs individuellement en mode normal.
- Les commandes ±1/±10, ZÉRO, SAUT MIN, PAIRE, préécoute A/B et AUTO ZÉRO restent utilisables.
- Si la boucle est inactive ou invalide, Option + double-clic ne doit pas planter ni déplacer de marqueur.
- Tester les WAV longs, le zoom maximal, le Standalone puis si possible AU et VST3.

**Règles incontournables** : aucun chevauchement même sur écran 13 pouces ; aucun symbole incorrect à la place des accents français (é, è, ê, à, ç). Aucun changement des fichiers WAV, banques, presets ou du moteur audio temps réel.
