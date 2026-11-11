# Liberty ROMpler v0.39 — Streaming disque : fondations A1

**Statut : prototype bas niveau compilé et testé, NON raccordé aux voix JUCE.** Les plugins restent en v0.38 ; la limite historique de chargement RAM de 20 secondes par zone demeure. Aucun nouveau format AU/VST3/Standalone ne sera publié pour un cache encore inactif. Ce document fait partie du jalon A de la Hybrid Workstation.

## Livrables vérifiables

- \`Source/StreamingPageCacheV039.h\` : cache préalloué de 16 pages stéréo PCM, 4096 frames par page, compteurs cache-hit/miss, index 64 bits, lecture non bloquante dans le thread temps réel.
- Protection des pages contre les lectures concurrentes pendant leur remplacement : annonce d'écriture, incrément/décrément de lecteurs et barrières atomiques séquentiellement cohérentes. L'écrivain attend les lecteurs **hors audio**.
- File de 64 demandes à accès sans mutex, multi-producteurs, sans pertes silencieuses : si pleine, compteur d'erreurs et retour immédiat, jamais attente dans l'audio callback.
- \`Source/StreamingReadAheadV039.h\` : worker de lecture anticipée sur thread séparé, demandes de page, prélecture de la suivante, démarrage/arrêt réservés au thread de gestion.
- \`Source/JuceStreamingReaderV039.h\` : adaptateur de décodage JUCE mono/stéréo conçu pour le worker ; son raccordement et sa compilation finale sous hôte restent à valider.

Tests C++17 : \`tests/streaming_page_cache_v039_test.cpp\`, \`tests/streaming_worker_v039_test.cpp\`, \`tests/streaming_mailbox_v039_test.cpp\`, lancés par \`tests/test_v039_streaming_foundation.py\`. Le cas de lecture simulée à **trois heures** valide les index 64 bits et la prélecture, **pas** la lecture d'un vrai WAV long dans Liberty. La race de remplacement de 12 000 pages vérifie que la gauche/droite ne sont pas déchirées. Essais locaux supplémentaires sur 20 passages de concurrence réussis.

Les sources ont été transférées en archive \`assets/v039/streaming-sources.tar.xz.b64\` et vérifiées SHA-256 (\`aff35f416c41c676c65c19a507ba9efe81f9f540d70114af30218e1f61e34a4b\`) avant extraction. Aucun fichier sample existant n'est remplacé. La prévalidation GitHub publie également les fichiers C++ séparés et éditables.

## Critères obligatoires avant le prochain build macOS

1. Un **seul service partagé** de lecture (pas de thread par voix/sample), allocations hors audio, préchargement des attaques, cache dimensionné pour au moins 64 voix/4 couches.
2. Adressage 64 bits jusqu'au renderer et aux outils de boucle ; requêtes de plusieurs voix, préparation en avance des zones de boucle et transposition/pitch bend.
3. Respect de la durée de vie des banques lorsque le preset est changé, lors de \`allNotesOff\` et à la fermeture du plugin.
4. Stratégie sans clic sur défaut de données, compteurs de sous-alimentation, tests sur stockage ralenti et vraie banque WAV longue.
5. Test avec MORPH, Round Robin, Keyswitch, GRANULAR, TIME, SLICE, HARMONY, effets, et tous les anciens projets.
6. Nouvelle prévalidation entière, **un seul build macOS par lot stable**.

**Risques connus :** avec 16 pages de 4096 frames par source, le prototype ne peut pas encore garantir 64 notes simultanées à des positions indépendantes ; les atomiques par frame ont un coût CPU à mesurer et optimiser par blocs. Il serait trompeur d'annoncer un streaming prêt pour la production avant ces étapes.

Règles Liberty : zéro chevauchement, accents français intacts, son historique inchangé, aucune lecture de fichier dans le callback audio.
