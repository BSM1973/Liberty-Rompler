# Liberty Rompler v0.13 - Recalage de boucle sur passage par zero

## Fonctions

Dans SAMPLES > BOUCLES, deux boutons `ZERO DEBUT` et `ZERO FIN` (affiches avec accents dans l'interface) calent individuellement les marqueurs sur le passage par zero le plus proche, en recherchant dans une plage de 512 frames de chaque cote. L'algorithme analyse le WAV source en stereo ou mono, privilegie la distance au marqueur et prend en compte l'energie des deux canaux. En absence de passage dans la plage valide, il ne deplace rien.

Un bouton `ANNULER` restaure les derniers reglages appliques de la zone selectionnee. Il n'annule pas les reglages precedemment sauvegardes dans la session DAW. `RESTAURER` garde sa fonction precedente : suppression des edits de la zone pour revenir aux valeurs de la banque.

La boucle ne peut pas franchir les limites de la zone audio et doit durer au minimum 11 frames. Les deux marqueurs restent edites de maniere independante, avec le crossfade de la version 0.12 toujours disponible. Les operations d'analyse ne s'executent jamais sur le thread audio.

## Compatibilite

Aucune modification du format de session, des banques, ni du moteur de lecture audio. L'alignement se materialise dans les valeurs start/end existantes au moment d'appuyer sur APPLIQUER, sans ecriture sur les fichiers WAV. Les anciens projets DAW restent lisibles.

## Limites

Ce recalage reduit les risques de clic de raccord mais ne garantit pas l'absence de clic sur toutes les formes d'onde. La qualite doit etre evaluee a l'oreille sur les sons charges. Un passage par zero sur un canal stereo peut ne pas correspondre exactement a un passage par zero sur l'autre. Pour une boucle complexe, utiliser aussi le fondu anti-clic.

## Controles

- Test C++ natif des passages par zero mono et stereo, des limites et des signaux corrompus
- Tests de boutons, lecture WAV, absence d'ecriture destructive et geometrie d'interface
- Tests conserves des anciennes versions, validation des banques et audit UTF-8
