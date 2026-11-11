# Liberty ROMpler v0.29 — Recette de zoom précis sur macOS

À réaliser une fois le build macOS v0.29 terminé, dans **SAMPLES > BOUCLES**.

1. Charger un WAV contenant une boucle valide ; relever DÉBUT, FIN, FONDU, zoom et position.
2. Partir d'une vue relativement large. Faire **⌘ + Option + clic sur ZÉRO DÉBUT** : le marqueur doit se rapprocher du centre et le zoom augmenter dans la limite du zoom maximal.
3. Faire **⌘ + Option + clic sur ZÉRO FIN** : l'autre extrémité doit être cadrée ; vérifier que le zoom reste identique s'il est déjà assez fort.
4. Faire **Maj + double-clic dans la forme d'onde** : le cadrage précédent (zoom et position) doit revenir ; répéter pour basculer entre les deux.
5. Mettre le zoom à son maximum avant de cliquer : le nouveau geste doit recentrer sans dézoomer.
6. Vérifier DÉBUT près de 0 et FIN à l'extrémité exclusive du WAV ; l'image ne doit jamais dépasser la longueur du fichier.
7. Désactiver la boucle et essayer le nouveau geste : aucune modification des réglages ou saut imprévu.
8. Après chaque zoom de précision, confirmer que DÉBUT, FIN, FONDU et les paramètres du projet n'ont pas changé.

## Contrôles de non-régression

- Clic normal sur ZÉRO : calage zéro historique.
- Option + clic : SAUT MIN.
- Maj + Option + clic : optimisation PAIRE.
- Commande + clic : simple centrage, sans zoom.
- Maj + double-clic : retour au cadrage précédent.
- Double-clic normal : vue WAV complète ; Option + double-clic : cadrage boucle ; Commande + double-clic : recentrage à l'endroit visé.
- Option + molette, Maj + molette, Option + glisser, Maj + glisser, préécoute A/B et ±1/±10 toujours fonctionnels.

**Deux règles :** aucun chevauchement dans la fenêtre même sur écran 13 pouces ; affichage normal de é, è, ê, à et ç. Test dans Standalone et, si possible, AU et VST3 au sein d'un DAW.
