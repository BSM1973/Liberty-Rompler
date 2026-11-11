# Liberty ROMpler v0.38 — Lot 3 : outils CRÉATIF

Le moteur v0.38 s'ajoute au MORPH à quatre couches de v0.36 et conserve les innovations du multisampling v0.37. Tous ses nouveaux modes sont **désactivés par défaut**, pour garantir que les projets historiques conservent leur comportement tant qu'ils ne sont pas activés.

## GRANULAR et TIME ENGINE

**GRANULAR ON** active un premier moteur de resynthèse audio constitué de deux grains avec fenêtres Hann complémentaires et recouvrement de 50 %. Taille des grains réglable de **30 à 240 ms**. La vitesse TIME de **0,5× à 2×** déplace la tête de lecture indépendamment de la hauteur MIDI grâce au moteur granulaire. TIME ne modifie pas le son quand GRANULAR est désactivé.

Limite : il s'agit d'un **premier étirement granulaire expérimental**, pas d'un algorithme de haute qualité optimisé pour préserver les transitoires des percussions ou les formants des voix. Les résultats extrêmes peuvent comporter des artefacts. Les fichiers utilisent encore la limite RAM historique des 20 s par zone ; aucun streaming disque longue durée n'est revendiqué.

## SLICE — huit segments jouables

**SLICE ON** divise le premier WAV de chaque couche en **huit tranches égales**, non destructives, déclenchées par les notes **MIDI 36 à 43**. Une courte atténuation en fin de tranche limite les clics. Le clavier multisample d'origine revient avec SLICE OFF. Ce premier SLICE ne propose **pas encore de détection des transitoires ni de placement graphique des divisions** ; ces étapes restent planifiées.

## HARMONY — accords à une touche

Six modes : majeur (0,4,7), mineur (0,3,7), sus2 (0,2,7), sus4 (0,5,7), majeur7 (0,4,7,11), mineur7 (0,3,7,10). HARMONY OFF conserve les notes originales. Les notes ajoutées sont générées **en interne avant l'arpégiateur**, avec suivi des note-offs par canal et note MIDI. Les notes générées ne sont pas envoyées comme sortie MIDI à d'autres instruments. Les accords joués avec plusieurs touches et des notes partagées nécessitent une validation audio : une libération commune de certaines voix reste possible. Inversions, reconnaissance de tonalité, voice-leading et strumming ne sont **pas encore inclus**.

## Interface, hôtes et compatibilité

Un nouvel onglet **CRÉATIF** (11e onglet) regroupe les contrôles, sans superposer les anciens. Les cinq paramètres hôtes automatisables sont **Liberty GRANULAR Active**, **Liberty GRANULAR Grain ms**, **Liberty TIME Speed**, **Liberty SLICE Active**, **Liberty HARMONY Mode**. Les états sont enregistrés en propriétés facultatives dans la session DAW ; les anciens projets n'activent aucune nouvelle fonction. Les WAV, manifestes et presets ne sont jamais modifiés.

**Sources exactes** : les patches sont conservés en sept fragments base64 dans `assets/v038`, vérifiés SHA-256 avant installation par `scripts/apply_v038_bundle.py`. Le build reconstitue les fichiers JUCE C++ éditables dans `Source/`, et le paquet contient les sources ainsi reconstituées et les tests.

**Tests** : `tests/creative_v038_test.cpp`, `tests/test_v038_creative.py` et toute la suite de non-régression. Toute compilation macOS doit suivre la prévalidation du patch sur GitHub.

Règles Liberty : **zéro chevauchement**, **accents français corrects**, **anciens projets compatibles**, **aucun accès disque dans le callback audio**.
