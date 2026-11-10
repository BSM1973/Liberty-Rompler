# Liberty ROMpler v0.20 — Déplacement manuel de la boucle entière

**Statut : en développement dans la branche `feature/v020-manual-paired-nudge`. La v0.19 reste la dernière version macOS validée.**

## Nouveauté dans SAMPLES > BOUCLES

Les quatre boutons existants `−1` et `+1` sont conservés, avec quatre modes :

- **Clic** : déplacer le marqueur DÉBUT ou FIN correspondant de **1 frame**.
- **Option + clic** : déplacer le marqueur correspondant de **10 frames**.
- **Maj + clic** : déplacer **DÉBUT et FIN ensemble** de **1 frame**, en conservant la durée de boucle.
- **Maj + Option + clic** : déplacer **DÉBUT et FIN ensemble** de **10 frames**, toujours sans modifier la durée.

Le déplacement couplé est disponible à partir de n'importe lequel des quatre boutons existants. Pour les nouveaux gestes, `−1` déplace la boucle vers le début du WAV et `+1` vers la fin.

## Sécurité et musicalité

- Le décalage commun est limité aux bornes du fichier : DÉBUT ne passe jamais sous 0 et FIN n'excède jamais la durée totale.
- **FIN − DÉBUT reste strictement identique**, même si le déplacement demandé est limité par une bordure du WAV.
- Le fondu de boucle garde sa valeur ; AUTO ZÉRO ne recale pas les marqueurs après un déplacement couplé, car cela casserait le verrouillage de durée.
- Le changement reste **non appliqué** jusqu'au clic sur APPLIQUER ; ANNULER restaure le dernier état enregistré dans le projet.
- Le calcul est purement arithmétique ; il n'a aucune incidence sur `processBlock()`, les samples WAV, les banques ou les presets.
- Un message bref **GLISSE ±1 f / ±10 f** indique le déplacement réellement effectué, ou **GLISSE : limite atteinte** si les marqueurs ne peuvent pas se déplacer.

Les fonctions des v0.15 à v0.19 restent disponibles : zoom, préécoute A/B, fondu anti-clic, recherche SAUT MIN et recherche PAIRE intelligente.

## Nos deux règles non négociables

1. **Aucun chevauchement de contrôles ou de textes**, notamment sur les écrans de 13 pouces : zéro nouveau bouton.
2. **Aucun caractère erroné à la place des lettres accentuées françaises** : caractères rendus par échappements Unicode dans les sources C++.

## Validation avant publication

`Source/LoopPairedNudgeV020.h` contient une fonction C++17 testable indépendamment de JUCE. `tests/loop_paired_nudge_v020_test.cpp` couvre les limites du WAV, les très petits déplacements, les valeurs entières extrêmes et le verrouillage de durée. `tests/test_v020_paired_nudge.py` contrôle le raccordement aux composants JUCE.

Après prévalidation Linux, un seul build macOS devra produire les formats AU/VST3/Standalone, le ZIP, le DMG et les sources. Les tests auditifs et visuels macOS sont décrits dans `docs/GUIDE_TEST_BOUCLES_V020.md`.
