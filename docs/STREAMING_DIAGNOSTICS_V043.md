# Liberty ROMpler v0.43 — Diagnostic visuel du streaming

La v0.43 est un lot d'observabilité **en lecture seule** construit sur le moteur v0.42 : aucun effet DSP, format de banque ou paramètre de session supplémentaire. Dans **RÉGLAGES**, le panneau « STREAMING DISQUE / DIAGNOSTICS » affiche quatre fois par seconde les compteurs cumulés des zones WAV longues du preset courant (les quatre couches réunies). Le bouton « COPIER LE RAPPORT » produit un texte à joindre aux retours de test. Les compteurs repartent de zéro au changement de zones ; ils ne mesurent pas automatiquement la CPU, la latence du disque ni les voix actives.

**Signaux présentés :** nombre de zones longues, succès/échecs de cache, taux du cache, pages publiées/perdues, demandes urgentes, urgentes servies/perdues, erreurs de décodage et blocs audio sous-alimentés. La lecture des compteurs est effectuée depuis un timer côté interface, pas depuis le callback audio. Le volet RÉGLAGES reste une des onze pages existantes : aucun douzième onglet, pas de modification de la navigation.

**Limites :** le panneau signale des incidents historiques depuis le chargement, pas forcément un incident qui se produit à l'instant. Il ne constitue pas un test d'écoute. Une compilation AU/VST3/Standalone et un essai musical macOS restent nécessaires. Le total peut changer lors des changements de preset ; ne pas comparer des rapports provenant de banques différentes.

## Banque artificielle reproductible

La commande python3 tools/generate_streaming_stress_v043.py --output /chemin/vers/Liberty-Stress-043 crée une banque v1 originale avec quatre sous-presets de 16 zones WAV longues stéréo (24 s par défaut), plus un preset « Stress streaming 4 couches ». Les 64 zones totalisent environ 271 Mo à 44,1 kHz/16 bits/stéréo. Cette génération peut prendre du temps ; elle n'altère aucune banque commerciale. Installer la banque dans le dossier de banques Liberty puis actualiser l'index. Aucun fichier n'est inclus automatiquement dans l'application.

Chaque couche réagit aux notes MIDI 48–63. Jouer ces 16 notes simultanément déclenche jusqu'à 64 voix (quatre couches) et permet d'éprouver le pool de workers. Comparer aussi la même banque sur SSD rapide, stockage plus lent et hôte AU/VST3. Tester boucles/crossfades, SLICE éloigné, changement de preset, long sustain et 30 minutes de lecture ; consigner les interruptions réellement entendues en regard des compteurs. Ne pas confondre validation automatique du générateur et essai réel du moteur JUCE.

Les deux règles UI obligatoires sont conservées : accents français corrects et aucun chevauchement à 1120×750, 1360×850 et 1920×1120. Inspection visuelle native macOS avant livraison.
