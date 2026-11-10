# Liberty ROMpler v0.19 — Recherche PAIRE avec durée verrouillée

**Développement :** `feature/v019-paired-loop-length-lock`. La v0.18 reste la dernière version macOS compilée tant que le build v0.19 n'a pas réussi.

## Nouvelle commande dans SAMPLES > BOUCLES

Sur l'un ou l'autre des boutons existants **ZÉRO DÉBUT / ZÉRO FIN** :

- **Clic normal** : recherche du passage par zéro le plus proche, comme auparavant.
- **Option + clic** : recherche SAUT MIN en déplaçant uniquement le marqueur choisi, comme en v0.18.
- **Maj + Option + clic** : **PAIRE**, recherche du meilleur déplacement simultané de DÉBUT et FIN, dans une plage de ±512 frames, **sans changer la longueur de la boucle**.

Un seul décalage signé est appliqué aux deux marqueurs. Le moteur compare la différence d'amplitude brute entre la dernière frame avant FIN (exclusive) et la première à DÉBUT, dans les canaux mono ou stéréo. Le critère principal est le pire saut des canaux ; la continuité de pente ne départage que les candidats déjà améliorés.

## Contrôles de sécurité

- **Durée strictement inchangée** : FIN − DÉBUT avant/après est identique.
- **Bornes respectées** : DÉBUT reste supérieur ou égal à 0 et FIN n'excède jamais la durée du WAV.
- **Choix conservateur** : aucune proposition qui augmenterait le saut d'amplitude maximal.
- **Non destructif** : ni le WAV, ni la banque, ni le fichier d'état du DAW ne sont modifiés sans APPLIQUER.
- **ANNULER** permet de retrouver les dernières valeurs appliquées.
- **FONDU** garde son réglage, et AUTO ZÉRO n'est pas déclenché par la recherche PAIRE.
- **Mémoire bornée** : lecture de deux petites fenêtres WAV autour de DÉBUT et FIN, jamais du fichier entier.
- **Temps réel inchangé** : le calcul reste dans l'éditeur et ne s'exécute pas dans `processBlock()`.

La meilleure différence d'amplitude n'est pas toujours le meilleur résultat perceptif. Comparer la préécoute A/B avec ou sans fondu avant d'appliquer.

## Règles Liberty

L'interface conserve les boutons existants, sans chevauchement supplémentaire. Les accents français sont préservés par des sources C++ ASCII avec échappements Unicode, comme dans les versions antérieures.

## Validation

- `Source/LoopPairedSeamV019.h` : optimisation C++17 indépendante de JUCE, sans allocation ni écriture sur le signal.
- `tests/loop_paired_seam_v019_test.cpp` : durée identique, stéréo, bornes WAV, cas déjà optimaux et données invalides.
- `tests/test_v019_paired_loop.py` : compilation C++ et contrats de raccordement JUCE.
- `docs/GUIDE_TEST_BOUCLES_V019.md` : contrôle manuel après compilation macOS.

La publication se fera en **un seul build macOS** après validation Linux et vérification des deux règles incontournables.
