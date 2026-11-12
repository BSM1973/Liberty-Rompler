# Liberty ROMpler v0.42 - Stream Priority et diagnostics

## Innovations effectivement codées

- **Priorité aux pages indispensables à la lecture** : un tampon atomique de 16 demandes urgentes par zone autorise plusieurs voix ayant des positions différentes. Chaque worker recherche les demandes urgentes de toutes ses zones avant la lecture anticipée standard.
- **SLICE, boucles et seek** : le premier bloc d'une nouvelle note/SLICE soumet une demande urgente sur sa position. Lorsque la lecture arrive à la fin d'une boucle, la page du début est demandée en priorité pour le crossfade.
- **Moins de décodages inutiles** : avant d'ouvrir la zone disque, le worker vérifie si la page correspondante se trouve déjà dans le cache.
- **Diagnostic à coût faible** : compteurs atomiques pour cache hit/miss, pages en attente, drops, erreurs de lecture, demandes urgentes et blocs réellement sous-alimentés. Un snapshot C++17 peut être lu hors thread audio ; il n'est pas encore représenté par des vumètres dans l'interface.

## Stabilité et compatibilité

MORPH v0.36, Round Robin/articulations v0.37, GRANULAR/TIME/SLICE v0.38, stream long WAV v0.40 et pool de quatre workers v0.41 sont conservés. Les petits WAV restent traités en RAM. Aucun nouveau paramètre de banque, état DAW ou onglet. Les appels du callback audio ne créent pas de thread, n'ouvrent pas de fichier et ne verrouillent aucun mutex.

Le pool reste **limité à 64 zones longues** pour cette version, 16 par worker en moyenne, et le format de banques v1 à 32 zones par preset. Le streaming ne constitue pas encore une garantie de qualité sur un disque lent ou à 64 voix : il faut une campagne de test sous AU/VST3/Standalone avant une affirmation commerciale.

## Tests attendus

1. Prévalidation C++17 de la politique de priorité face à une file de centaines de lectures lentes et de la mesure des sous-alimentations.
2. Régression complète des v0.35-v0.41 (toutes les banques, les onze onglets, les caractères accentués et la géométrie).
3. Compilation macOS Apple Silicon AU/VST3/Standalone et intégrité des archives.
4. Tests musicaux sur WAV longs, boucles, quatre couches, 64 voix, disque lent et changement de preset.
5. Vérifier une session ancienne et une banque courte par rapport à v0.41 sans changement audible.

Pas d'annonce d'un statut « streaming professionnel » avant la validation du point 4. Un seul build Mac après prévalidation verte.
