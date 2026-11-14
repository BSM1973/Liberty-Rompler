# Liberty ROMpler v0.52 — Interpolation PCM en temps réel et diagnostic des pics

## Référence v0.51
Run macOS [43598114427](https://github.com/BSM1973/Liberty-Rompler/actions/runs/43598114427) : 0 sous-alimentation poly64, SLICE et switch ; 227 Mo de RSS max lors des changements de presets ; **10 dépassements de budget wall-clock** sur 1 982 callbacks poly64 (512 échantillons à 44,1 kHz = 11,61 ms par bloc). Le 99e percentile wall-clock était 5,32 ms. Le banc CI n'utilise ni carte son ni support USB physique.

## Optimisation ciblée
- **Cache PCM :** lecture de deux frames stéréo consécutives avec un seul protocole atomique de reader pin + recheck. Identité PCM préservée, une seule prise de référence par paire ; au bord d'une page ou en cas de page indisponible, repli sur le chemin scalaire existant.
- **RAM :** accès direct à deux frames d'un même tampon d'attaque, de tête de boucle ou d'une tranche SLICE ; la lecture existante est conservée aux frontières.
- Aucun changement des 32 pages cache, de la politique mémoire v0.51, des 4 workers, du format des banques, des 11 onglets, des synthèses analogique/wavetable/FM/granulaire/spectrale et des paramètres enregistrés. Pas d'allocation, d'attente ou de disque sur le callback.
- Banc macOS enrichi : `callback_thread_cpu_peak_ms`, `callback_thread_cpu_p99_ms` et `callback_spikes[].thread_cpu_ms`, pour distinguer dépense CPU réelle du thread audio et retard wall-clock (préemption/ordonnanceur OS). La mesure emploie le CPU clock du thread du **banc**, pas une horloge dans le DSP du plugin.

## Acceptation
Comparer v0.52 à v0.51 avec exactement les mêmes 64 voix, 23 s, 44,1 kHz / 512 samples, plus SLICE et switch : aucun nouveau miss de cache ni sous-alimentation, RSS switch sans régression, audio non-fini zéro. Mesurer la durée des callbacks et leur CPU thread en parallèle pour déterminer si les pics viennent du code ou du scheduler. **Un build vert ne garantit pas zéro clic dans un DAW.** Ne déclarer ni validation USB lent ni stabilité matérielle tant que des essais dans Ableton, Logic et Bitwig avec périphérique réel ne sont pas réalisés.
