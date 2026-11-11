# Liberty ROMpler v0.34 — Guide de validation de l'infobulle BOUCLES

Tester dans SAMPLES > BOUCLES, après installation du Standalone, et dans un DAW en AU/VST3.

1. Charger un WAV de 96000 frames ; placer DÉBUT à 24000 et FIN à 48000 ; activer la boucle. Au survol de « Boucle activée », vérifier PART DU WAV 25,00 %, AVANT LA BOUCLE 25,00 % et APRÈS LA BOUCLE 50,00 %.
2. Avec DÉBUT à 0 et FIN à 96000, vérifier 0,00 % avant, 100,00 % dans la boucle, 0,00 % après.
3. Déplacer les deux marqueurs sans changer leur écart (DÉBUT 48000, FIN 72000) : 50,00 % avant, 25,00 % dans, 25,00 % après.
4. Vérifier les valeurs après déplacement d'un seul marqueur, changement de WAV, désactivation et réactivation de la boucle.
5. Vérifier qu'un WAV invalide ou une boucle inactive ne donne pas d'anciens pourcentages.
6. Vérifier la persistance des infos v0.30–v0.33 : frames, millisecondes, pourcentage du fondu et PART DU WAV. Vérifier les infobulles ZÉRO DÉBUT/FIN.
7. Vérifier les raccourcis : ZÉRO, SAUT MIN, PAIRE, centrage Commande, zoom Commande + Option, molette, glisser, historique Maj + double-clic, préécoute A/B.
8. Vérifier qu'aucun bouton, label ni texte visible n'est ajouté. Vérifier les accents d'APRÈS, DÉBUT, ZÉRO, DURÉE et l'absence totale de chevauchement.
9. Vérifier que le WAV original, les banques, les presets et les projets sont inchangés.

Note : les pourcentages sont calculés sur les frames. Une fois affichées à deux décimales, leurs valeurs peuvent différer de 100,00 % au centième près en raison des arrondis.
