# Liberty ROMpler v0.41 - Pool mutualisé de streaming

## Amélioration de la v0.40

La v0.40 a franchi un cap important : les WAV longs sont connectés aux vraies voix audio JUCE et son build macOS est vert. Mais elle lance encore un thread par zone longue ; 64 zones pourraient donc lancer 64 threads. La v0.41 consolide cette architecture sans modifier le comportement des samples courts.

- **Quatre workers de lecture disque mutualisés**, répartis équitablement entre un maximum de 64 zones longues : chaque décodeur appartient à un seul worker.
- Chaque worker traite une zone à la fois et alterne équitablement les demandes en attente ; aucune lecture concurrente d'un même décodeur.
- Les anciennes pages PCM stéréo et leur file atomique sont conservées. Les appels au callback audio restent \`tryReadFrame\` et \`requestAhead\`, sans nouveau mutex, allocation ou accès disque.
- L'inscription et la suppression des zones sont faites au chargement des banques (hors audio), sous verrou ; la suppression attend la fin d'une éventuelle lecture disque pour éviter un use-after-free.
- Une 65e zone longue ne peut pas être inscrite : l'échec est remonté au moteur et le sample est rejeté explicitement plutôt que de jouer silencieusement.
- Le prélèvement de 0,75 seconde de début, les fondus anti-underflow et la lecture RAM des petits samples ne changent pas.

## Tests et limites

- Tests C++17 réels sur 64 zones, quatre workers, équilibre, réinscription après retrait et arrêt sûr.
- Non-régressions v0.40 sur les WAV longs, v0.38 sur les petits WAV et les 11 onglets, MORPH, SLICE, GRANULAR.
- **Pas de garantie de 64 voix sur un disque lent** tant que le traitement de sous-alimentation, les tests audio longue durée et le rappel de projets n'ont pas été auditionnés dans AU/VST3/Standalone.
- Le chargement des presets et l'édition graphique des boucles sont encore limités par l'architecture existante, malgré la prise en charge audio des grands WAV.
- Le pool partage quatre threads au niveau du module logiciel ; il peut être ajusté après mesure de charge et profilage réel.

## Livraison

Ne déclencher qu'une compilation macOS après que tous les tests du lot sont verts. Ne pas annoncer streaming « professionnel » sur la boutique avant QA dans des DAW, incluant charge multivoix, disques plus lents, changement de preset en cours de lecture et boucles/granular.
