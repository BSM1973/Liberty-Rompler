# LIBERTY HYBRID WORKSTATION — Vision technique et feuille de route

**Projet** : évolution de Liberty ROMpler, sans abandonner ses banques, presets et anciens projets.
**Référence de départ** : Liberty ROMpler v0.38, CI macOS ARM64 vert le 11 novembre 2026.
**Ambition** : réunir les forces d'une station de sampling professionnel, d'un instrument de synthèse hybride modulaire et d'un environnement de création de banques et d'instruments, avec une interface musicale Liberty entièrement originale.

Ce document définit les objectifs et critères d'acceptation ; **aucune fonctionnalité simplement décrite ici n'est présumée implémentée**.

## 1. Inspirations et principes d'indépendance

- **Kontakt 8** : instruments multisamples expressifs, gestion de bibliothèques, édition profonde, traitement des boucles et outils MIDI musicaux.
- **UVI Falcon** : multiplicité de moteurs sonores, routage modulaire, modulations polyphoniques, time-stretch, granularité, effets et création de programmes hybrides.
- **Steinberg HALion** : arborescence de programmes/zones, sampling, édition sonore, resynthèse spectrale, scripts et création de pages macro. HALion 7.5 est la version de référence officielle constatée en novembre 2026 ; le nom « HALion 8 » est une ambition de gamme, pas une version vérifiée.
- **Identité Liberty** : navigation plus simple, français soigné, interface bleu nuit, aucun chevauchement, workflow non destructif, transparence des capacités.

Nous concevons nos propres moteurs et notre propre interface. Aucune promesse de compatibilité avec des formats propriétaires, bibliothèques protégées/chiffrées, presets de tiers ou technologies sous licence sans intégration/licence documentée.

## 2. État vérifié de Liberty ROMpler v0.38

### Réalisé et intégré
- Quatre couches indépendantes avec moteurs de voix et deux slots d'effets par couche.
- Enveloppes d'amplitude/filtre, deux LFO et quatre routes de modulation utilisateur.
- Arpégiateur 32 pas et six effets globaux en chaîne.
- Zone mapper, outils de boucles, points de fondu et préécoute.
- MORPH XY : quatre couches à coefficients bilinéaires, lissage, automation et MIDI CC.
- Round Robin, Keyswitch et fondus de vélocité optionnels sur les zones de banques.
- Smart Import depuis noms WAV : **outil Python hors interface**, prudence sur les noms ambigus.
- CRÉATIF : granularité expérimentale deux grains, TIME 0,5×–2×, huit tranches à durée égale, six formes d'accords simples.
- AU, VST3 et Standalone pour macOS Apple Silicon ; CI macOS v0.38 vert.

### Limitations à traiter avant toute comparaison sérieuse avec les références
- Chaque zone charge actuellement un maximum d'environ **20 secondes de WAV en RAM**.
- Limite de **32 zones par preset** dans le format/lecteur v1 actuel.
- Moteur basé uniquement sur les samples : aucune vraie famille d'oscillateurs multiples VA/wavetable/FM/additif/spectral.
- Quatre couches fixes, sans arborescence extensible de programmes/parties.
- Quatre routes de modulation, ciblage encore restreint.
- Pas encore de véritable streaming disque ni d'importation graphique intégrée.
- GRANULAR / TIME / SLICE et HARMONY sont des premières implémentations, non des équivalents des moteurs premium du marché.
- Pas de langage de script, de designer d'instrument, d'édition MIDI MPE complète ni de système mature de packaging d'extensions.
- Pas de preuve de qualité sonore « identique/supérieure » aux références sans tests d'écoute et mesures.

## 3. Architecture cible

\`\`\`text
LIBERTY PERFORMANCE (projet / multis)
  ├── 1 à 16 PARTS (cible, progressive ; canal MIDI, volume, panorama, bus)
  │     └── PROGRAM (instrument jouable)
  │           ├── LAYER / GROUP (nombre extensible, pas un empilement fixe en UI)
  │           │     ├── ZONE / OSCILLATOR
  │           │     │      SAMPLE · MULTISAMPLE · GRANULAR · SLICE
  │           │     │      ANALOG · WAVETABLE · FM · NOISE · ADDITIVE
  │           │     │      SPECTRAL / MODELISATION (phases ultérieures)
  │           │     ├── VOICE / ARTICULATION / VELOCITY / ROUND ROBIN
  │           │     ├── MODULATORS: ENV, LFO, STEP, MSEG, MACRO, MIDI, MPE
  │           │     └── FX + SENDS
  │           └── MIDI TOOLS / SCRIPT / MACRO PAGE
  ├── ROUTING BUSES / AUX FX / MASTER / STEREO
  ├── SOUNDFINDER / PACKAGES / FAVORIS / EXTENSIONS
  └── STATE HOST / PRESETS / AUTOMATION
\`\`\`

Les anciennes banques \`liberty-rompler-bank-v1\` doivent rester chargeables et produire le même son. Toute évolution du format sera versionnée, migrable, non destructive et testée sur des fixtures v0.35 à v0.38.

### Principes de moteur audio
- Aucun accès disque, allocation, destruction, mutex bloquant ou opération longue dans le callback audio.
- Les données de zones/patches/graphes sont construites en dehors du thread audio, publiées via snapshots immuables et swaps sûrs.
- Paramètres atomiques et transitions lissées ; gestion correcte des notes et note-offs, sustain, voice stealing et canaux MIDI/MPE.
- Polyphonie configurable et CPU/RAM mesurés, contrôle des erreurs disque et de la latence ; éviter des objectifs de performances impossibles à garantir.
- Distinguer **audio-rate**, **control-rate** et **event-rate** pour les modulateurs.
- Le graphe de routing doit valider les cycles et connexions avant publication ; aucun graphe invalide ne peut entrer dans l'audio.

## 4. Séquence des lots d'ingénierie

| Ordre | Lot | Livrables minimaux | Tests décisifs |
|---|---|---|---|
| A | **Streaming & fondations** | Lecteur disque préemptif hors audio, préchargement des attaques, cache borné, changement de banque sûr, prise en charge de WAV longs sans troncation | 64 voix, 4 couches, boucles et pitch bend ; stress disque lent ; RAM mesurée ; zéro E/S callback |
| B | **Sampling Studio** | Mapping zones sans plafond artificiel de 32, éditeur key/velocity graphique, vrai détecteur de transitoires, SLICE libre, release triggers, legato, pédale, smart import dans l'interface | Import/export sans perte, round robin, release trigger, chevauchements de zones, project recall |
| C | **Synth Engines** | Oscillateurs analogiques virtuels, tables d'ondes, FM, bruit puis granulaire avancé, avec enveloppes/filtre par voix | Anti-aliasing, tests FFT, tenue CPU, compatibilité avec les notes/polyphonie |
| D | **MODULATION PRO & routage** | Matrice de modulation extensible, 8 macros, modulateurs polyphoniques, MSEG, LFO additionnels, Step, enveloppes multi-étapes, bus/aux et effets modulables | Routage stable, pas de cycles, automation DAW, isolation par voix, performance |
| E | **TIME & SPECTRAL** | Time-stretch de qualité supérieure préservant transitoires et stabilité stéréo ; détection du tempo, suivi formants, analyse/résynthèse spectrale graduelle | Matériel percussif, voix, nappes, tests A/B, artefacts et décrochages |
| F | **HARMONY & PERFORMANCE** | Gammes et accords avancés, inversions, voice-leading, strumming, humanisation, arpèges et séquences, MIDI MPE | Note-off exact par origine/canal, accords qui se chevauchent, sustain, DAW sync |
| G | **LIBRARY CREATOR** | Banques d'extensions installables sans recompilation, recherche/étiquettes/favoris/préécoute, packaging ressources, export d'instruments, pages macro créateur | Installation/désinstallation, fichiers manquants, migration, sandbox de ressources |
| H | **INSTRUMENT DESIGNER** | Interfaces d'instruments personnalisables, scripts sûrs avec quotas CPU/mémoire, API publique versionnée, outils de diagnostic | Sécurité, déterminisme, sauvegarde, limites des scripts, tests de packs |
| I | **Qualité commerciale** | Banque d'usine étoffée et écoutée, presets classés, documentation et tutoriels, compatibilité macOS et Windows une fois vérifiée | QA AU/VST3/Standalone, installation, session recall, crash tests et audition |

Les lettres A–I représentent des **jalons techniques**, pas une promesse de les livrer intégralement dans neuf builds. Un jalon peut nécessiter plusieurs validations de sources mais **une seule compilation macOS par lot réellement prêt**.

## 5. Priorité immédiate — premier jalon

Commencer par **A : streaming disque + modèle de données évolutif**, car une synthèse sophistiquée sur un moteur de sample encore limité à 20 secondes et 32 zones ne répondrait pas au cahier des charges. Le travail commence par :
1. Tests reproductibles de la limite actuelle et de la compatibilité des anciennes banques.
2. Prototype de cache et de lecteur anticipé entièrement hors callback audio, activé sur option ; aucun changement du son par défaut.
3. Tests de boucles, voix multiples, stop/note-off et changements de preset en charge.
4. Intégration du streaming uniquement lorsqu'il passe les tests JUCE et macOS ; aucun affichage « streaming actif » avant.
5. À la suite, amélioration du format de banque en gardant le chargement v1 et l'éditeur non destructif.

Le plan existant \`docs/DISK_STREAMING_PLAN_V037.md\` reste valable comme contrat de sûreté.

## 6. Interface et ergonomie

La multiplication de pages dans une barre horizontale n'est pas le but. Introduire plutôt trois **espaces de travail** réorganisables :
- **JOUER** : instruments, performances, MORPH, 8 macros, browser.
- **ÉDITER** : samples, zones, boucles, oscillateurs, enveloppes, modulation, routing.
- **CRÉER** : structure de programmes, pages macro, scripts, banques/extensions.

Les panneaux seront redimensionnables et détachables uniquement après tests de géométrie ; raccourcis et comportement des onglets anciens préservés. Label UI en français accentué UTF-8 partout, zéro symbole de remplacement, zéro chevauchement à toutes les résolutions supportées. Thème bleu nuit Liberty, pas de copie de l'habillage de HALion/Falcon/Kontakt.

## 7. Conditions de sortie et méthode GitHub

- Aucune fonctionnalité affichée « terminée » sur une simple compilation de calcul C++ : le raccordement à l'audio, à l'interface, à l'automation et à la sauvegarde est obligatoire.
- Régression totale et tests d'unités de chaque fonctionnalité avant PR.
- Sur GitHub, sources sur branche de travail ; preflight Linux et C++ ; build macOS **unique** après les contrôles verts et la fusion.
- Chaque build publié est vérifié AU + VST3 + Standalone, intégrité de l'archive, fonctionnement du projet DAW, textes français et non-chevauchement.
- Séparer sans ambiguïté : \`conçu\`, \`codé\`, \`tests unitaires verts\`, \`build vert\`, \`testé musicalement dans un DAW\`.
- Conserver les licences, dépendances et droits de distribution des banques légitimes ; ne pas promettre la lecture de banques Kontakt protégées.
- Éviter toute dépense de crédits GitHub pour de simples changements de documentation ou prototypes non validés.

## 8. Définition du succès

Un créateur doit pouvoir importer légalement sa propre banque multisample, l'articuler musicalement, la jouer sans pénaliser la RAM, combiner samples et moteurs de synthèse dans un même programme, moduler librement, créer une interface de contrôle simple, enregistrer un preset, installer l'instrument chez un autre musicien et retrouver le même son dans son DAW.

Ce résultat vise **une alternative Liberty originale et professionnelle**. Aucune égalité de qualité ou de fonctions avec Kontakt 8, Falcon ou HALion 7.5 n'est affirmée avant des comparaisons mesurées et des tests d'écoute.
