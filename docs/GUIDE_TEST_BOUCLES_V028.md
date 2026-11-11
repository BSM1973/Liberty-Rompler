# Liberty ROMpler v0.28 — Vérifications macOS

Ces étapes sont à réaliser **après compilation et installation** dans SAMPLES > BOUCLES, d'abord avec Standalone, puis si possible avec AU et VST3.

1. Charger un WAV long avec une boucle active. Noter DÉBUT, FIN, FONDU, zoom et position de la vue.
2. Choisir un zoom intermédiaire. Maintenir **Commande (⌘)** et cliquer sur **ZÉRO DÉBUT** : le marqueur DÉBUT doit être centré lorsque la place le permet, **sans changer sa valeur ni le zoom**.
3. Faire **⌘ + clic sur ZÉRO FIN** : le marqueur FIN doit être centré de la même manière, avec un défilement limité au bout du WAV.
4. Faire **Maj + double-clic dans la forme d'onde** : la vue précédente doit être restaurée. Un deuxième Maj + double-clic doit alterner avec la vue suivante.
5. Tester une boucle tout près du début et une boucle dont FIN vaut la fin exclusive du WAV : aucun débordement ni modification de la longueur.
6. Désactiver la boucle : Commande + clic ne doit plus déplacer la vue ni modifier le sample.
7. Refaire avec zoom 1× (tout le WAV visible) : aucun déplacement.

## Régressions obligatoires

- Clic simple sur ZÉRO DÉBUT / FIN : optimisation zéro historique.
- Option + clic : SAUT MIN. Maj + Option + clic : optimisation PAIRE.
- Commande + double-clic de la v0.26 : recentrage sous le curseur.
- Maj + double-clic de la v0.27 : retour au cadrage précédent.
- Option + glisser, Maj + glisser, Option + molette, Maj + molette et défilement horizontal.
- Le curseur POSITION suit les changements et le curseur ZOOM conserve sa valeur.
- Les marqueurs DÉBUT/FIN, FONDU, le fichier WAV et l'état ANNULER/APPLIQUER restent inchangés après le nouveau geste.

## Nos deux règles

**Aucun chevauchement** des textes, boutons, valeurs et repères, notamment sur un Mac 13 pouces. **Aucun symbole erroné** à la place des accents é, è, ê, à et ç.
