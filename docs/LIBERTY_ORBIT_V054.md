# Liberty ROMpler v0.54 — LIBERTY ORBIT

## Quatre expériences sonores
- **MANUEL** : ancien pad XY à quatre couches, inchangé pour les projets existants.
- **ORBIT** : mouvement circulaire continu entre A/B/C/D.
- **INFINITY** : trajet en huit avec traversée du centre.
- **CONSTELLATION** : huit points de passage choisis par un générateur déterministe. GRAINE 0 à 15 donne 16 voyages reproductibles, sans dépendance à un réseau ou à l'IA.

## GRAVITÉ MIDI
La hauteur des notes et leur vélocité influencent temporairement la trajectoire du XY. La dernière note jouée exerce l'attraction ; relâcher une note met à jour les notes encore tenues. Le MIDI CC16/17 peut également déplacer le centre du parcours.

## Contrôles sur la page MORPH
MOUVEMENT, VITESSE (0,02–2 Hz), AMPLITUDE, GRAINE et GRAVITÉ MIDI, avec commandes bleu nuit et textes lisibles. Sélectionner un mouvement active MORPH ; l'option MANUEL et la commande MORPH OFF restent disponibles. Les paramètres sont sauvegardés dans les projets et exposés à l'automation AU/VST3 avec cinq nouveaux identifiants stables.

## Compatibilité et sûreté audio
ORBIT est désactivé par défaut. Les projets sans propriétés ORBIT restent en MANUEL, à leur position XY et avec les mêmes quatre poids. Le moteur fonctionne avec mémoire fixe et sans lecture disque, mutex ou allocation dans processBlock. Les gains MORPH conservent leur lissage 45 ms. Il n'y a aucun changement du cache de streaming 32 pages, des boucles SLICE, des onze onglets et des banques sonores.

## Qualification
Tests du calcul C++17 (mode manuel strict, positions bornées, seed déterministe, Notes On/Off, vélocité, poids égaux à un), tests Python de l'intégration des paramètres et de l'interface, prévalidation Linux et compilation native macOS AU/VST3/Standalone. Les écoutes en DAW et avec la Studio 1824C doivent toujours être effectuées sur un véritable Mac.

## Innovations futures proposées (non livrées en v0.54)
SPECTRAL DNA : morph spectral évolutif ; TIME BLOOM : épanouissement temporel d'une texture issue d'une note ; SCENE MEMORY : scènes instantanées avec transitions synchronisées ; HARMONIC TERRAIN : contrôle des harmoniques par la progression d'accords.
