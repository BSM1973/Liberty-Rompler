# Test macOS v0.42 - Streaming prioritaire

1. Charger un WAV stereo de 3 a 5 minutes, l'ecouter dans Ableton Live, Logic ou Bitwig, de la premiere a la derniere note.
2. Tester plusieurs SLICE sur des positions eloignees dans le WAV ; les nouvelles notes doivent utiliser en priorite les pages de leur position reelle.
3. Sur une boucle avec crossfade, tester des attaques pres de la fin et verifier le retour sur le debut sans clic ni silence.
4. Monter 4 couches de 16 zones longues, alterner rapidement des notes, des veloci tes et des articulations, en restant sous 64 zones. Verifier l'absence de blocages audio.
5. Utiliser un SSD et un support de stockage plus lent ; relever les interruptions audibles, la charge CPU et le comportement des fondus anti-underflow.
6. Tester le rappel d'anciennes sessions v0.38-v0.41 ; verifier les 11 onglets, la taille de l'interface, l'orthographe et les accents francais.
7. Comparer les fichiers de test et la consommation RAM a la v0.41. Le streaming garde ses limites d'edition graphique et de zones par preset.

Les compteurs v0.42 sont disponibles dans l'API interne, pas encore affiches dans l'interface. Aucun test d'ecoute reussi ne doit etre presume avant execution reelle.
