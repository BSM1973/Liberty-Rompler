# Liberty Rompler v0.10 — Interface premium à neuf onglets

L'interface conserve son thème sombre Liberty, son navigateur de presets large, ses modules audio et son clavier MIDI sur toute la largeur inférieure.

**Navigation, dans l'ordre :** PRESETS · COUCHES · ROUTAGE · EFFETS · MODULATION · MODULATEURS · ARP / STEP · SAMPLES · RÉGLAGES.

Le nouvel onglet **SAMPLES** donne accès aux zones WAV réelles du preset actif, y compris les références de presets multicouches : fichiers sources, note fondamentale, plages clavier/vélocité, boucles et visualisation de forme d'onde grâce à `juce::AudioThumbnail`. Cet inspecteur reste en **lecture seule** ; il ne réécrit ni les fichiers audio ni les manifests des banques.

Les 4 couches, le routage graphique, les 6 effets, les modulateurs et l'arpégiateur 32 pas restent inchangés dans leur fonctionnement. Les paramètres et presets DAW antérieurs conservent leur format d'état.

**Contrôles :** audit Unicode/accents en français, tests de géométrie à 9 onglets et 15 tailles de fenêtres, validation des banques et tests musicaux. Le rendu macOS réel et la stabilité du défilement restent à vérifier à l'ouverture dans un DAW.

**Distribution :** le workflow GitHub Actions n'est déclenché automatiquement que si son propre fichier est modifié ; un seul build est prévu pour cette livraison. Les futures modifications de code restent sans compilation jusqu'à validation d'un nouveau lot. Les artefacts incluent le ZIP, le DMG et une archive des sources générées pour faciliter les correctifs ultérieurs.

[Voir les compilations GitHub Actions](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml).

---

# Liberty Rompler v0.9 — Correction de l'arpégiateur et nouveaux motifs

Cette mise à jour regroupe dans **un seul build** :
- **Correctif JUCE 8.0.10** du compilateur macOS : l'itérateur MIDI du séquenceur utilise désormais la déréférence correcte, sans accès invalide par pointeur
- **6 modèles de séquences** : Droit, House, Trance, Basse, Syncope et Montée 32 pas
- **4 outils musicaux** directement dans l'onglet ARPÉGIATEUR : Inverser, Décaler (rotation), Humaniser la vélocité et Réinitialiser
- **Géométrie revue** : barre d'outils séparée des 32 pas et réduction adaptative des vu-mètres de vélocité lorsque la fenêtre est petite
- **Tests plus stricts** : le simulateur JUCE ne prend plus en charge l'accès erroné par pointeur ; nouveaux tests C++ pour motifs, MIDI, géométrie et encodage des accents
- Les modifications de motifs utilisent les paramètres déjà sauvegardés dans le DAW ; aucune migration des banques n'est nécessaire

**Aucun build GitHub Actions automatique :** le workflow est uniquement manuel. Afin d'éviter tout coût de runner macOS, compiler localement sur un Mac Apple Silicon avec Xcode et CMake, après clonage du dépôt :

```bash
git clone https://github.com/BSM1973/Liberty-Rompler.git ~/Liberty-Rompler
bash ~/Liberty-Rompler/scripts/build_local_github_v09.command --install
```

Si le dépôt existe déjà, remplacer la première commande par `git -C ~/Liberty-Rompler pull --ff-only`. Le script vérifie chaque archive (SHA-256), lance les tests et compile les versions AU, VST3 et autonome en une seule fois. Il crée aussi un DMG et un ZIP sous `~/Liberty-Rompler/.local-build/dist/`.

**État :** tests unitaires Linux verts, mais compilation native JUCE/macOS et essais audio dans le DAW **pas encore confirmés**. Les nouvelles actions de motifs affectent la séquence actuellement chargée et sont enregistrées avec le projet. Le bouton Humaniser fait varier la vélocité, pas le timing, afin de conserver la synchronisation au tempo.

---

# Liberty Rompler v0.8 — Arpégiateur professionnel

Le nouvel onglet **ARPÉGIATEUR** est directement connecté au moteur MIDI de l'instrument à quatre couches. Il est **OFF par défaut**, afin que les presets historiques et les projets existants se comportent comme avant.

### Fonctionnalités
- **1 à 32 pas** configurables ; 32 cellules en grille de 2 × 16.
- Modes **UP**, **DOWN**, **RANDOM** (aléatoire) et **CHORD** (accord).
- **1 à 4 octaves**, choix de division musicale 1/4, 1/8, 1/16 ou 1/32.
- **Swing 0–75 %**, **Gate 5–98 %**.
- Pour chaque pas : vélocité 1–127, transposition de −24 à +24 demi-tons, pas actif ou silencieux.
- Édition : glisser = vélocité ; Maj + glisser = transposition ; clic droit = activer/désactiver un pas. Pas courant surligné durant le jeu.
- Horloge basée sur le **BPM du DAW** avec alignement initial à la grille **PPQ** si le transport le fournit. Si le DAW n'indique pas de tempo, horloge de secours à 120 BPM.
- Notes MIDI générées **avant** les filtres, enveloppes et quatre couches ; événements programmés à l'échantillon près dans chaque bloc.
- Gestion des notes tenues et libérées, arrêt propre lorsque le dernier doigt quitte le clavier, lors d'une désactivation, et réinitialisation sur changement de position de transport.
- Paramètres et 32 pas enregistrés/restaurés avec l'état DAW. Compatibilité conservée avec les versions v0.1 à v0.7.

### Qualité et limites
**34 tests** passés localement, dont une suite C++ native testant réellement les événements MIDI Up/Down/Random/Chord, octaves, Swing, Gate, vélocité, transposition, PPQ et libération des notes. Audit Unicode français et géométrie 8 onglets vérifiés. Le rendu visuel, les hôtes DAW et le timing de transport sur Mac doivent néanmoins être validés en usage réel.

Prototype : l'arpégiateur génère les notes **à l'intérieur de l'instrument** pour les couches Liberty ; il n'exporte pas encore de piste MIDI vers d'autres instruments. La synchronisation utilise les informations disponibles dans le DAW ; des hôtes ne fournissant pas PPQ ne peuvent bénéficier que du tempo BPM.

[Voir la dernière compilation](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml), et télécharger le ZIP/DMG depuis **Artifacts** après validation verte.

---

# Liberty Rompler v0.7 — Onglet ROUTAGE graphique

**Routage fonctionnel, pas une illustration statique.** Le nouveau panneau ROUTAGE visualise les quatre couches audio et leur convergence vers la chaîne principale.

- Quatre synthétiseurs indépendants de **32 voix chacun**, soit jusqu'à 128 voix réparties sur les quatre couches.
- Chaque couche possède son propre filtre déjà intégré au moteur v0.6 et **deux emplacements d'effets audio indépendants**. Glisser un effet depuis la palette vers un emplacement ; échanger les deux blocs ou déplacer un effet vers une autre couche ; double-cliquer ou faire un clic droit pour retirer l'effet.
- Six effets disponibles sur chaque couche et sur le bus global : Distorsion, EQ, Compresseur, Chorus, Delay et Réverbération. Chaque couche et le bus global possèdent leurs propres mémoires d'effets (chorus, réverbération, delay, filtre et compresseur) pour éviter les échanges de signal entre couches.
- **Chaîne globale** de six blocs : déplacer un effet sur un autre pour échanger leurs positions. L'ordre affiché est réellement l'ordre du DSP, après le mélange des couches et le filtre global.
- **Sauvegarde DAW** des deux emplacements sur chaque couche et de l'ordre des six effets globaux. Les anciens projets conservent la chaîne historique Distorsion → EQ → Compresseur → Chorus → Delay → Réverbération et les slots de couche désactivés.
- Les réglages de chaque type d'effet restent actuellement partagés avec l'onglet EFFETS ; les traitements et les états DSP sont indépendants par couche. Le routage est **en série**, sans bus auxiliaire ni câblage arbitraire à ce stade.
- Répartitions clavier, vélocité, volumes, panoramiques et sources des quatre couches restent opérationnels.
- **30 tests automatisés** avant compilation : interfaces existantes, nouveaux chemins DSP, ordre des effets, sérialisation, calculs sans superposition à 15 tailles de fenêtre. Audit UTF-8 strict : accents français corrects, caractères de remplacement et corruption interdits.

Cette v0.7 reste un **prototype**. L'interface doit être validée visuellement sur macOS et le son doit être écouté après le build. Les effets à 0 % de mixage ou à réglages neutres n'altèrent pas le signal. Les échantillons restent chargés en mémoire, sans streaming disque.

La mise à jour est intégrée sous forme de patch source vérifié par SHA-256, dans `assets/ui-v07/part-{1..5}.b64`. Le workflow reconstruira les versions précédentes, appliquera le correctif du header v0.6, puis la v0.7 avant les tests et la compilation.

[Compiler et télécharger depuis GitHub Actions](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml) — ouvrir le dernier run vert, section **Artifacts**.

---

# Liberty Rompler — v0.6 : moteur multisample à 4 couches

**Le nouveau moteur combine jusqu’à quatre sources sonores dans un même preset.** L'onglet **COUCHES** affiche quatre panneaux distincts en grille 2 × 2. Chaque couche possède : sélection d'une source de banque et d'un preset, activation, **volume, panorama stéréo, transposition en demi-tons, filtre passe-bas, plage de notes et plage de vélocité**. Le moteur charge les zones multisamples référencées et les superpose pendant le jeu, avec 128 voix au maximum sur l'ensemble des couches. Les réglages des couches et les références aux sources sont conservés dans la sauvegarde de session du DAW.

**Compatibilité descendante :** les banques et presets historiques (zones WAV) restent lisibles. L'extension du format JSON accepte une propriété facultative `layers` (4 références au maximum : `source_bank`, `source_preset`, `enabled`, `gain`, `pan`, `transpose`, `cutoffHz`, `lowNote`, `highNote`, `lowVelocity`, `highVelocity`). Chaque couche source désigne un preset de zones directement jouable, et non une autre construction multicouche.

**Preset de test :** « Fusion Horizon » combine quatre sons de la banque Factory Starter. Le nombre de presets de démonstration est maintenant de **10** (7 Factory Starter + 3 Expansion Demo) sans reproduire les samples.

**Qualité :** 24 tests Python validés localement avant compilation, contrôle automatique des caractères français et géométrie des quatre panneaux à 15 résolutions. Le rendu réel macOS et les tests MIDI/audio restent à effectuer dans les hôtes DAW.

**Limites de ce prototype :** maximum 64 zones multisamples chargées par combinaison, chargement WAV en mémoire, pas de streaming disque, ni éditeur de sauvegarde/export de preset multicouche vers une banque externe depuis le plugin, ni protection de licence commerciale. Les presets historiques et leurs sons doivent être retestés avant distribution.

Télécharger la dernière compilation verte depuis [GitHub Actions](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml) → **Artifacts**.

---

# Liberty Rompler — Version UI v0.5

**Fonctions ajoutées sur une base v0.4 validée :**

- Nouvel onglet **MODULATEURS** : enveloppe ADSR d'amplitude (attaque, déclin, maintien, relâchement) et enveloppe ADSR de filtre (les mêmes réglages plus profondeur bipolaire). L'enveloppe de filtre est globale, déclenchée par les notes MIDI et le clavier virtuel. Les paramètres d'amplitude s'appliquent aux nouvelles notes.
- Deux LFO avec vitesse et **quatre formes d'onde** (sinusoïdale, triangle, dent de scie, carrée), utilisables dans les 4 routages de la matrice existante.
- Onglet **EFFETS** : 6 modules en grille 3 colonnes × 2 rangées, conservant Chorus, Réverbération, Delay et Distorsion, plus un **EQ 3 bandes** (grave 140 Hz, médium 1 100 Hz, aigu 7 600 Hz, ±12 dB) et un **Compresseur** (seuil, ratio, gain de sortie ; attaque 15 ms et relâchement 130 ms fixes pour ce prototype).
- Toutes les valeurs des nouveaux modules sont incluses dans l'état de session DAW. Valeurs neutres par défaut pour préserver les anciens projets (EQ plat, compression ratio 1:1, profondeur enveloppe filtre 0).
- **Règles de qualité non négociables :** chaînes françaises UTF-8, pas de caractères de substitution, aucun chevauchement des contrôles. Des tests de géométrie sont exécutés aux tailles minimales et maximales de l'interface.

Cette version reste un **prototype** : l'audit de code et les tests de zones ne remplacent pas les inspections visuelles dans Logic Pro/Ableton, les écoutes, ni les tests de stabilité. La bibliothèque comprend toujours 9 presets de démonstration. L'installation de banques payantes sous licence n'est pas encore implémentée.

Le dépôt conserve le chemin reconstructible v0.1 → v0.2 → v0.3 → v0.4 → **v0.5**. L'archive v0.5 est vérifiée par SHA256 puis appliquée avant les tests et le build GitHub Actions.

[Télécharger la dernière compilation](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml) en ouvrant le dernier run vert, section **Artifacts**.

---

# Liberty Rompler — Effets et audit des accents (v0.4)

## Nouveautés : deux effets audio réels
- **Delay stéréo** : temps de 60 à 1500 ms, feedback 0 à 85 %, mixage sec/humide.
- **Distorsion** : gain d'entrée (Drive) de 0 à 30 dB, contrôle tonalité (500 à 18 000 Hz), mixage parallèle.
- **Quatre modules sans chevauchement** : Chorus, Réverbération, Delay, Distorsion, dans une grille 2 × 2.
- Les 6 nouveaux paramètres sont enregistrés dans les projets DAW et initialisés à MIX = 0 pour préserver les anciens sons.

## Protection renforcée des textes français
- Les textes internes C++ utilisent des séquences Unicode explicites UTF-8 ; tous les caractères français (é, è, ê, à, ç, etc.) sont pris en compte.
- `tools/audit_ui_text.py` vérifie les sources, les libellés et les manifestes de banques, détecte les suites d'octets mal interprétées (par exemple les codes `U+00C3 U+00A9`), les caractères de remplacement et les caractères non imprimables. Le build échoue avant compilation si l'audit échoue.
- 13 tests hors ligne couvrent notamment la disposition des quatre effets, la sauvegarde des paramètres et l'absence de chevauchement sur plusieurs dimensions de fenêtre.

## Télécharger
Ouvrir [GitHub Actions](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml) et télécharger l'artifact du dernier build **vert**, au format ZIP ou DMG macOS arm64.

**Prototype** : l'audit automatique vérifie le texte et la géométrie, mais les polices et le rendu final doivent être contrôlés visuellement dans Ableton ou Logic. Le Delay ne possède pas encore la synchro au tempo et la Distorsion n'utilise pas encore de suréchantillonnage.

### Source v0.4
La compilation reconstruit v0.1 → v0.2 → v0.3 → v0.4. Le patch v0.4 est dans `assets/ui-v04/part-{1..3}.b64`, contrôlé par SHA-256 avant application.

---

# Liberty Rompler — Interface v0.3

**Nouvelle interface avec quatre pages fonctionnelles : PRESETS / EFFETS / MODULATION / RÉGLAGES.**

- **PRESETS** : le navigateur v0.2 et les banques d'extensions sont conservés.
- **EFFETS** : vrai Chorus stéréo et vraie Réverbération JUCE ; chaque module possède trois paramètres audios.
- **MODULATION** : quatre routages indépendants, sources LFO 1/LFO 2/Molette MIDI (CC1), destinations filtre/volume, intensité bipolaire, vitesses des LFO.
- **RÉGLAGES** : ouvrir le dossier de banques, actualiser la bibliothèque.
- **CLAVIER MIDI** : 7 octaves C–B (84 notes), 49 touches blanches, largeur adaptée à **100 % de la fenêtre** et scroll désactivé.
- Paramètres effets et routages enregistrés/restaurés dans l'état du projet DAW ; Mix à zéro par défaut pour préserver le son v0.2.
- Contrôles de non-chevauchement de l'UI, accents français UTF-8 et géométrie du clavier intégrés aux tests Python.

Le build macOS du prototype est dans [GitHub Actions](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml), sous **Artifacts** lorsque la compilation est verte.

**Limites :** prototype (9 presets de démonstration), non notarisé et non prêt pour la vente. Les tests de géométrie ne remplacent pas un contrôle visuel dans un hôte macOS. Le contrôle des effets et des routes est une première version, pas encore une matrice professionnelle de centaines de destinations.

## Détails techniques du correctif v0.3

Les fichiers sources v0.2 sont reconstruits depuis `assets/ui-v02/`. Le workflow applique ensuite `assets/ui-v03/part-{1..4}.b64` (patch C++ compressé, SHA256 vérifié) avant les tests et la compilation. Les banques et le moteur multisample original restent intacts.

---

## Historique : interface v0.2

## Prototype UI v0.2 (macOS Apple Silicon)

Instrument ROMpler JUCE 8/C++, indépendant de PluginMaker.ai et de Liberty DAW. **L'interface est inspirée de l'organisation générale du navigateur Nexus 5, avec un design et une identité Liberty originaux.** Aucun actif graphique ni code de reFX n'est utilisé.

### Installer depuis GitHub Actions

1. Ouvrir [Actions : Liberty Rompler](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml).
2. Ouvrir la dernière compilation verte (UI v0.2).
3. Dans **Artifacts**, télécharger `Liberty-Rompler-macOS-arm64-UI-v0.2-AU-VST3-Standalone`.
4. Décompresser le téléchargement, ouvrir le DMG et utiliser le script `INSTALLER_MAC.command`, au besoin depuis le Terminal avec `bash INSTALLER_MAC.command`.

**Prototype de test : signature ad hoc, non notarisé, non destiné à la distribution commerciale.**

### Nouveautés v0.2

- **Trois colonnes** : banques/extensions, navigateur de presets, modules audio.
- Palette graphite, cyan et or ; titres, espacements et graphes redessinés.
- Recherche sur nom, catégorie et banque ; catégories horizontales défilantes.
- Favoris par preset mémorisés sur le Mac ; navigation preset précédent/suivant.
- Filtre passe-bas DSP réel avec fréquence de coupure et résonance sauvegardées avec le projet.
- Enveloppe d'amplitude et volume toujours fonctionnels.
- Interface redimensionnable (minimum 1120 × 750) ; troncature des textes trop longs.
- **Accents français encodés en Unicode explicite**, sans symboles de remplacement.
- Tests hors ligne de zones sans chevauchement à 15 tailles et de chaînes Unicode.

### Banques de démonstration

Factory Starter : **6 presets** ; Expansion Demo : **3 presets**. Les banques restent distinctes du plugin et sont installables sans recompiler l'instrument. Il ne s'agit pas encore des 100 presets professionnels ni d'extensions commerciales licenciées.

### Structure du projet

- `assets/rompler-source-v0.1.tar.xz` : socle du moteur (JUCE/CMake, sample maps, génération des banques).
- `assets/ui-v02/part-1.b64` à `part-5.b64` : patch source UTF-8 compressé (nouvel éditeur C++, filtres DSP, tests géométriques).
- `.github/workflows/build-rompler.yml` : reconstitue et vérifie les deux archives, exécute les tests, compile et produit ZIP + DMG.

Pour extraire les sources v0.2 sur macOS :
```bash
tar -xJf assets/rompler-source-v0.1.tar.xz
cat assets/ui-v02/part-{1..5}.b64 | base64 -D | tar -xJf -
```

### Critères de validation non négociables

1. **Jamais de caractères erronés à la place des lettres françaises** (é, è, ê, à, ç, etc.).
2. **Aucun chevauchement** de textes, menus, boutons et panneaux.

Les tests de géométrie préviennent les régressions évidentes, mais la validation **visuelle dans l'application sur macOS** reste nécessaire. Voir `docs/UI_RULES.md`.

### Licence

Prototype public. Copyright Liberty Music Software ; droits réservés hors autorisation explicite. JUCE est sous sa propre licence et doit être vérifié avant la vente.
