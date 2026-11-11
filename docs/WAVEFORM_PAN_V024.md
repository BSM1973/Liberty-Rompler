# Liberty ROMpler v0.24 — Déplacement direct de la vue de forme d'onde

**Statut : en développement dans `feature/v024-option-drag-waveform-pan`. La v0.23 est la dernière distribution macOS validée tant que le build v0.24 n'est pas vert.**

## Nouvelle commande dans SAMPLES > BOUCLES

**Option + glisser sur la forme d'onde** déplace la fenêtre visible horizontalement, sans toucher aux marqueurs de boucle.

- **Option + glisser vers la droite** : afficher des frames plus anciennes, vers le début du WAV.
- **Option + glisser vers la gauche** : afficher des frames plus récentes, vers la fin du WAV.
- La translation est mesurée par rapport à la **position de la souris au clic initial** : aller et revenir au point initial ne provoque aucune dérive.
- Les déplacements sont limités aux bornes du WAV. Au zoom 1× (tout le fichier déjà visible), la vue reste immobile.
- L'opération fonctionne sur la forme d'onde même quand la boucle n'est pas activée, tant qu'une zone audio est sélectionnée.

## Compatibilité avec les commandes existantes

- **Glisser normalement un marqueur** : déplacer uniquement DÉBUT ou FIN.
- **Maj + glisser à l'intérieur de la boucle** : déplacer les deux marqueurs en conservant la durée (v0.21).
- **Option + molette** : zoom sur le point visé par le curseur (v0.23).
- **Option + double-clic** : cadrer la boucle (v0.22).
- Molette sans Option et défilement horizontal : comportements conservés.
- SAUT MIN, PAIRE, boutons ±1/±10, préécoute A/B et ANNULER/APPLIQUER restent inchangés.

La nouvelle navigation ne touche **ni DÉBUT, ni FIN, ni FONDU**, et ne modifie ni le WAV, ni la banque, ni le preset, ni l'état du DAW. Aucun code supplémentaire n'est exécuté dans le callback audio `processBlock()`.

L'algorithme `Source/LoopPanV024.h` est indépendant de JUCE, sans allocation et avec calculs sur 64 bits pour les grands fichiers. Les gestes sont intégrés à l'éditeur `overlays/v015/SampleZoneView.h` et ne nécessitent **aucun nouveau bouton ni redimensionnement de fenêtre**.

## Deux règles Liberty

1. Aucun chevauchement de textes, boutons ou marqueurs, y compris en fenêtre compacte / sur écran 13 pouces.
2. Aucun caractère parasite à la place des lettres françaises : les échappements Unicode sont vérifiés dans les sources C++.

## Contrôles de qualité

- `tests/loop_pan_v024_test.cpp` : ancrage, arrondis, début/fin du fichier, vue complète, grands entiers et données invalides.
- `tests/test_v024_waveform_pan.py` : compilation C++17, priorité des modificateurs, absence d'écriture sur la boucle, compatibilité des anciennes commandes, accents français.
- Prévalidation Linux : suite de régression, contrôle de géométrie et compilation JUCE 8 VST3.
- Distribution : **un seul build macOS** après résultat Linux entièrement vert (AU, VST3, Standalone, ZIP, DMG et sources).
- Contrôle manuel macOS : `docs/GUIDE_TEST_BOUCLES_V024.md`.

Les tests automatisés ne remplacent pas la validation visuelle et auditive sur un Mac réel.
