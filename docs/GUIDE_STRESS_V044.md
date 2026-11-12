# Liberty ROMpler v0.44 — Qualification technique, sans promesse excessive

## Tests automatisés
La banque de stress originale se génère hors du dépôt avec
`python3 tools/generate_streaming_stress_v044.py --output /tmp/Liberty-Stress-v044`.
Elle contient 64 WAV stéréo de 24 secondes, répartis en quatre presets source de 16 zones et un preset multicouche. Chaque zone couvre une note MIDI 48 à 63. Le manifest respecte tous les champs requis par le validateur v1. L'outil refuse d'écraser un dossier existant.

Le banc C++ natif `LibertyStressV044` appelle le **vrai** `LibertyRomplerAudioProcessor::processBlock` à 44,1 kHz / blocs de 512 frames en cadence temps réel (sans interface audio ni DAC).
Scénarios :
- `poly64` : 16 notes × 4 couches = jusqu'à 64 voix simultanées. Le test dure 23 secondes pour franchir les boucles (fin de boucle à 80 % d'un WAV de 24 s).
- `switch` : alternance contrôlée des presets multicouche et monocouche ENTRE blocs audio. Ce n'est pas un test de changement concurrent UI/audio.
- `slice` : lecture des tranches MIDI 36-43 à différentes positions du fichier.
Les rapports JSON fournissent CPU de processus (workers compris), temps de rendu moyen/maximal et dépassements du budget audio, mémoire RSS maximale, sorties non finies, blocs de silence suspects et compteurs de sous-alimentation.

**Précision des mesures :** la CPU mesurée est celle du processus pendant le banc, et les temps de callback sont des durées murales de `processBlock`, pas une mesure de latence du DAC. Les « blocs sous-alimentés » proviennent de la télémétrie v0.43, pas de l'écoute directe. Une compilation et des tests unitaires verts ne démontrent pas une absence de clics en situation musicale.

## Capacité transitoire sûre lors des changements de presets

Le pool partagé conserve 4 workers, mais dispose désormais de 32 emplacements d'inscription par worker (128 au total). Il peut ainsi préparer un prochain preset de 64 zones alors qu'un précédent preset de 64 zones est encore présent. La limite de 64 zones actives **par preset** reste inchangée. Cette capacité est globale à l'ensemble des instances du module : plusieurs instances et la saturation du pool doivent toujours être testées. Ce correctif n'augmente pas la polyphonie nominale.

## Résistance aux disques lents
Le test C++17 indépendant `streaming_slow_pool_v044_test.cpp` simule 64 zones et des lectures de 2 ms par demande pour vérifier la priorité des pages urgentes face aux pages anticipées. **Il ne simule pas un disque physique dans le moteur JUCE.**
À vérifier manuellement sur macOS Apple Silicon avec SSD interne puis SSD externe/USB lent :
- Charger la banque complète, jouer 16 touches à la fois, maintenir 30 minutes, activer les boucles et crossfades, essayer SLICE.
- Relever CPU, RAM et incidents depuis RÉGLAGES > STREAMING DISQUE > COPIER LE RAPPORT.
- Changer les presets en cours de lecture, sauvegarder et rappeler une session AU puis VST3.
- Comparer l'audio, la mémoire et les valeurs de la v0.43, sans régression sur les petites banques.
- Inspecter visuellement 1120×750, 1360×850, 1920×1120, les accents français et tout chevauchement.

**Critère de qualification sonore** : zéro NaN, zéro crash, pas de bloc sous-alimenté ni de dépassement audio récurrent dans les conditions ciblées, absence de clics à l'écoute. En cas d'incident, conserver le JSON et ouvrir une correction moteur avant de déclarer le jalon de streaming professionnel terminé.

Cette v0.44 est un jalon de mesure/qualification, pas une augmentation artificielle de polyphonie ni une certification des performances sur toutes les machines.
