# Liberty Rompler v0.11 — Éditeur SAMPLES

L'onglet **SAMPLES** comporte deux vues complémentaires :

**MAPPING** affiche les zones du preset réellement chargé dans les quatre couches, leur plage de notes sur un clavier horizontal et leurs étages de vélocité. Sélectionnez une zone dans la liste, faites glisser les bords de sa plage sur le schéma (ou réglez les curseurs `NOTE DE`, `NOTE A`, `VEL DE` et `VEL A`). Le repère doré indique la note fondamentale de l'échantillon, qui reste inchangée.

**BOUCLES** affiche le fichier WAV en forme d'onde, une région de boucle colorée, une case pour activer/désactiver la lecture en boucle, et les paramètres `DÉBUT (FRAME)` et `FIN (FRAME)`. Les points de boucle sont exclusifs en fin de région et doivent couvrir au moins 11 frames. L'éditeur tient compte de la limite actuelle du moteur à 20 secondes chargées par zone et ne prétend pas régler une portion au-delà de cette durée.

Cliquez sur **APPLIQUER** pour reconstruire les sons concernés à partir des nouvelles valeurs, ou sur **RESTAURER** pour revenir aux valeurs de la banque. L'application des changements recharge les zones et peut interrompre les notes en cours ; réaliser ces modifications pendant que le transport est arrêté.

Les modifications sont **non destructives** : aucun WAV ni `manifest.json` n'est modifié. Elles sont enregistrées uniquement dans le **projet DAW** via la propriété `sampleZoneEdits`. Elles sont restaurées avant le rechargement des échantillons ; les projets plus anciens restent compatibles. Si le fichier source ou la banque ne sont plus disponibles, les entrées concernées sont ignorées. La fonction n'exporte pas de banque multisample éditée.

Les plages de notes acceptées vont de 0 à 127, celles de vélocité de 1 à 127, et les points de boucle doivent appartenir aux frames réellement chargées. Les ajustements s'appliquent aux zones WAV référencées, même dans un preset multicouche. Les réglages de volume, panoramique et filtre **par couche** demeurent dans l'onglet COUCHES.

**Règles de vérification** : caractères français correctement affichés, aucun chevauchement d'interface, vérification de la sauvegarde/restauration dans un hôte AU et VST3, comparaison audible des notes/régions de vélocité et d'une boucle soutenue. Les tests statiques et le build macOS ne remplacent pas les contrôles visuels sur l'ordinateur de l'utilisateur.
