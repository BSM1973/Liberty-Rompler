# Liberty ROMpler v0.45 — Sampling Studio (Lot B1)

## Implémenté en sources

**Capacité :** 128 zones WAV maximum par preset source, 128 zones actives maximum dans les quatre couches combinées, au lieu de 32 par source et 64 au total. Le pool partage toujours quatre workers ; ses 256 inscriptions maximales (64 par worker) permettent de préparer 128 nouvelles zones pendant qu'un instrument de 128 zones est encore chargé. Ce plafond global est partagé par toutes les instances du module, et une saturation doit être refusée proprement. Il ne s'agit pas d'une promesse de 128 voix de polyphonie.

**Interface SAMPLES :** véritable carte Key/Velocity à deux axes (notes MIDI 0–127 horizontalement, vélocité 1–127 verticalement), rectangles colorés correspondant aux zones, sélection, modification par glisser des quatre frontières et quatre réglages numériques déjà présents. L'éditeur de boucles reste séparé dans le même onglet ; pas de douzième page. À petite taille, le menu de performance est aussi accessible par clic droit sur une zone afin d'éviter les chevauchements.

**Performance :** menu permettant de modifier les groupes/prises Round Robin (0–16), articulations (0–16), note keyswitch (0–127 ou aucune) et fondu de vélocité. Ces propriétés sont injectées dans les véritables `MappedSampleSound` par les overlays de projet, sans changer le moteur DSP existant. Les nouvelles données sont enregistrées dans l'état du DAW. Les sessions plus anciennes, sans ces champs, conservent les propriétés des banques originales.

**Importation :** Smart Import accepte désormais 128 zones dans un preset au lieu de les séparer arbitrairement par groupes de 32. Ses protections contre les suffixes ambigus, fichiers WAV invalides et chemins dangereux sont conservées. Le bouton **IMPORTER UNE BANQUE** dans SAMPLES permet de sélectionner un dossier contenant une banque Liberty v1 avec `manifest.json` et des WAV associés. La copie est non destructive, les collisions d'identifiants sont refusées ; les fichiers source ne sont pas modifiés et les données du projet restent indépendantes. Ce bouton **ne convertit pas encore automatiquement un dossier de WAV sans manifest** : pour cela, utiliser l'outil Smart Import fourni séparément avec les sources.

**Limite d'ergonomie à traiter en B2 :** la copie d'une grosse banque par le bouton s'exécute sur le thread de l'interface. Il faut une copie asynchrone avec progression et annulation avant de présenter l'importation de très grosses bibliothèques comme finalisée.

## Validations de sécurité

- Appliquer le patch source vérifié par SHA-256 `assets/v045/source-patch.b64` via `scripts/apply_v045_bundle.py` **après v0.44** ; aucune republication automatique des sources transformées.
- Tests du mapping C++17, 128 zones acceptées / 129 refusées, importation de 40 WAV en un seul preset, conservation des WAV, compatibilité des états anciens.
- Contrôler le format `liberty-rompler-bank-v1`, AU/VST3/Standalone macOS, les onze pages, l'orthographe française et la géométrie sans chevauchement.
- Test musical manuel : 128 zones, 4 couches, accents et Round Robin, keyswitch, velocity fade et rappel du DAW. Ces tests **ne peuvent pas être déclarés réussis par les tests unitaires**.
- Mesures v0.44 non qualifiées à protéger : 3 callbacks hors budget au test 64 voix, 1687 blocs sous-alimentés lors de SLICE. Le jalon A Streaming reste ouvert.

## Reproduire l'intégration

`python3 scripts/apply_v045_bundle.py` sur la source JUCE reconstruite et intégrée v0.44, puis `python3 -m unittest discover -s tests -p 'test_*.py'`.

La banque v1 antérieure reste lisible : les champs RR/Art supplémentaires dans les presets et l'état du DAW sont entièrement facultatifs.
