# Liberty ROMpler v0.37 — Guide macOS de vérification du multisampling

- Préparer une banque sans écraser les WAV : `Piano_C4_V001-063_RR01_ARTSUSTAIN_KS24.wav`, `Piano_C4_V001-063_RR02_ARTSUSTAIN_KS24.wav`, etc.
- Exécuter Smart Import sans `--write` : vérifier le rapport et que les fichiers n'ont pas changé. Vérifier le JSON après utilisation volontaire de `--write`.
- Charger la banque dans le ROMpler, jouer dix fois MIDI 60 : les prises RR01 et RR02 doivent alterner. Vérifier notes adjacentes et plages de vélocité.
- Vérifier `velocity_fade=0` (ancienne sélection brutale) puis `velocity_fade=8` (transition), sans clic ni variations inattendues.
- Envoyer le keyswitch MIDI 24 et vérifier SUSTAIN, puis MIDI 25 pour STACCATO sur une banque préparée ; les notes keyswitch ne doivent pas jouer de sample.
- Vérifier que la sélection d'articulation suit le rappel du projet et que l'ouverture d'un ancien projet laisse les fonctions neutres.
- Tester MORPH v0.36 avec les quatre couches et les effets ; vérifier le maintien du Round Robin, l'absence de blocage en audio, l'arpégiateur et les commandes SAMPLES.
- Tester une banque avec des WAV >20 s : l'import doit les refuser explicitement. Ne pas annoncer de streaming disque actif.
- Vérifier les 10 onglets, les accents français, l'absence de chevauchement et les formats Standalone, AU et VST3.

Attention : le Round Robin et les articulations nécessitent des banques préparées avec les métadonnées optionnelles ; les banques précédentes restent inchangées.
