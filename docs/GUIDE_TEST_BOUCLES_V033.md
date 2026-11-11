# Liberty ROMpler v0.33 — Contrôle de couverture du WAV sur macOS

À exécuter dans **SAMPLES > BOUCLES**, une fois le build v0.33 installé en Standalone, puis si possible en AU/VST3.

1. Charger un WAV de 96000 frames. Définir DÉBUT = 24000 et FIN = 48000. Survoler **Boucle activée** : l'infobulle doit présenter **PART DU WAV 25,00 %**.
2. Mettre DÉBUT = 0 et FIN = 96000. La part du WAV doit afficher **100,00 %**.
3. Tester une boucle de longueur constante mais déplacée, par exemple DÉBUT = 48000 et FIN = 72000 : la mesure doit toujours indiquer **25,00 %**.
4. Modifier seulement FIN ou DÉBUT : la couverture doit suivre exactement le changement de nombre de frames.
5. Tester une boucle terminant sur l'extrémité exclusive du fichier : le pourcentage doit être correct sans déborder du WAV.
6. Remplacer par un WAV à 44,1 kHz, puis un WAV à 96 kHz : pour une même proportion de frames, le pourcentage reste identique ; les temps en ms des versions précédentes continuent d'utiliser la fréquence réelle du fichier.
7. Désactiver la boucle ou changer de zone : aucune mesure de l'ancienne boucle ne doit persister.
8. Vérifier les autres informations dans l'infobulle : frames/ms de boucle, frames/ms de FONDU, pourcentage du FONDU, ainsi que les positions DÉBUT/FIN dans les infobulles ZÉRO.
9. Contrôler les raccourcis historiques, la préécoute, le retour au cadrage, le zoom de précision, ZÉRO, SAUT MIN, PAIRE et ANNULER/APPLIQUER.

**Règle 1 : aucun chevauchement** graphique, notamment sur écran Mac 13 pouces.

**Règle 2 : aucun caractère erroné** à la place des accents français é, è, ê, à, ç et É.

Survoler l'infobulle ne doit jamais activer NON APPLIQUÉ ni modifier le WAV, les marqueurs ou le fondu.
