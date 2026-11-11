# Lot 2 — Cahier technique du streaming disque (non encore implémenté)

## Pourquoi différer

Le moteur actuel lit les zones entièrement en RAM et plafonne chaque zone à 20 secondes. Un streaming fiable exige davantage qu'une lecture WAV dans `renderNextBlock` : un accès disque dans le thread audio provoque des décrochages.

## Architecture proposée

- Préchargement de l'attaque (0,5 à 2 secondes) par zone pour garantir les transitoires lors de l'appui sur une note.
- Lecteur/décodeur WAV dédié, exécuté sur un ou plusieurs threads de lecture anticipée **hors callback audio**.
- Tampons circulaires par voix ou caches de tranches partagées, avec accès non bloquant et compteur de version pour éviter d'écraser un bloc lu par l'audio.
- Prélecture prioritaire selon le curseur réel de chaque voix et son ratio de lecture/transpose ; prise en compte de la tête et fin de boucle pour les crossfades.
- Politique mémoire plafonnée par banque, récupération hors audio, télémétrie de cache et détection des sous-alimentations.
- Lecture de fichiers de durée supérieure à 20 secondes, mono/stéréo, avec réinitialisation sûre en cas de changement de preset, reprise, arrêt, boucles et pitch bend.

## Critères d'acceptation avant activation

1. Aucun `AudioFormatReader::read`, ouverture de fichier, mutex bloquant, allocation ou libération mémoire dans `SampleVoice::renderNextBlock`.
2. Absence de clic et de trou avec au moins 64 voix, 4 couches, transposition variable et boucle crossfade.
3. Cas de sous-alimentation maîtrisé sans crash ni contamination des buffers ; métriques audibles et logs hors audio.
4. Tests de nettoyage des lecteurs à la fermeture et au changement de banques ; chargement des anciens projets inchangé.
5. Tests longs sur SSD et disques plus lents en AU, VST3 et Standalone ; test de consommation RAM.
6. Mettre à jour le validateur de banques et Smart Import pour lever la limite 20 s uniquement lorsque le streaming fonctionne réellement.

Cette version **ne revendique pas de streaming disque opérationnel**. Ce document définit le contrat pour l'implémentation ultérieure du Lot 2.
