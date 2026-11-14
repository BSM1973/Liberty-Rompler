# Liberty ROMpler v0.53 — SLICE : prélecture temporisée et répétition native

## Référence
v0.52, macOS [43621242966](https://github.com/BSM1973/Liberty-Rompler/actions/runs/43621242966) : 0 sous-alimentation SLICE, p99 2,338 ms, mais un callback isolé de 23,384 ms (wall-clock) et 23,387 ms CPU thread à 4,934 secondes (512 frames, 44,1 kHz, budget 11,61 ms). L'origine de cet unique pic n'est pas démontrée.

## Correction prudente
Lorsque SLICE joue depuis une fenêtre PCM préchargée, les demandes disque « critical + ahead + ahead » sont rafraîchies environ toutes les 75 ms au lieu de chaque bloc. Au moins 0,38 seconde de lecture avant le bord de la fenêtre, elles redeviennent systématiques, selon la vitesse de lecture et la fréquence. En l'absence de fenêtre en RAM ou au-delà de son bord, l'ancien système de requêtes est conservé.

Aucune modification du cache 32 pages, des quatre workers, des 64 voix non-SLICE, du traitement spectral/granulaire, des banques, des onze onglets ni des paramètres DAW. Pas d'allocation, attente, disque ou verrou ajouté au callback.

## Validation
- C++17 : cadence, rafraîchissement, limite de fenêtre, 44,1/96 kHz, forte vitesse et reset MIDI.
- Banc JUCE natif SLICE : trois exécutions indépendantes de 6 secondes, synthèse des underflows, ratés cache, p99, durées wall-clock et CPU-thread.
- Banc original poly64, switch et SLICE maintenu, plus tests hybride, granulaire, spectral et sauvegardes.
- Aucune qualification matérielle sur le runner GitHub : suivre le GUIDE_TEST_DAW_USB_V053.md dans un DAW et avec un véritable disque USB.

Le pic v0.52 peut être ponctuel : ne prétendre ni correction avérée ni cause exacte sans répétitions et profilage.
