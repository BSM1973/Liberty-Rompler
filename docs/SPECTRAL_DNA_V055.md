# Liberty ROMpler v0.55 — SPECTRAL DNA (lot A : fondation)

## Statut
Branche de développement, **pas encore un effet audible dans AU/VST3**. Le moteur spectral v0.48 et ORBIT v0.54 restent inchangés. Aucun nouveau paramètre host, aucun changement de banque, aucune modification du thread audio et aucun build macOS déclenché par ce lot.

## Architecture
`Source/SpectralDNAV055.h` apporte une représentation de 128 bandes d'une empreinte harmonique par couche (A/B/C/D), la normalisation défensive des magnitudes, des poids XY bilinéaires, et une interpolation du profil cible à quatre couches. Elle propose un contrôle Wet et une priorité aux transitoires à l'attaque. `Settings.enabled` est **false** par défaut ; `wet` est **0** par défaut.

L'extraction FFT des profils et leur publication dans un cache immuable devront être réalisées hors du callback. Ce lot ne prétend pas faire la FFT, la resynthèse, la synchronisation du cache ou le traitement spectral en temps réel. Le branchement devra réutiliser le moteur spectral v0.48 plutôt qu'en créer un deuxième.

## Étapes suivantes
1. Examiner les interfaces du moteur v0.48 reconstruit, leurs coûts CPU et le protocole de publication des profils.
2. Extraire hors thread audio quatre profils de spectre depuis des fenêtres de samples; prévoir la régénération lors d'un changement de banque.
3. Raccorder les coefficients calculés à la resynthèse existante, avec dry/wet lissé et protection des attaques; aucun lock, FFT ni allocation dans processBlock.
4. Ajouter paramètres automatisables, stockage rétrocompatible et contrôles dans MORPH sans chevauchement. L'ancienne session doit garder un rendu bit-identique avec SPECTRAL DNA OFF.
5. Tester silence, NaN, basses fréquences, harmoniques, transitoires, quatre couches, streaming et SLICE; Linux d'abord, un seul build macOS après prévalidation et tests d'écoute réels.

## Tests de ce lot
`tests/spectral_dna_v055_test.cpp` vérifie les quatre coins, le centre XY, le bypass, la protection des transitoires et la robustesse des entrées invalides. La CI Linux compile en C++17 avec avertissements stricts et UBSan.

## Lot B — Analyse FFT hors thread audio (ajouté)
`Source/SpectralDNAAnalysisV055.h` met en place une FFT radix-2 de 256 points avec fenêtre de Hann, production d'un profil normalisé de 128 bandes et analyse de quatre fenêtres mono. Le code exige une exécution côté chargement/worker, **jamais dans processBlock**. Les entrées nulles, les silences, NaN et Inf sont neutralisés. Les résultats sont déterministes à plateforme donnée.

`tests/spectral_dna_analysis_v055_test.cpp` couvre les sinusoïdes aux bins 8 et 24, la sélection XY des couches, le silence, les nombres invalides et la protection des transitoires. La CI Linux exécute les deux tests C++17 avec UBSan et `-Werror`.

### Limites actuelles
- La FFT courte de 256 points est un prototype de profil harmonique, pas encore une analyse multirésolution adaptée aux graves.
- Aucun branchement à la lecture WAV, aucun cache de publication thread-safe et aucune resynthèse audiblement nouvelle ne sont livrés à ce stade.
- La prévalidation GitHub et les tests natifs en DAW doivent réussir avant la fusion. Ne pas annoncer un nouveau plugin fonctionnel tant que l'intégration JUCE reste inachevée.

## Lot C — Resynthèse timbrale hors ligne et banc audible (ajouté)
`Source/SpectralDNAResynthesisV055.h` fournit un prototype complet de rendu d'une fenêtre FFT de 256 échantillons : analyse, application d'une courbe harmonique cible au spectre complexe, conservation de la phase des bins existants, FFT inverse et production d'un bloc audio calculé. Avec Wet=0, la sortie est strictement égale à l'entrée. Les gains sont bornés, et une bande sans énergie originale ne crée pas d'harmonique par magie.

`tests/spectral_dna_resynthesis_v055_test.cpp` vérifie la modification effective des échantillons d'un mélange de deux sinusoïdes, le bypass exact, un réglage intermédiaire et la symétrie complexe. La CI Linux intègre ce test avec UBSan.

**Limite importante :** ce prototype est exécuté hors thread audio. Le flux temps réel n'est pas raccordé. Il manque la reconstruction overlap-add (WOLA/COLA), la compensation de latence, le cache de frames préparés, l'adaptation à l'API de resynthèse v0.48 reconstruite en CI, la modulation ORBIT continue et les contrôles AU/VST3. Le profil FFT de 256 points n'autorise pas encore un véritable morphing piano → voix si le signal d'origine ne contient pas les harmoniques visées. Ne pas prétendre que l'effet est audible *dans le plugin* à ce stade.

## Lot D — Flux audio overlap-add et modulation ORBIT (prototype hors ligne)
`Source/SpectralDNAWolaV055.h` ajoute un moteur de référence à fenêtre Hann périodique, FFT 256, saut de 64 échantillons et reconstruction overlap-add normalisée par la somme des fenêtres au carré (WOLA). Les trames sont positionnées sur une grille temporelle cohérente; leur signal traité est aligné avec le signal d'entrée hors ligne. Le bypass OFF ou Wet=0 copie exactement le signal sans FFT. La trajectoire ORBIT/INFINITY/CONSTELLATION de `OrbitEngineV054.h` est calculée une fois par saut de 64 échantillons pour faire évoluer les poids des profils A/B/C/D.

`tests/spectral_dna_wola_v055_test.cpp` vérifie l'identité du bypass, la fidélité de l'overlap-add sur une combinaison de deux sinusoïdes (profil uniforme), un mouvement ORBIT sur plusieurs trames, les valeurs finies et un flux vide. Le workflow Linux précompile le test avec UBSan et `-Werror`.

### Ce qui n'est PAS encore intégré
- `renderOffline` utilise des `std::vector` et effectue les FFT pendant la reconstruction, et n'est donc PAS utilisable directement dans `processBlock`.
- Le report de latence en temps réel, la gestion de la queue de travail (sous-alimentation, dépassements, synchronisation), les transitions sans clic lors des changements de banque et la publication lock-free de profils ne sont PAS implémentés.
- Le pipeline natif v0.48 extrait par `scripts/apply_v048_bundle.py` doit être audité et exploité sans créer de deuxième moteur spectral dans le plugin. Aucun raccordement JUCE, aucun nouveau AU/VST3 et aucun build macOS ne sont revendiqués.
- Les tests CI doivent être confirmés verts et l'écoute sur l'interface physique validée avant la fusion de PR #49.

## Lot E — Etage de sortie temps réel, retard dry/wet et banc 64 voix (code publié)
`Source/SpectralDNARealtimeV055.h` introduit un étage de sortie mono indépendant par voix avec un tampon circulaire fixe de 4096 échantillons. Il aligne le dry sur un wet **déjà rendu et déjà aligné** par la chaîne spectrale, avec une latence configurable avant lecture. La valeur initialement suggérée est 256 échantillons mais ce n'est PAS une latence validée ou reportée au DAW. Le bypass OFF retourne l'entrée à l'identique; pendant le mode actif, les variations du taux wet sont limitées à 1/128 par échantillon. Si les échantillons wet manquent, un fondu de secours ramène le dernier échantillon wet valide vers le dry retardé sur au plus 128 échantillons. Ce mode de secours évite un saut instantané mais **ne remplace pas une stratégie de concealment testée à l'écoute**.

`tests/spectral_dna_realtime_v055_test.cpp` simule 64 instances indépendantes, vérifie la mémoire fixe, l'absence de NaN, le bypass exact, l'alignement d'un impulse dry avec 256 samples de latence, les fondus Wet, un manque de données et les bornes de latence. `tests/spectral_dna_rt_benchmark_v055.cpp` mesure le p99 et le maximum de calcul de **cet étage de sortie seulement** à 64 voix / 48 kHz / 128 frames. Le test chronométré est informatif, sans seuil CI instable. Tout le code de ce lot est ajouté à la prévalidation Linux, sans modifier le workflow macOS de la v0.54.

### Limites de certification
- L'étage de sortie n'est pas encore appelé depuis `PluginProcessor::processBlock`, et aucun résultat AU/VST3 n'est revendiqué.
- Le tampon dry n'est PAS un transport de trames entre threads : la file audio wet, le réveil des workers, la propriété des buffers, la compensation du délai moteur et le report JUCE `setLatencySamples` restent à construire.
- Un bypass sans délai sur un flux qui était actif peut provoquer une discontinuité et ne doit pas être présenté comme commutation sans clic, même si le bypass historique est exact à l'arrêt.
- La transformation piano -> voix réelle nécessite un modèle de sources/excitations, un suivi de la fondamentale, un traitement des formants et des transitoires; une enveloppe spectrale de 128 bins seule ne suffit pas.
- Les résultats du workflow Linux, un audit du moteur v0.48 et les essais sur interface audio réelle restent des conditions préalables à la fusion de la PR #49.

## Lot F — Transport PCM SPSC, générations de presets et contrat de latence
`Source/SpectralDNAPcmQueueV055.h` définit une file mono SPSC de 32 trames de 64 échantillons : un producteur (worker) et un consommateur (audio), sans allocation, verrou ou attente dans push/pop. Les frames portent un numéro de génération et une position d'échantillon. Le curseur audio refuse les trames d'un ancien preset, les horodatages inattendus et les nombres non finis. Débordement = refus de push ; sous-alimentation = refus de pop, à gérer par le fondu de secours du lot E. Le reset de la file doit avoir lieu **uniquement avant** le démarrage ou après arrêt coordonné des deux threads. Il n'est PAS concurrent.

`Source/SpectralDNASessionV055.h` définit un epoch atomique pour les changements de presets, et un contrat provisoire de 256 échantillons de latence. La méthode `configureHostLatency` est une interface à appeler depuis une phase de préparation autorisée par le DAW ; elle n'est PAS appelée automatiquement et doit être auditée avant intégration JUCE. Les deux contrats disposent de tests C++17 ajoutés à la prévalidation Linux.

### Étapes restantes obligatoires
1. Extraire et inspecter le véritable code du moteur v0.48 reconstitué en CI ; déterminer exactement où ses trames audio calculées sont disponibles.
2. Fournir un worker réel avec horodatage source, coordination des propriétés des buffers et publication vers cette file (la file seule ne produit aucun son).
3. Consommer les trames dans `processBlock`, par canal/voix avec stratégie temporelle définie, sans mélanger des sons de voix différentes. Le prototype de file est mono et SPSC : pour une polyphonie 64 voix, il faut une stratégie de transport et un budget mémoire/CPU précis.
4. Mesurer la latence effective avant d'appeler `setLatencySamples` dans une phase hôte conforme; 256 est un chiffre **provisoire**, non validé.
5. Implémenter le changement de preset sans clic (invalidation + fondu vers le dry), analyser les sous-alimentations, puis lancer les tests de charge dans les DAW.
6. Ne fusionner la PR #49 et ne lancer le build macOS v0.55 qu'après confirmation de la prévalidation et de l'intégration réelle.

Cette étape apporte des briques vérifiables pour le vrai temps réel mais **ne constitue pas encore un raccordement au moteur spectral v0.48** ni une version AU/VST3 opérationnelle.

## Lot G — Premier raccordement JUCE réel (branche expérimentale)
Le script `scripts/apply_v055_juce_spectral_dna.py`, appliqué **après v0.54** pendant les builds Linux et macOS, connecte des paramètres AU/VST3 `libertySpectralDNAEnabled` (OFF par défaut) et `libertySpectralDNAMix` (0 par défaut) aux **quatre véritables moteurs `spectral_v048::Engine`** déjà appelés dans `PluginProcessor::processBlock`. Les trajectoires MORPH/ORBIT existantes modulent la part du signal de resynthèse harmonique de chaque couche. Les anciennes valeurs spectrales des quatre couches sont inchangées quand DNA est OFF.

**Architecture réelle vérifiée :** la v0.48 analyse par FFT 2048 hors callback, construit des tables harmoniques antialiasées et produit dans `render` un oscillateur polyphonique sans FFT ni file PCM. La voie JUCE raccordée réutilise cette architecture préexistante. Les prototypes WOLA/FFT 256, SPSC et la classe `SessionControl` v0.55 restent **inactifs** dans le plugin pour éviter une double synthèse et des changements de latence injustifiés.

### Latence et limites
- La voie directe `spectral_v048::Engine::render` n'introduit pas le lookahead WOLA de 256 frames. Les 256 samples de `SessionControl` sont une constante expérimentale inutilisée : **ne pas les déclarer au DAW dans cette voie**. Mesurer l'éventuelle latence du pipeline complet avant toute nouvelle déclaration.
- Le morphing audible actuel module les niveaux de quatre resynthèses harmoniques, pas les formants/partiels d'une voix vers un piano. Le vrai morphing timbral profond exige une banque de tables hybrides ou la fusion de profils avant resynthèse.
- Les contrôles existent en paramètres hôte AU/VST3 ; une UI MORPH dédiée, les tests audio A/B sous DAW et la validation macOS sont encore nécessaires.
- Aucun test du banc natif v0.48 existant ne remplace un test A/B ciblant les nouveaux paramètres SPECTRAL DNA.

## Lot H — Commandes MORPH, tests A/B natifs, fondation formants
`scripts/apply_v055_dna_ui.py` insere sur la page MORPH un bouton SPECTRAL DNA ON/OFF et un curseur de 0 a 100 %, tous deux connectes aux parametres hote AU/VST3. Un panneau bleu nuit compact est reserve dans le bas de l'interface, avec espace conserve au-dessus des textes de pied et sous le pad XY. Aucun nouveau panneau flottant n'est cree.

`Source/SpectralDNAMixV055.h` centralise le calcul du melange DNA utilise en production dans `PluginProcessor.cpp` et dans les tests. Quand DNA=OFF, le niveau spectral v0.48 est renvoye a l'identique. `tests/spectral_dna_native_ab_v055_test.cpp` utilise le vrai moteur v0.48 avec signal test et compare DNA OFF et ON numeriquement; cela atteste une modification de la resynthese, pas une appreciation auditive.

`Source/SpectralDNAFormantsV055.h` introduit un prototype hors ligne de deplacement et interpolation d'enveloppes spectrales, teste sur des pics synthetiques a l'aide de `tests/spectral_dna_formants_v055_test.cpp`. Il n'est pas relie aux tables harmoniques JUCE v0.48: aucun detecteur de formants, aucune correction de hauteur ni transformation voix/piano complete n'est revendique. Son raccordement devra intervenir sur la preparation des frames hors audio thread.

## Validation manuelle obligatoire
Tester AU et VST3 dans Ableton Live, Logic Pro et Bitwig, a 44,1/48/96 kHz et blocs 64/128/512. Auditionner les quatre coins du pad, le centre, ORBIT et CONSTELLATION; comparer DNA OFF/ON a niveau compense, demarrages et arrets de notes, changement rapide de presets, sauvegarde et rappel de projet. Mesurer clics, pics CPU, voix et differences spectrales; reproduire a 64 voix. Les tests CI de resynthese ne sont pas une ecoute humaine, et un build vert ne la remplace pas.

## Lot I — Défaut SLICE intermittent détecté pendant la validation macOS
Le run macOS `43702458590` a compilé AU/VST3/Standalone, mais son test SLICE strict a échoué sur une répétition : quatre cache misses et quatre blocs sous-alimentés vers 5,99 s ; les deux autres répétitions ne présentaient aucune sous-alimentation. Le signal est conservé comme un **échec réel de QA**, et non masqué en modifiant les seuils.
La branche v0.55 teste un ajustement conservateur du préchargement SLICE : seuil anticipé de 0,65 s au lieu de 0,38 s (à vitesse 1x), et rafraîchissement de 40 ms au lieu de 75 ms lorsque la zone préchauffée est encore sûre. Les limites restent bornées et les appels sont non bloquants; le coût CPU et les changements de débit doivent être mesurés. Le test `tests/slice_prefetch_v055_test.cpp` vérifie les décisions de cadence. Les trois répétitions natives macOS doivent toutes repasser sans sous-alimentation, sans défaut de cache et sans dépassement de budget avant qualification. En cas d'échec persistant, il faudra instrumenter les pages lues aux frontières et l'ordonnancement des workers plutôt que d'augmenter aveuglément les requêtes.

## Lot J — Formants réellement raccordés aux tables v0.48 (expérimental)
Le script `scripts/apply_v055_formants.py` enrichit `Source/SpectralEngineV048.h` après reconstruction de la v0.54 et installation du contrôle DNA. Pour chaque frame FFT de 2048 points analysée hors callback, il prépare deux banques de tables supplémentaires, représentant une interpolation de l'enveloppe harmonique avec une excursion de ±0,5 octave. La phase et la fondamentale de chaque harmonique sont conservées dans ces banques; les gains sont bornés, les pics de variantes plafonnés, et la position FORMANTS=0 utilise toujours la table historique sans lecture supplémentaire.

Dans `Engine::render`, l'interpolation entre la table historique et la variante basse/haute demande uniquement des accès à des tables immuables, quelques multiplications et aucune FFT, allocation, attente ou opération disque. L'interface MORPH propose un curseur FORMANTS de -12 à +12 demi-tons en façade, lié au paramètre hôte `libertySpectralDNAFormant` (plage normalisée -1 à 1, 0 par défaut). Ce contrôle agit seulement lorsque SPECTRAL DNA est actif; DNA OFF conserve l'ancien rendu. Le curseur utilise une étendue de ±12 demi-tons pour piloter une excursion morphologique des enveloppes préparées de ±6 demi-tons : ce n'est pas un transpositeur ni un suivi de formants vocaux.

`tests/spectral_dna_native_formants_v055_test.cpp` compare le moteur réel v0.48 avec -1/0/+1, vérifie la validité des tables, les échantillons finis, les changements audibles de forme d'onde et l'identité numérique du rendu neutre. Les workflows Linux et macOS sont raccordés à ce test.

**Limites à lever :** les variantes supplémentaires augmentent le coût d'analyse et la mémoire des frames; aucune mesure de 64 voix SPECTRAL DNA ON + FORMANTS variable n'est encore qualifiée. Les transitions rapides du paramètre FORMANTS ne sont pas encore lissées à l'échantillon : auditer les clics en DAW avant publication. Le moteur ne sépare pas encore excitateur et filtre vocal (ce n'est pas une conversion voix-piano). La qualité audio, la largeur du pad XY, les changements de presets et la compatibilité AU/VST3 doivent être confirmés avec la compilation native et l'écoute.

## Lot K — Lissage du contrôle FORMANTS dans le vrai moteur JUCE v0.48
`Source/SpectralDNAFormantSlewV055.h` fournit un état par moteur spectral, sans allocation dynamique, pour limiter les variations du contrôle FORMANTS à 2/256 par échantillon (balayage complet -1 vers +1 en 256 samples ; centre vers une extrémité en 128 samples). `scripts/apply_v055_formants.py` relie ce lissage aux interpolations de tables préparées dans `SpectralEngineV048.h` directement dans la boucle `render`, avec remise à zéro dans `prepare` et lors de la publication d'une nouvelle génération de preset. À neutralité constante et sans ancien mouvement, le chemin historique continue d'utiliser sa table d'origine.

`tests/spectral_dna_formant_slew_v055_test.cpp` contrôle la progression monotone, les limites de pas, la remise à zéro, la neutralité et les cibles invalides; le test réel de tables `spectral_dna_native_formants_v055_test.cpp` reste exécuté sur Linux et macOS. Ce lissage réduit les discontinuités de modulation, mais ne prouve pas à lui seul l'absence de clic lors des changements de banque ou d'une lecture disque difficile. L'audition en DAW et les mesures de 64 voix restent obligatoires. Aucun lookahead de 256 samples n'a été activé : le nombre 256 décrit ici uniquement une durée de rampe de paramètre, sans latence ajoutée.

## Lot L — Banc 64 voix spectral + FORMANTS + ORBIT (vrai moteur v0.48)
Le nouveau `tests/spectral_dna_64voice_v055_bench.cpp` prépare hors callback quatre spectres harmoniques distincts, active 16 notes par couche (64 voix), module l'XY via le moteur ORBIT v0.54 et fait osciller FORMANTS pendant 360 blocs de 128 échantillons à 48 kHz. Il mesure les durées de rendu p99/max, le nombre de blocs au-dessus du budget de 2,667 ms, RMS/pic, la variation maximale entre échantillons et le nombre de valeurs non finies. La mémoire comptabilisée représente uniquement les 4 frames de tables originales et formantiques (3 × 7 × 2048 flottants par couche), soit 688 128 octets, sans compter les moteurs, le cache des banques, les allocations JUCE ou le système.

Prévalidation Linux `43709902064`, verte : p99=61,250 µs ; maximum=71,716 µs ; dépassements=0 ; mémoire des tables=688128 octets ; pic=0,5081 ; max variation inter-échantillon=0,0727 ; RMS=0,1336 ; valeurs non finies=0. Ces chiffres proviennent du runner Linux et NE qualifient PAS la chaîne complète macOS, le DAC, le streaming ni les DAW. Le test CPU est informatif et ne bloque pas la CI à cause des écarts de machines ; la non-finitude, la sortie muette et la polyphonie sont des assertions strictes. La variation inter-échantillon ne constitue pas à elle seule une preuve d'absence de clics.

Le build macOS devra passer le même banc et tous les contrôles SLICE stricts (aucune sous-alimentation) avant toute qualification. Le lissage FORMANTS ne garantit pas la continuité lors des changements de presets ou d'une indisponibilité disque. Le prototype WOLA et sa latence expérimentale de 256 samples restent inactifs.
