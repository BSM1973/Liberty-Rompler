# Liberty ROMpler v0.16 — Recette de validation macOS

Cette recette ne concerne que la v0.16 **après son build macOS**. La v0.15 existante reste utilisable indépendamment.

## Vérification de la préécoute A/B

1. Chargez un son tenu, ouvrez **SAMPLES > BOUCLES** et choisissez une zone WAV dont les marqueurs début et fin sont valides.
2. Réglez un fondu de 256 frames. Cliquez sur **PRÉÉCOUTE** : le mode **B (avec fondu)** doit se faire entendre.
3. Maintenez **Maj** et cliquez sur le même bouton : le mode **A (sans fondu)** doit jouer exactement les mêmes points de boucle, sans toucher au réglage du fondu.
4. Comparez les deux à l'oreille et vérifiez qu'il n'y a ni décrochage ni note MIDI déclenchée.
5. Après sauvegarde et réouverture du projet, le fondu paramétré doit être inchangé.

## Écart brut en dB

Le message de retour doit indiquer **A | BRUT ... dB** ou **B | BRUT ... dB**. La mesure représente la différence d'amplitude entre le dernier échantillon de la boucle et le premier : elle peut être identique pour A et B, puisque ce n'est pas une mesure après application du fondu. Elle ne suffit **pas** pour conclure à la présence ou à l'absence d'un clic audible.

## Gros fichiers WAV

1. Testez un WAV court et un WAV très long (plusieurs minutes), de préférence à 44,1 kHz et 48 kHz.
2. Lancez plusieurs préécoutes, sans augmenter exagérément la mémoire de l'application.
3. Changez de preset pendant ou après une préécoute. Le DAW ne doit ni bloquer ni planter.
4. Vérifiez les formats Standalone, AU et VST3 si possible.
5. Ne faites pas ces tests en enregistrant une prise importante, la préécoute est encore expérimentale.

## Règles absolues d'interface

- Aucun chevauchement de texte, bouton, menu ou marqueur, notamment avec un écran de 13 pouces.
- Tous les caractères français (é, è, ê, à, ç) doivent être corrects.
- AUTO ZÉRO reste désactivé par défaut dans les anciens projets.
- Les fichiers WAV et les banques d'extensions ne doivent pas être modifiés par la préécoute.

En cas de problème, noter le WAV, sa durée, sa fréquence, le format du plugin, la position des marqueurs, la valeur du fondu et le résultat des deux modes A et B.
