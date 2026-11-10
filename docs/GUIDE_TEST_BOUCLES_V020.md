# Liberty ROMpler v0.20 — Contrôle macOS du déplacement couplé manuel

Ce guide concerne la v0.20 **après compilation et installation**. Aucun WAV ne doit être réécrit.

## Déplacement d'un seul marqueur

1. Ouvrir l'application Standalone et choisir une zone contenant une boucle dans **SAMPLES > BOUCLES**.
2. Noter DÉBUT, FIN et FIN − DÉBUT.
3. Cliquer sur chacun des boutons `−1` et `+1` : seul le marqueur correspondant doit bouger de 1 frame.
4. Refaire avec **Option + clic** : le marqueur doit bouger de 10 frames, dans les limites du WAV.

## Déplacement de toute la boucle

1. Avec la même zone, maintenir **Maj** et cliquer sur un bouton `+1`. DÉBUT et FIN doivent avancer ensemble d'**une frame**. Leur différence doit rester strictement identique.
2. Maintenir **Maj + Option** et cliquer sur `−1` : les deux marqueurs doivent reculer ensemble d'**au maximum dix frames**, sans changer la durée.
3. Répéter ces gestes sur les boutons ±1 associés à l'autre marqueur : ils doivent avoir exactement le même effet global.
4. Vérifier les limites du WAV : quand la boucle touche le début ou la fin du fichier, un déplacement vers l'extérieur ne doit ni déborder, ni modifier la longueur.
5. Comparer les positions avant/après en utilisant la préécoute A/B avec et sans fondu. Le réglage de fondu doit rester inchangé.
6. Cliquer sur **ANNULER** et confirmer le retour aux dernières valeurs appliquées ; refaire un mouvement, puis **APPLIQUER**, sauvegarder et rouvrir pour contrôler sa persistance.

## Contrôles de régression

- Les commandes ZÉRO et SAUT MIN des versions antérieures restent opérationnelles.
- Le geste Maj + Option sur ZÉRO DÉBUT/FIN doit continuer à lancer la recherche intelligente **PAIRE** v0.19.
- Aucun changement des WAV, des banques, du moteur audio temps réel ou des anciens projets.
- Les accents français é, è, ê, à, ç s'affichent normalement.
- Aucun chevauchement des boutons, menus, valeurs ou repères à la résolution d'un Mac 13 pouces.
- Le mode AUTO ZÉRO reste désactivé par défaut dans les anciens projets.

Effectuer ces vérifications dans l'application Standalone, puis si possible dans un DAW en AU et en VST3.
