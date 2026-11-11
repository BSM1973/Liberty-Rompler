# Liberty ROMpler v0.30 — Durée de boucle en frames et millisecondes

**Statut : en développement dans `feature/v030-loop-duration-ms-readout`. La v0.29 demeure la dernière version Mac validée jusqu'au résultat de la v0.30.**

## Une mesure directement dans SAMPLES > BOUCLES

La ligne d'aide déjà présente **sous la forme d'onde** affiche désormais, quand une boucle valide est active :

**BOUCLE 24000 frames | 500.00 ms** (exemple pour un fichier WAV à 48 kHz).

Il ne s'agit pas d'une estimation : la durée en frames est exactement **FIN - DÉBUT**, et la durée en millisecondes est calculée à partir de la **fréquence d'échantillonnage du WAV chargé**. Les fréquences 44,1, 48, 88,2, 96 et 192 kHz sont donc correctement prises en compte.

Une **infobulle sur « Boucle activée »** fournit une précision supplémentaire : durée de la boucle en frames et millisecondes (trois décimales), **nombre de frames du FONDU et durée du FONDU en millisecondes**.

Les durées se mettent à jour lorsque les marqueurs ou la durée du fondu changent. Quand la boucle est désactivée ou qu'aucun WAV valide n'est sélectionné, la ligne d'aide historique réapparaît et l'infobulle indique que la mesure n'est pas disponible.

## Aucune modification destructive

Le calculateur `Source/LoopTimingV030.h` est autonome en C++17, sans JUCE ni allocation, et travaille uniquement sur les nombres de frames et la fréquence réelle du WAV.

La mesure est strictement **en lecture seule** : elle ne change **ni DÉBUT, ni FIN, ni FONDU**, ni les fichiers WAV, banques, presets, projets ou l'état du moteur audio. Aucun traitement supplémentaire dans `processBlock()`.

**Aucun bouton, panneau ou nouvelle ligne n'est créé.** La ligne d'information existante possède déjà une hauteur réservée de 14 pixels ; l'affichage y est ajusté automatiquement, sans chevauchement, y compris sur Mac 13 pouces. Les accents français restent protégés par les échappements Unicode C++.

## Fonctions conservées

Tous les gestes des v0.15 à v0.29 restent disponibles : recherche ZÉRO, SAUT MIN, PAIRE, optimisation par zoom, zoom de précision v0.29, glissement couplé, historique de vue, déplacement à la molette, préécoute A/B, commandes ANNULER/APPLIQUER.

## Tests

- `tests/loop_timing_v030_test.cpp` : durées en frames et ms, 44,1/48/88,2/96/192 kHz, extrémité FIN exclusive, fondu nul, WAV très longs et valeurs invalides.
- `tests/test_v030_loop_timing.py` : compilation C++17, intégration de la fréquence réelle, affichage dans les 14 pixels existants, absence de modification des marqueurs, accents français et compatibilité avec les gestes antérieurs.
- Prévalidation Linux JUCE 8 et VST3, puis **un seul build macOS AU, VST3, Standalone** après validation verte.
- Guide de vérification sur Mac : `docs/GUIDE_TEST_BOUCLES_V030.md`.

Les tests automatisés ne remplacent pas le contrôle visuel sur une vraie machine Mac.
