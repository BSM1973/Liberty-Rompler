# Liberty Rompler v0.14.1 — vérification des boucles sur Mac

Cette livraison conserve le moteur audio et l'interface v0.14. Elle améliore la fiabilité de la distribution et l'installation macOS ; le comportement audio doit toujours être confirmé dans le Standalone puis dans un DAW.

## Test visuel

1. Ouvrir le Standalone, charger un son tenu et aller dans **SAMPLES > BOUCLES**.
2. Confirmer que **AUTO ZÉRO** est désactivé au premier affichage, pour respecter les anciens projets.
3. Zoomer progressivement de 1× à 16×, puis 64× ; déplacer la position horizontale et vérifier l'alignement de l'onde, des marqueurs et de la zone de fondu.
4. Déplacer un marqueur proche du pointeur ; cliquer loin d'un marqueur et vérifier qu'aucun marqueur ne saute.
5. Activer **AUTO ZÉRO** et déplacer à nouveau un marqueur : le point de boucle doit rejoindre le passage par zéro valide le plus proche. Tester aussi **ZÉRO DÉBUT** et **ZÉRO FIN**.
6. Utiliser **TOUT** et le double-clic de réinitialisation du zoom. Revenir à une vue globale sans décalage des marqueurs.
7. Vérifier que les accents français sont corrects et que boutons, libellés, marqueurs et commandes ne se chevauchent pas, même avec une fenêtre réduite.

## Test sonore

1. Activer la boucle sur un son tenu, maintenir une note MIDI et écouter plusieurs répétitions.
2. Écouter avec un fondu de boucle nul, puis augmenter progressivement le fondu anti-clic. Comparer avec AUTO ZÉRO désactivé et activé.
3. Tester un WAV mono puis stéréo ; noter tout clic, décrochage, variation de volume ou artefact aux points de raccord.
4. Sauvegarder et rouvrir le projet DAW : les positions et le fondu enregistrés doivent être restaurés, sans modifier les fichiers WAV d'origine.
5. Tester un ancien projet : aucun déplacement automatique ne doit survenir tant qu'AUTO ZÉRO n'est pas activé manuellement.

## Installation

- Utiliser **INSTALLER_MAC.command** pour installer le Standalone, AU, VST3 et les banques par utilisateur, sans privilèges administrateur.
- Les installations précédentes des trois bundles sont sauvegardées à côté de leur destination. Les banques déjà présentes ne sont pas écrasées.
- Utiliser **DIAGNOSTIC_MAC.command** pour vérifier l'architecture, les emplacements, les signatures et les banques sans modifier la machine.
- Cette version de test est signée ad hoc et n'est pas notarisée par Apple.

**À relever en cas d'anomalie :** DAW et version, macOS, format AU/VST3/Standalone, nom du preset, WAV mono/stéréo, zoom, état AUTO ZÉRO, réglage de fondu et éventuellement une vidéo ou un court enregistrement audio.