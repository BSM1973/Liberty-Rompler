# Liberty ROMpler v0.50 — Fiabilisation poly64

## Problème reproduit en v0.49
Le build macOS n° 43582104103 a signalé 39 blocs de sous-alimentation à 64 voix, contre 7 dans le banc v0.48. En revanche SLICE était passé de 1 926 à 1 seul incident. Ces chiffres sont propres aux runners macOS et ne représentent pas une mesure sur matériel audio USB.

## Correctif ciblé
- WAV longs non prioritaires SLICE : allongement contrôlé de la portion d'attaque préparée en RAM de 0,75 s à 1,5 s, plafonné à 147 456 frames. Les zones pilotes SLICE conservent leur calcul v0.49 et leurs huit fenêtres.
- WAV longs bouclés : création hors temps réel d'une fenêtre de tête de boucle stéréo d'au plus 49 152 frames, 0,75 s à 44,1 kHz, au-delà de l'attaque initiale. Les premières positions de la boucle (y compris crossfade) sont lues directement dans cette fenêtre, sans seek simultané de 64 décoders.
- Les pages qui suivent cette fenêtre sont demandées à l'avance par le cache existant, à proximité de la fin de la fenêtre. Si la préparation d'une fenêtre échoue, l'ancien streaming demeure utilisable.
- Aucune modification des petites banques WAV, des anciens presets, des paramètres sauvegardés, des onze onglets ou du système de voix du synthétiseur. Les appels reader.read, setSize et new sont réservés au chargement, jamais au callback.
- Le banc natif inclut un relevé temporel des sous-alimentations au pas approximatif de 186 ms, hors callback. Il reste impossible de garantir zéro clic sans tests d'écoute en DAW.

## Coûts et validation
À 44,1 kHz, la fenêtre de boucle ajoute environ 16,9 Mo pour 64 zones stéréo (maximum 24 Mio toutes fréquences confondues). L'attaque supplémentaire des zones non-SLICE ajoute environ 16 Mo, selon nombre de zones et fréquence. La RAM de la v0.50 doit donc être comparée à celle de la v0.49.

Le test natif macOS doit vérifier : 64 zones, boucles (23 s), SLICE, changements de presets, CPU/RSS, dépassements du budget temps réel et sous-alimentations. L'objectif est zéro sous-alimentation aux conditions ciblées, et **aucune régression sur SLICE**. Ne jamais déclarer la fiabilité acquise uniquement parce que CI/packaging est vert. Tester également SSD interne et vrai support externe lent.
