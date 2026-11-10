# Liberty ROMpler v0.18 — Recherche assistée du raccord de boucle

**Statut : développement sur `feature/v018-smart-loop-seam`. La dernière version macOS distribuée reste la v0.17 jusqu'à ce que le build v0.18 réussisse.**

## Fonction : SAUT MIN

Dans **SAMPLES > BOUCLES**, les deux boutons déjà présents conservent leur comportement :

- **Clic sur ZÉRO DÉBUT / ZÉRO FIN** : rechercher le passage par zéro le plus proche (fonction historique).
- **Option + clic sur ZÉRO DÉBUT / ZÉRO FIN** : chercher dans une plage de ±512 frames le point qui minimise **l'écart brut d'amplitude** entre la fin exclusive et le début inclusif de la boucle, en tenant compte des deux canaux quand le WAV est stéréo.

L'optimiseur fait un choix conservateur : il n'accepte pas un point dont le saut brut maximal serait supérieur à celui du point initial. Une différence de pente n'intervient que pour départager des candidats de saut similaire. Si aucun point n'améliore la mesure, le marqueur ne bouge pas.

### Méthode et limitations

- La recherche lit **au maximum environ 1 029 frames** près du marqueur mobile et deux frames à l'autre extrémité. Elle ne décode pas tout le WAV.
- La durée minimale de la boucle (11 frames) et les bornes du fichier restent respectées.
- Les changements de marqueurs restent **non appliqués** : cliquer sur **ANNULER** pour retrouver les positions enregistrées ; cliquer sur **APPLIQUER** seulement après vérification à l'écoute.
- Le résultat s'affiche en format court (SAUT MIN, marqueur et décalage) pour préserver la lisibilité des fenêtres compactes. L'infobulle conserve l'écart d'amplitude avant/après et invite à une comparaison A/B.
- Cette méthode **n'est pas un détecteur de clic audible** et ne garantit pas la meilleure continuité perceptive : l'écoute avec et sans fondu reste nécessaire.
- Aucun changement du fichier WAV, des banques, du format des presets ou du moteur temps réel.

## Contrôles obligatoires

- `Source/LoopSeamSearchV018.h` : algorithme C++17 autonome sans JUCE, sans modification du signal, avec bornage et prise en charge mono/stéréo.
- `overlays/v015/SampleZoneView.h` : utilisation des boutons ZÉRO existants avec le modificateur Option. Aucun nouveau bouton et aucun agrandissement de la fenêtre.
- `tests/loop_seam_search_v018_test.cpp` et `tests/test_v018_seam_search.py` : recherche stéréo, bornes, cas déjà optimum, valeurs non finies et non-destruction.
- Prévalidation sur Linux avec vérification des caractères français et compilation VST3 JUCE 8.
- Un seul build macOS AU/VST3/Standalone après validation Linux, puis contrôle manuel sur Mac.

**Règles Liberty : aucun chevauchement graphique et aucun symbole remplaçant les lettres françaises.**
