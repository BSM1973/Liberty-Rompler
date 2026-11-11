# Liberty ROMpler v0.28 — Accès direct aux marqueurs de boucle

**Statut : développement sur `feature/v028-jump-to-loop-markers`. La v0.27 reste la dernière compilation macOS confirmée tant que la v0.28 n'a pas de build vert.**

## Nouvelle commande dans SAMPLES > BOUCLES

- **Commande (⌘) + clic sur ZÉRO DÉBUT** : centrer le marqueur DÉBUT sur la forme d'onde, au zoom actuel.
- **Commande (⌘) + clic sur ZÉRO FIN** : centrer le marqueur FIN sur la forme d'onde, au zoom actuel.
- Si le marqueur se trouve près du début ou de la fin du WAV, la vue s'arrête aux limites du fichier. La frame de FIN peut correspondre à la fin exclusive du WAV.
- Le nouveau cadrage est ajouté à l'historique de la **v0.27** : **Maj + double-clic** permet de revenir au cadrage précédent.
- Aucune opération si la boucle n'est pas activée, si la zone WAV n'est pas chargée ou si les marqueurs sont invalides.

## Tous les gestes des boutons ZÉRO sont préservés

| Geste sur ZÉRO DÉBUT ou ZÉRO FIN | Résultat |
|---|---|
| Clic normal | Recherche du zéro de passage pour un seul marqueur |
| Option + clic | SAUT MIN intelligent pour un seul marqueur |
| Maj + Option + clic | Optimisation PAIRE verrouillant la durée |
| **Commande + clic** | **Centrer le marqueur concerné sans l'éditer** |

En cas de combinaisons supplémentaires, les gestes historiques avec Option conservent la priorité.

Le double-clic normal, Option + double-clic, Commande + double-clic, Maj + double-clic, les zooms à la molette, les glissements de vue ou de boucle, la préécoute A/B et ANNULER / APPLIQUER restent disponibles.

## Intégrité et interface

Le calcul du cadrage est implémenté en C++17 indépendant de JUCE dans `Source/LoopMarkerFocusV028.h`. Il maintient la durée et le niveau de zoom, n'utilise aucune allocation, et protège les fichiers très longs contre les dépassements arithmétiques.

**Aucune modification** des marqueurs DÉBUT/FIN, du FONDU, des WAV, banques, presets, projets ou callback audio. Aucun nouveau bouton, aucun panneau et aucun déplacement de composant.

Nos deux règles Liberty sont respectées : **aucun chevauchement** des éléments graphiques, notamment sur écran 13 pouces, et **aucun caractère de remplacement des accents français**.

## Contrôles

- `tests/loop_marker_focus_v028_test.cpp` : marqueurs aux extrémités, longueur exclusive, zoom conservé, fichier maximum et données invalides.
- `tests/test_v028_marker_focus.py` : compilation C++17, branchements JUCE, priorité des anciens modificateurs et absence d'écriture.
- Prévalidation JUCE 8 Linux et VST3 avant une seule compilation macOS AU/VST3/Standalone.
- Guide de vérification réelle : `docs/GUIDE_TEST_BOUCLES_V028.md`.
