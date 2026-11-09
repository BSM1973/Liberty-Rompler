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
