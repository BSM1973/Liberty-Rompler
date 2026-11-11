# Liberty ROMpler v0.35 — Test des durées autour de la boucle

Sur macOS, après compilation, tester SAMPLES > BOUCLES avec un WAV à 48 kHz de 96000 frames (2 secondes).

1. Activer BOUCLE, DÉBUT 24000, FIN 48000 : infobulle « Boucle activée » : AVANT = 24000 frames / 500,000 ms ; APRÈS = 48000 frames / 1000,000 ms. Les anciens pourcentages doivent rester 25,00 % / 25,00 % / 50,00 %.
2. Boucle sur tout le WAV (DÉBUT 0, FIN 96000) : avant et après = 0 frame / 0,000 ms.
3. DÉBUT 48000, FIN 72000 : AVANT = 48000 frames / 1000,000 ms ; APRÈS = 24000 frames / 500,000 ms.
4. Répéter avec les WAV 44,1 / 88,2 / 96 / 192 kHz ; la conversion doit utiliser la fréquence du WAV, pas le réglage du DAW.
5. Vérifier changement de sample, boucle désactivée, édition des deux marqueurs et absence de pourcentage périmé.
6. Vérifier durées de boucle et de FONDU, pourcentage du FONDU, PART DU WAV, AVANT LA BOUCLE, APRÈS LA BOUCLE et infobulles des deux ZÉRO.
7. Vérifier que ZÉRO, SAUT MIN, PAIRE, Commande, Commande + Option, zoom, navigation, historique et audition A/B restent disponibles.
8. Vérifier les accents français DÉBUT, ZÉRO, DURÉE et APRÈS et qu'aucun élément ne se chevauche.
9. Vérifier les mêmes fonctionnalités en Standalone, AU et VST3 ; comparer les WAV, banques, presets et projets avant/après.

Les valeurs en millisecondes sont affichées à trois décimales, avec point décimal dans JUCE.
