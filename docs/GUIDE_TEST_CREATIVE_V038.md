# Liberty ROMpler v0.38 — Guide de vérification macOS

Tester Standalone, AU et VST3 dans plusieurs hôtes après le build.

1. Ouvrir un ancien projet v0.37 : GRANULAR, SLICE, HARMONY doivent être OFF et TIME à 1×. Vérifier le son d'origine, MORPH, Round Robin, articulations et les 10 anciens onglets.
2. Ouvrir **CRÉATIF**, 11e onglet. Vérifier tous les contrôles sans chevauchement en 1120×750, 1360×850 et 1920×1120, notamment les accents CRÉATIF, À et é.
3. Avec une note longue, activer GRANULAR. Tester taille des grains 30, 90 et 240 ms. Tester TIME 0,5×, 1× et 2× ; vérifier que les changements de vitesse ne changent pas la hauteur de la note. Noter les artefacts audio potentiels sur les percussions.
4. Avec GRANULAR OFF, vérifier que TIME ne modifie pas la vitesse. Avec GRANULAR ON à TIME 1×, comparer au mode normal et surveiller les limites de boucles.
5. Activer SLICE sur une banque contenant un WAV exploitable. Jouer MIDI 36 à 43 : les huit tranches successives doivent s'entendre, un sample par couche. Les notes en dehors de cette plage doivent rester silencieuses ; SLICE OFF restaure le clavier normal.
6. Jouer MIDI 60 en HARMONY majeur : vérifier C-E-G (60,64,67). En mineur 60,63,67 ; sus2 60,62,67 ; majeur7 60,64,67,71. Relâcher la touche et vérifier la fin des notes générées ; changer le mode pendant les notes tenues et tester les contrôles all-notes-off.
7. Vérifier HARMONY avec l'ARP et le MORPH à quatre couches, et la consommation CPU pour de nombreux accords ; ne pas confondre harmonisation MIDI et analyse d'accords en audio.
8. Automatiser chacun des cinq paramètres dans le DAW, sauvegarder, fermer et restaurer un projet. Vérifier la stabilité et les valeurs par défaut des anciens projets.
9. Aucun WAV, preset ni manifeste ne doit être modifié. Vérifier absence de lettres françaises corrompues ou de chevauchement.
10. Le streaming disque pour WAV très longs n'est pas implémenté ; la limite de 20 s par zone reste applicable.

La compilation verte ne remplace pas l'écoute manuelle ni un test d'endurance et de polyphonie.
