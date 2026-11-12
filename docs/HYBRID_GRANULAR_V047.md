# Liberty Hybrid Engine v0.47 — Granulaire individuel et correction JUCE

## Nouvelle fonctionnalité reliée au moteur audio

Dans **COUCHES**, chacune des quatre couches dispose d'un interrupteur GRANULAR et d'un dosage 0–100 % entre sa lecture de sample et deux grains fenêtrés, réutilisant le moteur expérimental v0.38. L'automation du dosage est lissée progressivement. Le résultat passe toujours dans les effets de couche, MORPH et les effets master.

Nouveaux paramètres DAW stables : `libertyLayerGrain0..3` et `libertyLayerGrainWet0..3`. Ils sont ajoutés sans modifier les quatre valeurs originales du sélecteur `libertyHybridMode0..3`, pour préserver les automations Analog / Wavetable / FM enregistrées.

Les huit réglages sont conservés dans l'état DAW par couche. Les projets précédents ne contiennent pas ces champs, donc les interrupteurs sont initialisés OFF. Les WAV et manifestes de banques restent intacts.

### Limitation importante du premier lot

Le traitement granulaire **par couche** s'applique aux samples **courts en RAM**. Pour les WAV longs streamés, le nouveau contrôle laisse la lecture sèche : un vrai granular multi-page sur disque doit d'abord disposer d'une stratégie sûre de prélecture, sans provoquer davantage d'underflows. Le GRANULAR global déjà présent dans l'onglet CRÉATIF n'est pas modifié.

Les paramètres de durée des grains et vitesse proviennent encore de l'onglet CRÉATIF. Ce n'est pas encore un moteur Falcon à densité, diffusion, fenêtres personnalisables, grains aléatoires et traitement spectral complet.

## Réparation v0.46

Le banc natif hybride de la version précédente échouait à compiler avec JUCE 8 à cause d'un appel à `AudioParameterChoice::setValue`, qui est privé. La v0.47 le remplace par l'interface publique `juce::AudioProcessorParameter::setValue`. Cette réparation concerne le banc de test, pas les oscillateurs analogique/wavetable/FM.

## Vérifications

- Tests unitaires C++17 : dry/wet exact, valeurs invalides, automation lissée.
- Régressions : anciennes banques et sessions, audio streamé identique si option OFF, français et absence d'intersection des nouvelles commandes de carte.
- Banc JUCE macOS : la banque d'usine `factory-starter` (preset `aurora-pad`) est jouée à 44,1 kHz, et un rapport JSON vérifie la différence entre son sec et son granulaire, l'énergie audio, les échantillons finis et le rappel de projet.
- Banc hybride v0.46 réparé et compilations AU/VST3/Standalone.
- Une écoute dans les DAW et sur plusieurs fréquences d'échantillonnage reste indispensable : CI vert n'est pas une certification musicale.

## Étape suivante

Synthèse **spectrale** : analyse FFT hors thread audio, données spectrales immuables, resynthèse bornée CPU, mesures anti-artefacts et contrôle des transitoires. Rien de cela n'est annoncé comme livré dans la v0.47.

Règles Liberty : thème bleu nuit, aucun douzième onglet, aucun chevauchement, UTF-8 français correct, pas de régression des projets ni modification des samples originaux.
