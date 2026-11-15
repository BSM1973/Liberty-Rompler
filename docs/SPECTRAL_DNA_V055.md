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
