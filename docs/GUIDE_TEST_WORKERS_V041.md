# Liberty ROMpler v0.41 - Test du streaming mutualise sur macOS

Utiliser exclusivement des banques de tests autorisees ou personnelles.

1. Installer le paquet AU/VST3/Standalone d'une compilation macOS **verte**, puis ouvrir la v0.41 dans Ableton Live, Logic ou Bitwig, avec une session sauvegardee sous un nouveau nom.
2. Charger un WAV mono de 1 min puis stereo de 5 min. Jouer la note du debut a la fin, sans coupure ni saut ; tester un changement de hauteur et le pitch bend. Ne pas confondre la compilation avec un test d'ecoute valide.
3. Construire une banque avec 4 couches et 16 zones longues par couche (64 au total) : Round Robin, Keyswitch, SLICE et MORPH. Tester polyphonie, automations, boucles et moteur GRANULAR, note-on/off.
4. Observer CPU, memoire et nombre de threads au chargement. Il doit y avoir quatre workers mutualises pour le streaming, pas 64 threads de decodage.
5. Changer de preset en pleine lecture, plusieurs fois. Fermer puis reouvrir l'instance. Aucune fuite de memoire, blocage, plantage ou lecteur encore actif ne doit apparaitre.
6. Tester sur SSD puis disque externe plus lent. En cas de manque de pages, le signal doit s'attenuer sans clic sec. Mesurer les cache misses (la metrologie GUI reste a developper).
7. Tester un ancien projet v0.38 avec des banques courtes, puis comparer le rendu a la v0.40. Verifier l'affichage des accents francais, les onze onglets et l'absence de chevauchement.
8. Limites connues : le format v1 est encore plafonne a 32 zones par preset ; le chargement graphique des grandes formes d'onde n'est pas complet ; ne pas revendiquer une prise en charge universelle de toutes les grosses banques.

Cette feuille de test constitue un protocole d'acceptation. Les controles manuels doivent etre effectivement executes avant de qualifier la v0.41 de streaming professionnel.
