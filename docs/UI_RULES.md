# Règles UI incontournables — Liberty Rompler

## 1. Français et encodage
- Tous les fichiers source restent en UTF-8.
- Les littéraux avec accents de l'éditeur sont exprimés via `u8"...\\u00e9..."` afin que le compilateur produise des octets UTF-8 corrects.
- Utiliser `juce::String::fromUTF8` et une police capable de rendre les caractères.
- Rejeter toute occurrence de `�` ou de séquences de mojibake comme `Ã©`.
- Tester notamment : Bibliothèques, Catégories, Résonance, Relâchement, Préset sélectionné.

## 2. Aucun chevauchement
- Trois colonnes non chevauchantes : banque, presets, modificateurs.
- Une fonction `panels()` définit la géométrie partagée entre `paint` et `resized`.
- Taille minimale de l'éditeur : 1120 × 750.
- Troncature ou réduction contrôlée du texte (une ligne), jamais débordement.
- Catégories dans un Viewport horizontal avec clipping plutôt que superposition.
- Lignes de presets : NOM / TYPE / BANQUE / étoile dans des rectangles distincts.
- Tests automatiques dans `tests/test_ui_regression.py` aux tailles (1120, 1180, 1360, 1600, 1920) × (750, 850, 1120).
- Effectuer une inspection native sur Mac avant de considérer le rendu totalement validé.

## 3. Inspiration Nexus 5, identité originale
- Reprendre le flux ergonomique : bibliothèques à gauche, recherche et presets au centre, modificateurs à droite.
- Ne reprendre ni logo, ni illustrations, ni assets, ni code propriétaire de reFX.
- Garder une palette et des composants graphiques Liberty spécifiques.

## 4. Contrôles réels
- Aucun potentiomètre décoratif : coupure, résonance, volume, attaque et relâchement sont connectés au DSP.
- Favoris et filtres fonctionnels, persistants pour les favoris.
- Sauvegarder les contrôles DSP dans l'état de session DAW.
