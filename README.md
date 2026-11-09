# Liberty Rompler — Liberty Music Software

**Prototype technique v0.1 · GitHub Actions macOS Apple Silicon**

Instrument multisample original en **JUCE 8/C++**, indépendant de PluginMaker.ai et du dépôt Liberty DAW.

## Télécharger une compilation

1. Ouvrir [GitHub Actions — Liberty Rompler](https://github.com/BSM1973/Liberty-Rompler/actions/workflows/build-rompler.yml).
2. Ouvrir le run **Liberty Rompler - Build macOS AU VST3 Standalone**.
3. Quand le run est **vert**, descendre à **Artifacts** et télécharger **Liberty-Rompler-macOS-arm64-AU-VST3-Standalone**.
4. Décompresser l'archive du run. Elle contient un ZIP et un DMG ; les deux contiennent le même prototype.
5. Dans le dossier fourni, exécuter `INSTALLER_MAC.command` depuis le Terminal avec `bash INSTALLER_MAC.command`. L'installateur copie l'app, les plugins et les banques dans les dossiers utilisateur.

**Ce prototype n'est pas signé Developer ID ni notarisé.** macOS peut avertir ou bloquer son lancement. Il ne s'agit pas encore d'un installateur commercial.

## Capacités actuelles

- Lecture multisample WAV polyphonique ; mapping notes/vélocité, boucles, ADSR.
- Navigateur de presets et de banques, recherche et filtres.
- 2 banques : Factory Starter (6 presets) et Expansion Demo (3 presets).
- Extension séparée des fichiers du plugin ; la protection commerciale/licence reste à développer.
- Cibles de compilation : **AU, VST3 et application autonome macOS arm64**.

## Source et banques

Le code C++ et les utilitaires Python sont conservés dans `assets/rompler-source-v0.1.tar.xz` ; le workflow vérifie son SHA256 et l'extrait avant la compilation. Les échantillons **originaux de démonstration** sont générés pendant le build par `tools/make_demo_banks.py`, vérifiés avec les tests Python, puis livrés avec les binaires.

Pour reproduire en local :

```bash
tar -xJf assets/rompler-source-v0.1.tar.xz
python3 -m pip install numpy
python3 tools/make_demo_banks.py
python3 tools/validate_banks.py
python3 -m unittest discover -s tests -v
```

Pour compiler, il faut JUCE 8.0.10, CMake >=3.24 et Xcode. Le workflow les utilise automatiquement sur GitHub Actions.

## Limites de cette v0.1

**Ce n'est ni un équivalent de Nexus 5, ni une bibliothèque premium de 100 presets.** Il manque notamment le streaming disque, les 100 sons définitifs, les banques payantes activables sous licence et les validations DAW. Le code et les banques sont publics ; ne pas y placer de futurs samples commerciaux.

## Licence

Prototype public à finalité de développement. Les droits sur le code, les sons et le nom Liberty sont réservés à Liberty Music Software. JUCE, utilisé par le workflow, possède sa propre licence ; vérifier les conditions de redistribution avant commercialisation.
