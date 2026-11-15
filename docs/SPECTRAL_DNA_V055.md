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
