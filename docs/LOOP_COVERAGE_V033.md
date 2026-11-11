# Liberty ROMpler v0.33 — Part du WAV occupée par la boucle

**Statut : en développement sur `feature/v033-loop-wav-coverage`. La v0.32 est la dernière compilation macOS confirmée jusqu'au build v0.33 vert.**

## Nouvelle mesure dans SAMPLES > BOUCLES

L'infobulle existante **Boucle activée** indique une mesure supplémentaire : **PART DU WAV**, le pourcentage de la durée totale du fichier audio réellement utilisée par la boucle.

Par exemple, avec un WAV de 96000 frames et une boucle allant de DÉBUT = 24000 à FIN = 48000, la boucle contient 24000 frames et couvre **25,00 % du WAV**.

Formule : `couverture = 100 × (FIN - DÉBUT) / framesTotalesDuWAV`.

Le marqueur FIN est **exclusif**. Une boucle du début jusqu'à la fin du WAV couvre 100,00 %. Une boucle déplacée de même longueur conserve le même pourcentage. La mesure reste indépendante de la fréquence d'échantillonnage et fonctionne pour les très longs WAV.

La valeur est actualisée en même temps que les autres paramètres de boucle. Aucune mesure trompeuse n'est affichée en l'absence de sample ou de boucle valide.

## Informations des versions précédentes conservées

L'infobulle existante **Boucle activée** affiche toujours :
- Durée de la boucle en frames et ms (v0.30).
- Durée du FONDU en frames et ms (v0.30).
- Pourcentage du FONDU dans la boucle (v0.32).
- **Part de la boucle dans le WAV en pourcentage (v0.33).**

Les infobulles des boutons ZÉRO DÉBUT et ZÉRO FIN montrent toujours les positions absolues en frames et millisecondes (v0.31). Tous les gestes ZÉRO, SAUT MIN, PAIRE, centrage, zoom de précision, navigation et préécoute A/B restent inchangés.

## Contraintes de développement

**Aucun chevauchement** : la nouvelle information est ajoutée à une infobulle déjà existante. Aucun bouton, ligne ou panneau supplémentaire.

**Français correct** : accents é, è, ê, à, ç et É intacts, jamais de symboles de substitution.

**Strictement en lecture seule** : la mesure ne change ni DÉBUT, ni FIN, ni FONDU, ni le WAV, les banques, les presets, les projets, ou l'audio temps réel.

L'algorithme C++17 `Source/LoopCoverageV033.h` est indépendant de JUCE, sans allocation et calcule la longueur en entier 64 bits.

## Validation et distribution

- `tests/loop_coverage_v033_test.cpp` : limites WAV, calcul de 25/50/100 %, boucle déplacée, grands entiers et données invalides.
- `tests/test_v033_loop_coverage.py` : compilation C++17, intégration dans les infobulles, non-destruction, conservation des fonctions v0.23–v0.32 et des caractères français.
- Prévalidation GitHub Linux comprenant la suite complète des tests et une tentative de compilation JUCE VST3 (un problème d'accès DNS aux dépôts Ubuntu peut empêcher la compilation Linux, indépendamment du code).
- **Un seul build macOS** AU/VST3/Standalone, puis vérification réelle ; les archives doivent comprendre ZIP, DMG, source et guide `docs/GUIDE_TEST_BOUCLES_V033.md`.

Les tests automatisés ne remplacent pas l'examen visuel sur Mac.
