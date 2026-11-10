# Liberty Rompler v0.14 — ZOOM et AUTO ZÉRO

Les réglages précédents de SAMPLES (mapping des notes, vélocités, points de boucle, fondu anti-clic, ZÉRO DÉBUT et ZÉRO FIN, APPLIQUER et ANNULER) restent disponibles.

## Zoom de la forme d'onde

Dans **SAMPLES > BOUCLES**, le curseur ZOOM agrandit la forme d'onde de **1× à 64×** sans affecter le son. Le curseur POSITION permet de parcourir l'enregistrement, y compris lorsque les deux marqueurs sont hors champ. La molette de la souris sur la forme d'onde peut aussi zoomer ; le double-clic ou le bouton **TOUT** revient à 1×. Le défilement horizontal déplace la vue lorsque celle-ci est agrandie.

Le dessin, la zone colorée de boucle et le fondu violet utilisent exactement le même repère en **frames**. Un clic ne déplace un marqueur que si le pointeur se trouve à 18 pixels au plus de celui-ci. Les marqueurs hors champ ne peuvent pas être déplacés accidentellement.

## Calage sur les passages par zéro

Les boutons **ZÉRO DÉBUT** et **ZÉRO FIN** existants fonctionnent à toute échelle de zoom. La nouvelle case **AUTO ZÉRO** applique la même recherche (rayon de 512 frames, lecture du WAV mono/stéréo) lorsque l'on termine un glissement de marqueur, et à la fin du déplacement des curseurs DÉBUT/FIN. Elle est désactivée par défaut et n'affecte pas les anciens projets. Les calculs n'ont pas lieu dans le moteur audio ; l'édition est non destructive.

Le calage au passage par zéro ne garantit pas l'absence absolue de clic : un fondu de boucle peut être nécessaire sur certains échantillons.

## Compatibilité

La navigation à 9 onglets et le clavier MIDI pleine largeur sont conservés. Aucun fichier WAV ou manifeste de banque n'est écrit. Les réglages de zoom sont un état d'affichage temporaire ; les points de boucle appliqués, leurs fondus et les plages MIDI continuent d'être enregistrés avec le projet DAW. Les modifications audio prennent effet après **APPLIQUER**.
