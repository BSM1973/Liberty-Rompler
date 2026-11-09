# Liberty Rompler — Liberty Music Software

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
