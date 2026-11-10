# Liberty ROMpler v0.15 — Édition des boucles et préécoute

**Statut : raccordement JUCE et traitement de la préécoute réalisés dans `overlays/v015`. La prévalidation GitHub Linux est verte : reconstruction, tests, audit des accents et compilation VST3 JUCE 8.0.10. Le build macOS et les essais auditifs restent à confirmer.**

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

## Raccordement JUCE intégré

Les composants JUCE suivants sont désormais disponibles dans `Source/` :

- `LoopEditorPanelV015.h` : barre d'édition autonome avec champs DÉBUT/FIN en frames, boutons ±, PRÉÉCOUTE, APPLIQUER, ANNULER et disposition réactive. `setLoop()` charge la zone sélectionnée et `getPendingLoop()` lit les valeurs non encore appliquées.
- `LoopFadeOverlayV015.h` : calque non interactif aligné sur la forme d'onde. `setState(loop, fadeFrames, viewport)` redessine les deux bandes de fondu ; `setInterceptsMouseClicks(false, false)` laisse les marqueurs existants manipulables.
- `tests/test_v015_ui_contract.py` : vérifie que le panneau, les callbacks et le calque sont présents et que les sources sont bien en UTF-8.

### Branchements réalisés dans les overlays (contrôle technique)

1. Après assemblage des correctifs jusqu'à la v0.14, inclure les deux en-têtes JUCE dans le fichier source de l'éditeur, puis créer le panneau dans l'onglet SAMPLES > BOUCLES.
2. À chaque changement de zone WAV, appeler `setLoop()` avec les positions exactes en frames et `setState()` avec la même vue zoomée. Les coordonnées du calque doivent correspondre aux limites locales de la forme d'onde.
3. Relier `onPendingChanged` à la mise à jour non destructive des marqueurs et des bandes. Relier `onApplyRequested` à la sauvegarde/au rafraîchissement audio déjà utilisés par la v0.14, en retournant `true` uniquement lorsque l'application a réellement réussi. ANNULER doit retrouver les dernières valeurs appliquées.
4. Relier `onPreviewRequested` à l'accès au WAV déjà décodé, préparer le raccord hors thread audio et faire transiter le déclenchement par une commande sûre vers `processBlock()`. Ne pas lire les fichiers, allouer ni échanger de pointeurs non protégés dans le callback audio. Traiter les différences de fréquence d'échantillonnage entre le WAV et le DAW.
5. Monter le panneau avec l'espace réellement disponible et le masquer proprement si son `makeLoopToolbarLayout()` ne peut pas le contenir ; conserver un accès via défilement ou agrandissement de l'onglet.

**Intégration effective :** `overlays/v015/SampleZoneView.h` branche les boutons ±1 et PRÉÉCOUTE aux fonctions du processeur ; `overlays/v015/PluginProcessor.h/.cpp` prépare le WAV hors du callback et joue le buffer par accès non bloquant. `scripts/apply_v015_overlays.sh` remplace les trois fichiers après assemblage de la v0.14. `PluginEditor.cpp` conserve l'onglet SAMPLES et reçoit uniquement la nouvelle indication de version. La compilation Linux a validé le raccordement ; le rendu et l'écoute sur macOS restent à contrôler.

### Vérifications avant le build macOS unique

- La compilation des tests C++ autonomes et le test de contrat UI doivent réussir.
- La compilation réelle des deux nouveaux en-têtes avec JUCE 8 doit réussir après raccordement.
- L'éditeur de boucles ne doit masquer aucun contrôle et ne doit afficher aucun caractère incorrect en français.
- La préécoute doit s'arrêter sans clic perceptible et fonctionner en mono/stéréo, au bon tempo/fréquence d'échantillonnage.
- Les anciennes sessions DAW doivent charger sans changer le comportement par défaut d'AUTO ZÉRO.
- Déclencher un build macOS seulement après ces vérifications, pour éviter les runs inutiles.

## Conditions obligatoires avant de distribuer v0.15

- Reconstruction des sources JUCE depuis les correctifs GitHub **réalisée et validée sur Linux** ; vérifier à nouveau lors du build macOS.
- Raccordement des boutons, champs et du flux de préécoute **réalisé et compilé sur Linux** ; confirmer le résultat dans le Standalone, AU et VST3 sur Mac.
- Vérifier à l'écoute que le raccord entendu reproduit la méthode de fondu réelle du moteur v0.14 ; l'algorithme de référence devra être harmonisé si nécessaire.
- Contrôler les tailles des fenêtres sur Mac : **aucun chevauchement**, surtout sur écran 13 pouces.
- Vérifier l'UTF-8 et les accents français : **aucun symbole ne remplace une lettre**.
- Contrôler la compatibilité de sauvegarde/restauration DAW, avec `AUTO ZÉRO` inactif dans les projets antérieurs.
- Lancer les tests sans compilation payante au préalable ; déclencher **un seul build macOS** lorsque l'intégration est prête.

**Attention :** Les composants et tests présents dans le dépôt ne signifient pas que les nouveaux boutons sont déjà disponibles dans le plugin téléchargé.
