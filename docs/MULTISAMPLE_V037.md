# Liberty ROMpler v0.37 — Multisampling professionnel (étape 1 du Lot 2)

L'étape 1 ajoute trois fonctions **réellement connectées au moteur JUCE** et un utilitaire d'import non destructif :

1. **Round Robin** jusqu'à 16 prises par groupe de zone et par note MIDI. L'alternance est déterministe et saute les prises absentes de la zone MIDI/vélocité actuelle. Les groupes restent indépendants par note.
2. **Articulations/Keyswitch** : note MIDI de sélection ; les zones avec `articulation=0` jouent quelle que soit l'articulation, les zones spécifiques uniquement lorsque leur articulation est active. Articulation par défaut 1 ; la valeur active est conservée dans le projet DAW.
3. **Velocity Crossfade** : largeur `velocity_fade` en pas MIDI 0..32, mélanges linéaires entre les zones voisines, sans effet si valeur absente ou égale à 0. Le fondu intervient avant la lecture de la voix, dans le gain d'attaque.
4. **Smart Import v0.37** : `python3 tools/smart_import_v037.py --bank-folder Banks/MaBanque --bank-id ma_banque --bank-name "Ma Banque"`. Par défaut, lecture seule avec rapport. Ajouter `--write` après validation manuelle pour créer `manifest.json`. Le programme utilise les notes C4/F#3/N060, les suffixes `V001-063`, `RR01`, `ARTSUSTAIN`, `ARTSTACCATO`, `KS24`. Les suffixes L/S/H ou S01 non documentés, ainsi qu'un nom de note ambigu, provoquent un refus explicite ; aucune zone inventée. Aucun WAV n'est modifié.

Le format de banque `liberty-rompler-bank-v1` reste inchangé et les nouveaux attributs sont **facultatifs** :
```json
{
  "file": "Piano_C4_V001-063_RR01_ARTSUSTAIN_KS24.wav",
  "root_note": 60, "low_note": 0, "high_note": 127,
  "low_velocity": 1, "high_velocity": 63,
  "loop_start": -1, "loop_end": -1, "gain": 1.0,
  "rr_group": 1, "rr_index": 1,
  "articulation": 1, "keyswitch_note": 24, "velocity_fade": 8
}
```

**Limite importante : le streaming disque n'est pas encore actif.** L'ancien moteur garde son plafond de 20 secondes par zone. Smart Import rejette explicitement les WAV plus longs plutôt que de les tronquer silencieusement. L'architecture de streaming envisagée est décrite dans `docs/DISK_STREAMING_PLAN_V037.md`, séparément de cette livraison.

Règles Liberty : aucune modification destructive des WAV, des banques commerciales existantes ou des presets ; anciens projets réouvrables ; aucun chevauchement de l'interface ; accents français corrects ; tests hors temps réel et en hôte avant diffusion.

Smart Import v0.37 est pour l'instant un **outil Python autonome fourni avec les sources et le paquet macOS**, pas encore un bouton d'importation intégré à l'interface ROMpler. Son interface intégrée fera l'objet d'une étape ultérieure du Lot 2. Le mode `--replace` conserve une copie de l'ancien manifeste sous `manifest.json.liberty-backup`.
