# Liberty ROMpler v0.36 — Liberty MORPH XY

## Concept

Pad XY réel de fusion progressive des **quatre couches sonores** A/B/C/D. Le moteur applique après chaque chaîne d'effets de couche, mais avant la chaîne globale, des coefficients bilinéaires :

- A (haut gauche) : (1-X)(1-Y).
- B (haut droite) : X(1-Y).
- C (bas gauche) : (1-X)Y.
- D (bas droite) : XY.

La somme vaut 1,0. Aux quatre coins une seule couche est entendue ; au centre chaque couche a 25 % de coefficient. Les gains lissés sur 45 ms évitent les sauts brutaux lors du déplacement ou de l'activation ; les notes restent déclenchées dans les quatre synthétiseurs.

## Contrôles

Nouvel onglet MORPH, pad XY avec point glissable et quatre coins identifiés, bascule MORPH ON/OFF, bascule MIDI CC16/17 et retour au centre par double-clic. Les paramètres hôtes réels : Liberty MORPH Active, Liberty MORPH X, Liberty MORPH Y, Liberty MORPH MIDI CC16/17. Gestes de souris transmis aux hôtes par beginChangeGesture / setValueNotifyingHost / endChangeGesture.

MIDI CC16 pilote X et CC17 pilote Y lorsque le mode MIDI est activé. En mode MIDI, les positions reçues prévalent sur l'automation de X/Y sans modifier leurs valeurs dans le DAW. Glisser sur le pad désactive la source MIDI.

## Compatibilité

**MORPH désactivé par défaut** ; mixage standard à 100 % de chaque couche comme dans v0.35. État ajouté au JSON DAW de façon facultative ; les anciens projets restent intacts et démarrent avec MORPH inactif. Aucun changement du format des banques ni du chargement WAV. Les effets de couche, effets maîtres, enveloppes, boucles et arpégiateur restent actifs.

Implémentation : `Source/MorphWeightsV036.h`, `Source/LibertyMorphPadV036.h`, `overlays/v015/PluginProcessor.cpp/.h`, `scripts/apply_v036_morph.py`.

Qualité : tests C++17 pour les quatre coins, le centre, les bornes et le MIDI ; vérifications Python des paramètres JUCE, de la géométrie et de l'interface, des caractères français et du comportement audio. Tests d'hôte macOS nécessaires après compilation.

Ne pas présenter cette version comme du morphing spectral ou granular : il s'agit d'un mélange dynamique de quatre couches audio indépendantes.
