# Liberty ROMpler v0.17 — Préécoute courte et longue, édition précise

**Statut : lot en développement sur la branche `feature/v017-loop-audition-precision`. La v0.16 reste la dernière version distribuée.**

## Édition des marqueurs

Les boutons existants affichent toujours `-1` et `+1` pour limiter l'encombrement.

- **Clic ordinaire :** déplacement du marqueur de 1 frame.
- **Option + clic :** déplacement du même marqueur de 10 frames, sans ajouter de contrôle.
- Les valeurs saisies manuellement restent disponibles et sont limitées par les bornes du sample.
- AUTO ZÉRO reste désactivé par défaut pour les anciennes sessions.

## Préécoute A/B avec durée ajustable

La préécoute conserve les raccourcis de la v0.16 :

- **Clic :** B, avec le fondu paramétré.
- **Maj + clic :** A, sans fondu, sans changer le réglage du projet.
- **Option + clic :** préécoute longue, jusqu'à 250 ms de contexte de part et d'autre du raccord, au lieu de 60 ms par défaut.
- **Maj + Option + clic :** A sans fondu et contexte long.

La durée exacte est limitée par la longueur de la boucle et par un maximum de **32 768 frames par côté** ; elle peut donc être réduite automatiquement à très haute fréquence d'échantillonnage.

## Sortie de la préécoute

L'écoute de diagnostic est mélangée **après** le gain principal de l'instrument. Le son de préécoute reste audible même si le volume général du ROMpler est à zéro. La sortie utilise toujours le gain modéré de préécoute (0,32), un buffer pré-calculé et une synchronisation non bloquante.

Les WAV, les banques et les presets ne sont pas réécrits ; le `processBlock()` n'effectue ni décodage de fichier ni allocation de buffer.

## Deux règles Liberty

1. Aucun chevauchement : les raccourcis sont ajoutés aux boutons existants et les valeurs de durée figurent dans le message de retour et les infobulles.
2. Aucun caractère français incorrect : les fichiers C++ restent ASCII avec échappements Unicode pour les libellés accentués.

## Contrôles et publication

- `tests/loop_audition_v017_test.cpp` : conversions à 44,1, 48 et 384 kHz ; limites mémoire, boucles courtes, valeurs invalides.
- `tests/test_v017_audition.py` : compilation C++ et vérification des branchements UI/DSP.
- `scripts/apply_v017_audition.sh` : raccordement aux fichiers JUCE une fois les correctifs des v0.15 et v0.16 appliqués.
- Prévalidation Linux avec compilation du VST3 avant un seul build de livraison macOS.
- Validation manuelle du son et du rendu sur macOS après compilation.

**Ne pas considérer la v0.17 comme publiée avant un build macOS vert.**
