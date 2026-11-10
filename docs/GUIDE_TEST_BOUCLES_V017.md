# Liberty ROMpler v0.17 — Guide de contrôle macOS

Ce guide concerne la v0.17 **après compilation et installation**. Les versions précédentes et les banques WAV ne doivent pas être modifiées.

## Édition des marqueurs

1. Lancez le Standalone, sélectionnez un preset, puis ouvrez SAMPLES > BOUCLES.
2. Activez une boucle et notez les valeurs de début/fin.
3. Cliquez sur `-1` puis `+1` : chaque action change le marqueur d'une frame.
4. Répétez en maintenant **Option** : chaque action doit déplacer exactement 10 frames, sauf aux limites de la boucle.
5. Vérifiez ANNULER et APPLIQUER puis rouvrez la session pour confirmer la sauvegarde.
6. Vérifiez que le mode AUTO ZÉRO reste désactivé par défaut dans les anciens projets.

## Écoute courte et longue A/B

1. Réglez un fondu anti-clic sur une boucle longue et cliquez sur **PRÉÉCOUTE** : B avec fondu, contexte court (~60 ms de chaque côté).
2. Utilisez **Option + clic** : B avec fondu, contexte long (~250 ms de chaque côté).
3. Utilisez **Maj + clic** : A sans fondu, contexte court.
4. Utilisez **Maj + Option + clic** : A sans fondu, contexte long.
5. Dans le retour d'interface, les valeurs de durée et l'indication BRUT en dB doivent s'afficher sans couper les textes voisins.
6. À 192 kHz ou davantage, la fenêtre d'écoute peut être plafonnée à 32768 frames par côté : c'est volontaire pour limiter la mémoire.

## Gain et sécurité

1. Mettez le volume général du ROMpler à zéro et cliquez sur PRÉÉCOUTE : le raccord doit rester audible à un niveau de contrôle modéré.
2. Modifiez le preset et la valeur de fondu ; aucun plantage, blocage ou écriture dans les WAV ne doit survenir.
3. Vérifiez au moins un WAV mono, un stéréo et une session à fréquence d'échantillonnage différente.
4. Testez les formats Standalone, AU et VST3.

## Deux règles absolues

- Aucun chevauchement de boutons, marqueurs, zones colorées ou textes, y compris sur écran 13 pouces.
- Aucun symbole ni caractère incorrect à la place des lettres françaises : é, è, ê, à, ç.

Si un test échoue, relevez le format (Standalone, AU, VST3), la fréquence, le fichier WAV et la manipulation précise.
