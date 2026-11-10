# Liberty ROMpler v0.15 — noyau d'édition des boucles

Statut : **composants C++ et tests ajoutés ; intégration dans l'interface et préécoute audio à effectuer avant publication d'une v0.15 utilisable**.

Ce lot prépare les trois fonctionnalités demandées :

1. **Précision à la frame** : `nudge()` déplace séparément le début et la fin de boucle d'une frame (ou d'un pas configurable), en garantissant les limites de l'échantillon et une durée minimale.
2. **Préécoute du raccord** : `renderSeamPreview()` fabrique hors thread audio un court extrait contenant la fin de boucle et le début ; version sans fondu ou avec superposition calculée, séparément sur les canaux. Il reste à intégrer un lecteur de préécoute indépendant du DAW.
3. **Repères du fondu** : `crossfadeBands()` fournit les deux zones graphiques, côté fin et côté début, toujours alignées sur le zoom/la position et coupées aux bords de la vue.

## Règles impératives

- Aucun chevauchement, y compris à 64× de zoom ou quand le marqueur sort de la zone visible.
- Aucun symbole erroné à la place des lettres françaises ; UTF-8 dans les sources et dans les libellés.
- Les bornes de boucle sont traitées dans le code comme `[début, fin)` (fin exclusive).
- Aucune création de buffer, lecture de WAV, ni allocation dynamique dans le callback audio.
- **AUTO ZÉRO** doit rester désactivé par défaut pour les projets existants.
- Préécoute sans modifier le WAV, les banques ou les réglages sauvegardés.
- Ce module n'est **pas encore raccordé** à `SAMPLES > BOUCLES` ; ne pas annoncer une fonctionnalité visible ou un build v0.15 tant que ce raccordement et les tests macOS ne sont pas verts.

## Tests autonomes

Sur macOS ou Linux, lancer :

```sh
c++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -o /tmp/loop_v015_test tests/loop_editor_v015_test.cpp
/tmp/loop_v015_test
```

Avant la compilation macOS finale : réintégrer ce module dans les sources JUCE reconstruites, vérifier avec les tests d'édition v0.14, tester en Standalone et dans un DAW, puis déclencher **un seul** build GitHub regroupé.
