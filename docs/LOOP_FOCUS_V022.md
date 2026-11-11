# Liberty ROMpler v0.22 — Cadrage automatique de la boucle

**Statut : en développement sur `feature/v022-focus-loop-viewport`. La v0.21 est la dernière distribution macOS validée tant que la v0.22 n'a pas de build macOS vert.**

## Nouvelle commande d'affichage

Dans l'onglet **SAMPLES > BOUCLES**, sur la forme d'onde :

- **Double-clic normal** : afficher tout le fichier WAV, comme dans les versions précédentes.
- **Option + double-clic** : centrer et zoomer pour voir la boucle active au complet, en conservant environ 12,5 % de la longueur de boucle comme marge de chaque côté, quand les limites du WAV et le zoom maximal le permettent.
- Si aucune boucle valide n'est active, le double-clic avec Option conserve le comportement normal d'affichage complet.

Le curseur de zoom et la barre de position suivent le cadrage, sans conflit. Aucun nouveau bouton, texte permanent ni panneau n'est ajouté.

## Sécurité et compatibilité

Le calcul du cadrage est purement numérique et s'effectue sur le fil de l'interface ; il ne lit ni n'écrit aucun échantillon WAV et ne modifie ni les marqueurs DÉBUT/FIN, ni le FONDU, ni les banques, ni les presets. La valeur de zoom est limitée à la plage de zoom déjà autorisée par l'application, avec vérifications des boucles courtes et des longs fichiers.

Le glissement de boucle de la v0.21, les déplacements par 1/10 frames, les commandes ZÉRO, SAUT MIN et PAIRE, ainsi que la préécoute A/B restent disponibles.

## Deux règles Liberty

- **Aucun chevauchement** : réutilisation du geste de double-clic existant, sans composant supplémentaire.
- **Aucun caractère parasite à la place d'une lettre française** : la source C++ emploie les échappements Unicode préservés par les audits.

## Vérifications

`Source/LoopFocusV022.h` contient le plan de cadrage (C++17 sans JUCE). `tests/loop_focus_v022_test.cpp` couvre les marges, les limites, les boucles courtes et les valeurs extrêmes. `tests/test_v022_loop_focus.py` valide l'intégration de la souris sans effet sur le moteur audio.

Prévalidation Linux complète avant un seul build macOS AU/VST3/Standalone, puis test visuel réel suivant `docs/GUIDE_TEST_BOUCLES_V022.md`.
