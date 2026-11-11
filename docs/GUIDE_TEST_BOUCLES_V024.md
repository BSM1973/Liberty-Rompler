# Liberty ROMpler v0.24 — Recette macOS Option + glisser

Cette recette se réalise sur la v0.24 **après compilation et installation**, avec un preset contenant un WAV long.

## Vérifier le déplacement horizontal

1. Dans **SAMPLES > BOUCLES**, sélectionner un sample, activer le zoom puis noter DÉBUT, FIN, FONDU et la position de la barre POSITION.
2. Maintenir **Option**, cliquer dans la forme d'onde et **glisser vers la droite** : le contenu doit se déplacer vers la droite et révéler des échantillons antérieurs.
3. Toujours en maintenant le bouton de souris, glisser vers la gauche, puis revenir à l'endroit du clic initial : la position doit revenir exactement, sans dérive cumulative.
4. Répéter sur un marqueur ou dans une zone colorée : **Option + glisser** doit déplacer la vue, pas les marqueurs.
5. Tester la navigation jusqu'au début puis jusqu'à la fin du WAV : aucun déplacement hors du fichier ne doit être possible.
6. Avec tout le fichier visible (zoom 1×), tenter Option + glisser : la vue ne doit pas bouger.
7. Désactiver la boucle et vérifier que le déplacement de la forme d'onde reste possible.
8. Vérifier la synchronisation avec le curseur POSITION.

## Tests de régression

- Glisser normalement DÉBUT ou FIN déplace toujours un seul marqueur.
- **Maj + glisser** à l'intérieur de la boucle déplace les deux marqueurs ensemble.
- **Maj + Option + glisser** dans une boucle conserve le comportement Maj de la v0.21.
- Option + molette zoome sur le point visé ; Option + double-clic cadre la boucle.
- Les boutons ±1/±10, ZÉRO, SAUT MIN, PAIRE et préécoute A/B restent opérationnels.
- À aucun moment Option + glisser ne modifie DÉBUT, FIN ou FONDU, n'active une édition non appliquée ou ne déclenche AUTO ZÉRO.
- Les fichiers WAV et les banques restent intacts.

## Règles Liberty

**Aucun chevauchement** : vérifier fenêtre compacte et écran 13 pouces.

**Aucun caractère incorrect** : vérifier é, è, ê, à et ç dans les libellés et les infobulles.

Tester en Standalone, puis si possible dans un DAW avec AU et VST3. Signaler les coordonnées approximatives de la souris, le zoom, la longueur du WAV et la position avant/après en cas d'écart.
