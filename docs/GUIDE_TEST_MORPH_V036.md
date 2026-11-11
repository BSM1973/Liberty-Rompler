# Liberty ROMpler v0.36 — Recette macOS Liberty MORPH

Tester d'abord la version Standalone, puis AU (Logic) et VST3 (Ableton/Bitwig ou autre hôte compatible).

1. Vérifier le dixième onglet MORPH, le pad XY et les boutons MORPH ON/OFF, MIDI CC16/17 ON/OFF. Redimensionner à 1120 × 750 puis 1360 × 850 : zéro chevauchement ; le clavier et les onglets doivent rester lisibles.
2. Charger un preset à quatre couches. Activer MORPH. Déplacer le point : coins A, B, C, D ; au centre, quatre poids proches de 25 %. Vérifier les sons et les transitions en notes maintenues.
3. En MORPH OFF, vérifier que le mixage historique des couches revient à l'identique après le lissage de 45 ms. Les projets anciens doivent démarrer avec MORPH OFF.
4. Pendant des notes tenues, déplacer rapidement le curseur, activer/désactiver MORPH et changer de coin. Vérifier l'absence de clic abrupt et de blocage audio.
5. Automatiser dans le DAW les paramètres Liberty MORPH Active, Liberty MORPH X, Liberty MORPH Y et Liberty MORPH MIDI CC16/17 ; enregistrer une automation et la rejouer. Vérifier le suivi du point dans l'interface.
6. Activer MIDI CC16/17 ; envoyer CC16 et CC17 depuis un contrôleur, valeurs 0, 64, 127. Vérifier le mouvement du point et la transition audio. Le MIDI n'écrase pas les paramètres d'automation X/Y ; il commande le son uniquement lorsque la source MIDI est active.
7. En mode MIDI, cliquer et glisser sur le pad : le mode MIDI se désactive et l'édition à la souris reprend. Vérifier les gestes de notification au DAW.
8. Enregistrer un projet, fermer l'hôte, réouvrir. Vérifier position XY, activation, source MIDI et coordonnées MIDI rappelées. Contrôler un ancien projet sans données MORPH.
9. Vérifier la présence des effets indépendants par couche, du routage, de l'arpégiateur, des neuf anciens onglets, de la page SAMPLES, des banques et des presets. Les fichiers WAV et manifestes ne doivent pas changer.
10. Contrôler les accents français partout ; jamais de symboles de remplacement, et aucun chevauchement.

Les poids de mélange de ce premier MORPH sont bilinéaires et totalisent 100 % quand MORPH est actif, sans compensation de loudness avancée. Une couche absente reste silencieuse à son angle.
