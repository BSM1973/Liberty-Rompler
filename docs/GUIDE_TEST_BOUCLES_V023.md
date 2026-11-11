# Liberty ROMpler v0.23 — Guide de test macOS

À réaliser après le build macOS v0.23. Aucun fichier WAV ou preset ne doit être modifié.

## Zoom autour de la souris

1. Ouvrir le Standalone puis **SAMPLES > BOUCLES** et charger un sample WAV avec une boucle active.
2. Régler le zoom intermédiaire, noter DÉBUT, FIN, FONDU et observer un transitoire facilement reconnaissable.
3. Pointer le transitoire au quart gauche de la forme d'onde puis faire **Option + molette vers le haut**. La zone doit s'agrandir autour de ce point, et non se recentrer systématiquement au milieu.
4. Effectuer **Option + molette vers le bas** : revenir vers la vue précédente ; le point visé doit rester proche du curseur dans la mesure permise par les limites du WAV.
5. Répéter le test à droite et près des bords du fichier, ainsi qu'au zoom minimum et maximum. Aucun débordement, saut anormal ou blocage ne doit apparaître.
6. Faire de la **molette normale** sans Option : le zoom reste centré sur la fenêtre, comme dans les anciennes versions.
7. Faire un **défilement horizontal** : POSITION doit suivre. Vérifier le curseur ZOOM et la barre POSITION après un zoom autour de la souris.

## Vérifier les fonctions des versions antérieures

- Double-clic normal = afficher tout le WAV.
- Option + double-clic = cadrer la boucle, conservé depuis la v0.22.
- Maj + glisser dans la zone colorée = translater toute la boucle sans changer sa durée, depuis la v0.21.
- Boutons ±1/±10, modes PAIRE, SAUT MIN, ZÉRO et préécoute A/B.
- ANNULER/APPLIQUER, AUTO ZÉRO et limites des marqueurs doivent rester inchangés.
- Le zoom seul ne doit jamais activer NON APPLIQUÉ ni modifier DÉBUT/FIN/FONDU.

## Règles obligatoires Liberty

1. **Aucun chevauchement** des textes, boutons, menus ou repères sur écran 13 pouces.
2. **Aucun symbole parasite** à la place des accents français : é, è, ê, à, ç.

Tester dans le Standalone puis, si possible, en AU et VST3. Pour signaler un problème : fréquence d'échantillonnage, taille WAV, position des marqueurs, valeur du zoom et geste exact.
