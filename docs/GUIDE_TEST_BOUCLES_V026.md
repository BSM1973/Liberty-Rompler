# Liberty ROMpler v0.26 — Vérification du recentrage sur Mac

Cette recette concerne la v0.26 **après compilation et installation**.

## Commande + double-clic

1. Dans **SAMPLES > BOUCLES**, charger un WAV long et choisir un zoom intermédiaire.
2. Relever la valeur du curseur ZOOM, DÉBUT, FIN, FONDU et la position de la barre POSITION.
3. Repérer un transitoire au quart gauche de la forme d'onde, puis faire **⌘ + double-clic** exactement dessus. Il doit se retrouver près du centre de l'affichage.
4. Vérifier que **ZOOM n'a pas changé**, que POSITION a suivi, et que DÉBUT, FIN et FONDU sont restés identiques.
5. Répéter avec un point situé à droite, puis près du début et de la fin du WAV : la vue ne doit jamais sortir des limites du fichier.
6. À zoom 1×, Commande + double-clic ne doit pas déplacer la vue.
7. Désactiver la boucle et refaire le geste : il doit encore fonctionner avec un sample sélectionné.

## Régressions

- Double-clic normal : affichage de tout le WAV.
- Option + double-clic : cadrage de boucle conservé.
- Option + Commande + double-clic : priorité au cadrage de boucle (Option).
- Option + molette : zoom sur le point visé.
- Option + glisser : déplacement horizontal de la vue.
- Maj + molette : navigation horizontale.
- Maj + glisser dans la boucle : déplacement couplé.
- ±1/±10, ZÉRO, SAUT MIN, PAIRE, préécoute A/B, ANNULER et APPLIQUER : aucun changement.
- Après Commande + double-clic, aucun état NON APPLIQUÉ, aucune écriture de WAV ou recalage AUTO ZÉRO.

## Deux règles obligatoires

1. **Aucun chevauchement** des textes, menus, curseurs ou boutons sur un écran de 13 pouces.
2. **Accents français intacts** : é, è, ê, à et ç ne doivent pas apparaître comme des symboles.

Effectuer le test dans Standalone, puis si possible en AU et VST3 dans un DAW.
