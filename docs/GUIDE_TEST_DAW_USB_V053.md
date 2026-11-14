# Liberty ROMpler v0.53 — Guide des essais audio réels

**À réaliser sur Mac Apple Silicon.** GitHub Actions ne dispose ni de la PreSonus Studio 1824C, ni d'un périphérique CoreAudio de production, ni d'un support USB externe lent réel.

## Préparation
Conserver une copie du projet et la v0.52 avant l'installation. Utiliser une banque de 64 zones WAV longues avec boucles et 8 tranches SLICE. Noter la version exacte de macOS et du DAW, l'interface/pilote et le support disque. Sélectionner la PreSonus Studio 1824C sous CoreAudio et synchroniser la fréquence de la session.

## Procédure
1. Tester Ableton Live (AU ou VST3), Logic Pro (AU) et Bitwig Studio (VST3). Commencer à 44,1 kHz / 512 frames, puis 256, 128 et 64 ; répéter à 48 kHz.
2. Tenir 64 voix pendant 23 secondes. Écouter les clics et relever les compteurs underflow/cache sous RÉGLAGES.
3. Passer en SLICE, jouer MIDI 36–43, une tranche toutes les 0,5 s, pendant 60 secondes. Tester la vitesse TIME x0,5 et x2, avec GRANULAR désactivé puis actif.
4. Alterner 1 et 4 couches toutes les secondes pendant 60 secondes. Vérifier la RAM dans Moniteur d'activité, l'absence de blocage et la persistance après sauvegarde/réouverture.
5. Refaire les mêmes essais sur SSD interne puis sur **une copie identique** des WAV placée sur un disque USB physique plus lent. Noter modèle/bus/système de fichiers et bande passante observée.
6. Exporter ou enregistrer les sorties stéréo et noter le timestamp de chaque anomalie audible. Les compteurs de streaming à zéro n'excluent pas les décrochages CoreAudio.

## Fiche de résultat
DAW/version, format AU/VST3, macOS, interface, pilote, 44,1/48 kHz, buffer, support disque, banque et zones, durée, modes activés, CPU moyen/pic, mémoire, underflow/cache, clics/notes perdues et horodatage. En cas de problème, joindre un bref extrait audio et les journaux non sensibles à [l'issue de fiabilité](https://github.com/BSM1973/Liberty-Rompler/issues/25).

Ce document est un **protocole**, pas un rapport de validation déjà effectuée.
