# Liberty v0.40 — Streaming WAV longs : connexion audio expérimentale

## Objectif et statut

Premier raccordement réel du lecteur anticipé v0.39 aux voix audio JUCE. Les WAV jusqu'à vingt secondes doivent conserver le chemin RAM historique. Les WAV plus longs (mais <INT_MAX-2 frames) doivent disposer d'une mémoire d'attaque de 0,75 s, puis de lecture dans le cache PCM. **Le build macOS et les tests musicaux sont nécessaires avant livraison.**

## Audio temps réel

- Le décodage WAV et les lectures fichier sont effectués uniquement dans \`ReadAheadWorker\`, hors \`SampleVoice::renderNextBlock\`.
- \`MappedSampleSound\` conserve le lecteur via \`StreamedZoneV040\`, qui arrête et joint son worker avant destruction du décodeur et du cache.
- Les pages sont stéréo, préallouées ; l'audio ne fait qu'appeler \`tryReadFrame\` / \`requestAhead\`. La queue est bornée.
- Les demandes anticipent plusieurs pages au démarrage, en fin de bloc et aux têtes de boucle. Les demandes manquées ne bloquent pas.
- En cas de cache en retard, une enveloppe de gain sur 64 échantillons amortit l'interruption. Ce mécanisme ne garantit pas l'absence de trou si le disque ne fournit plus les données.
- SLICE utilise la longueur totale de la source, et GRANULAR demande deux positions de lecture ; leurs résultats sur de longs samples restent à auditionner.

## Limitations *actuelles*, sans promesse excessive

- Un worker par **zone** longue : architecture pilote, à mutualiser en pool pour les banques très importantes.
- Maximum de 64 zones dans les quatre couches et 32 zones par preset, hérité des banques v1.
- Au plus \`INT_MAX-2\` frames par source longue ; refus explicite plutôt que troncation silencieuse.
- L'affichage et l'édition graphique des formes d'onde et boucles utilisent encore les anciens chemins bornés à 20 s. Le validateur et Smart Import du paquet v0.40 acceptent les WAV longs jusqu'à la limite int32 du moteur.
- Le préchargement de l'attaque est synchrone sur le thread de chargement de preset ; à déporter plus tard dans une tâche asynchrone.
- Pas de métrologie GUI des défauts de cache, ni d'assurance 64 voix à la fois en situation de disque lent.
- **Ne pas commercialiser ou promettre le streaming professionnel sans validation en DAW.**

## Tests exigés

1. Comparaison null test court WAV RAM v0.38/v0.40 ; volume, filtres, boucles, artefacts et automation.
2. WAV stereo mono de 5 min + 4 couches et jusqu'à 64 voix, pitch bend, transposition, keyswitch, RR et effet MORPH.
3. SLICE et GRANULAR sur très longs WAV, balayage des morceaux avec boucles non alignées sur les pages.
4. Reload preset pendant que des voix jouent : chaque worker s'arrête et aucune ressource n'est libérée dans le callback.
5. Test de disque bridé et mesure du taux de cache miss/underrun sans boucle d'attente dans l'audio.
6. Retour d'anciens projets et tests d'interface 11 onglets ; accents et zéro chevauchement.

Une étape ultérieure devra introduire **un pool de lecteurs partagé** et **un chargement des presets hors thread UI** avant d'augmenter les limites de banques.
