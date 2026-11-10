# Liberty ROMpler v0.15 — édition des boucles : composants développés

**Statut : implémentation des composants C++ terminée et tests locaux réussis ; intégration JUCE à réaliser. Aucun nouveau build macOS déclenché.**

## Fonctions préparées

1. **Marqueurs à la frame** — `Source/LoopFrameInputV015.h` gère la saisie numérique stricte, les pas ±1 et ±10 frames, les bornes de la boucle, l'annulation et le retour aux dernières valeurs appliquées. `AUTO ZÉRO` reste désactivé par défaut pour protéger les anciens projets.
2. **Préécoute du raccord** — `Source/LoopEditorV015.h` prépare le court extrait sans modifier les WAV. `Source/LoopPreviewVoiceV015.h` ajoute un lecteur de préécoute *one-shot* et un fondu d'entrée/sortie court. La méthode `process()` n'alloue aucune mémoire et conserve séparément les canaux stéréo. **`prepare()` doit être appelé uniquement lorsque le traitement audio est suspendu**, avec un buffer de préécoute immuable jusqu'à l'arrêt.
3. **Repères anti-clic** — `crossfadeBands()` retourne les zones fondu-début / fondu-fin, bornées à la fenêtre visible, adaptées à la position horizontale et au zoom.
4. **Géométrie sans chevauchement** — `Source/LoopToolbarLayoutV015.h` place les champs de frames, les boutons ±, PRÉÉCOUTE, APPLIQUER et ANNULER sur 1, 2, 3 ou 5 rangées selon l'espace. Il ne dessine jamais un contrôle hors des limites ni au-dessus d'un autre. Géométrie testée sur plusieurs tailles, y compris les fenêtres compactes.

## Tests exécutables

```sh
c++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -o /tmp/loop_v015_test tests/loop_editor_v015_test.cpp
/tmp/loop_v015_test
```

Le lanceur `tests/test_v015_loop_helpers.py` exécute ce test dans la suite Python préexistante de GitHub Actions. La matrice couvre les changements de frames, les limites, l'annulation, les buffers mono/stéréo, l'arrêt de préécoute et les rectangles d'interface. Des tests locaux supplémentaires ont été exécutés avec `-fsanitize=address,undefined`, sans erreur.

## Conditions obligatoires avant de distribuer v0.15

- Reconstruire les fichiers `PluginEditor.cpp` et `PluginProcessor.cpp` du projet à partir des correctifs GitHub existants.
- Raccorder les boutons et les champs de `SAMPLES > BOUCLES` aux classes ci-dessus ; relier PRÉÉCOUTE au flux audio de test sans interrompre le moteur du DAW.
- Vérifier à l'écoute que le raccord entendu reproduit la méthode de fondu réelle du moteur v0.14 ; l'algorithme de référence devra être harmonisé si nécessaire.
- Contrôler les tailles des fenêtres sur Mac : **aucun chevauchement**, surtout sur écran 13 pouces.
- Vérifier l'UTF-8 et les accents français : **aucun symbole ne remplace une lettre**.
- Contrôler la compatibilité de sauvegarde/restauration DAW, avec `AUTO ZÉRO` inactif dans les projets antérieurs.
- Lancer les tests sans compilation payante au préalable ; déclencher **un seul build macOS** lorsque l'intégration est prête.

**Attention :** Les composants et tests présents dans le dépôt ne signifient pas que les nouveaux boutons sont déjà disponibles dans le plugin téléchargé.
