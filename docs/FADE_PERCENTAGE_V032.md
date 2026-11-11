# Liberty ROMpler v0.32 — Pourcentage du fondu dans la boucle

**Version combinée : la v0.32 reprend également les positions DÉBUT/FIN en millisecondes prévues pour la v0.31, dont la prévalidation GitHub Linux avait été annulée. Un seul build Mac v0.32 est prévu après validation complète.**

## SAMPLES > BOUCLES

L'infobulle déjà existante sur **Boucle activée** indique désormais, en plus de la durée totale de la boucle et de la durée du FONDU en frames/ms, **le pourcentage du fondu par rapport à la longueur de boucle**.

Exemple : une boucle de 24000 frames avec un FONDU de 2400 frames correspond à **10,00 %**. Le pourcentage ne dépend pas de la fréquence d'échantillonnage, contrairement aux millisecondes. Il est défini par la formule :

`pourcentage = 100 × framesFondu / (FIN - DÉBUT)`.

Le début de boucle et la fin de boucle sont considérés comme des marqueurs avec **FIN exclusif**. La mesure suit les réglages et ne s'affiche pas si la boucle est inactive ou si la longueur ou le fondu sont invalides.

## Nouveauté v0.31 incluse

Les infobulles **ZÉRO DÉBUT** et **ZÉRO FIN** indiquent aussi la position exacte de chaque marqueur en frames et millisecondes **depuis le début du WAV**, avec la fréquence d'échantillonnage réelle du fichier.

Tous les gestes des boutons ZÉRO restent identiques : clic = zéro, Option + clic = SAUT MIN, Maj + Option + clic = PAIRE, Commande + clic = centrage, Commande + Option + clic = zoom de précision.

## Contraintes absolues

**Aucun chevauchement** : le pourcentage et les positions de marqueurs apparaissent exclusivement dans les infobulles existantes. Aucun bouton ou champ n'est ajouté à l'interface compacte.

**Accents français intacts** : aucune lettre accentuée ne doit être remplacée par des symboles. Les sources C++ utilisent des séquences Unicode protégées.

**Non destructif** : ni WAV, ni banque, ni preset, ni projet, ni moteur audio temps réel, ni marqueurs, ni FONDU ne sont modifiés par la mesure.

## Tests

- `Source/LoopFadeRatioV032.h` : calcul autonome C++17, sans JUCE ni allocation, avec intermédiaire entier 64 bits.
- `tests/loop_fade_ratio_v032_test.cpp` : 0 %, 10 %, 50 %, limite 100 %, FIN exclusif, boucle déplacée, fichiers très longs et états invalides.
- `tests/test_v032_fade_ratio.py` : compilation C++17, présence de la v0.31, aucune mutation ni contrôle supplémentaire, vérification des textes français.
- Prévalidation Linux complète JUCE 8 et VST3. **Une seule compilation macOS** avec AU, VST3, Standalone, ZIP, DMG et sources après succès.
- Recette manuelle : `docs/GUIDE_TEST_BOUCLES_V032.md`.

Les tests automatisés ne remplacent pas la vérification ergonomique et auditive sur un Mac réel.
