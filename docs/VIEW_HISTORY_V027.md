# Liberty ROMpler v0.27 — Retour au cadrage précédent

**Statut : développement sur `feature/v027-previous-waveform-view`. La v0.26 demeure la dernière version macOS confirmée jusqu'au build v0.27 vert.**

## Nouveau geste dans SAMPLES > BOUCLES

**Maj + double-clic dans la forme d'onde** restaure le cadrage précédent (position horizontale et zoom), sans toucher à la boucle. Un nouveau **Maj + double-clic** inverse l'opération : les deux cadrages peuvent être comparés à volonté, tant qu'aucune autre navigation n'intervient.

La mémoire conserve **une étape**. Elle est mise à jour lors d'un zoom, d'un recentrage, d'un déplacement à la molette ou à la barre POSITION, d'un cadrage de boucle et d'un déplacement de la fenêtre avec Option + glisser. Pour un même geste glissé, la position initiale n'est enregistrée qu'une fois : les mouvements intermédiaires ne polluent pas l'historique.

Lorsque l'utilisateur charge un autre sample, cette mémoire est effacée. La restauration est refusée si la vue enregistrée n'est plus valide ou ne correspond pas au même fichier.

## Les raccourcis existants sont conservés

- Double-clic normal : afficher la totalité du WAV.
- Option + double-clic : cadrer la boucle active.
- Commande + double-clic : recentrer le point choisi sans modifier le zoom.
- Option + molette : zoomer sous le curseur.
- Maj + molette : naviguer horizontalement.
- Option + glisser : déplacer la fenêtre visible.
- Maj + glisser : déplacer toute la boucle avec une durée constante.
- Édition des marqueurs, ±1/±10, ZÉRO, SAUT MIN, PAIRE, préécoute A/B, ANNULER/APPLIQUER : conservés.

## Sécurité de l'édition

Le moteur C++17 `Source/LoopViewHistoryV027.h` ne dépend pas de JUCE, n'alloue pas de mémoire et n'exécute aucun traitement audio. L'interface enregistre uniquement le **zoom et la position d'affichage**. Elle ne touche **ni DÉBUT, ni FIN, ni FONDU**, ni les samples WAV, banques, presets ou projet.

Aucun nouveau bouton, panneau ou champ n'est ajouté. Une infobulle discrète présente le raccourci, en conservant les textes des versions précédentes.

Les deux règles Liberty demeurent absolues : **aucun chevauchement**, même sur un écran 13 pouces, et **aucun symbole erroné à la place d'une lettre accentuée française**.

## Validation

Tests du moteur : `tests/loop_view_history_v027_test.cpp` ; intégration des événements JUCE, raccourcis, Unicode et non-destruction : `tests/test_v027_view_history.py`. Prévalidation Linux complète avec JUCE 8 VST3 avant un seul build macOS AU/VST3/Standalone. Recette manuelle : `docs/GUIDE_TEST_BOUCLES_V027.md`.

Une réussite CI ne remplace pas une vérification visuelle sur un Mac réel.
