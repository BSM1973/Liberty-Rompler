# Liberty ROMpler v0.32 — Test du pourcentage de fondu sur Mac

À tester dans **SAMPLES > BOUCLES** après construction de la v0.32.

1. Charger un WAV 48 kHz. Régler DÉBUT à 24000 frames et FIN à 48000 frames ; la boucle dure 24000 frames soit 500.000 ms.
2. Régler le fondu à 2400 frames (50.000 ms). Survoler **Boucle activée** : l'infobulle doit indiquer **10,00 %**.
3. Mettre le fondu à 0 : l'infobulle doit indiquer **0,00 %**. Vérifier un fondu de 1200 frames : **5,00 %**.
4. Déplacer DÉBUT et FIN ensemble (Maj + glisser) : le pourcentage doit rester identique si la durée et le fondu restent constants.
5. Modifier un seul marqueur, puis le fondu : le pourcentage doit suivre la nouvelle durée, calculée exactement comme **FIN moins DÉBUT**.
6. Remplacer le WAV par un autre à 44,1 kHz : le pourcentage reste fondé sur les frames ; les millisecondes utilisent la fréquence du nouveau WAV.
7. Survoler **ZÉRO DÉBUT** et **ZÉRO FIN** : les positions en frames et ms ajoutées en v0.31 doivent apparaître sans effacer les raccourcis.
8. Désactiver la boucle : aucun pourcentage ni position active fictive ne doit être présent. Changer de sample : aucune valeur ancienne ne doit subsister.
9. Vérifier ZÉRO, SAUT MIN, PAIRE, zoom de précision, historique de cadrage, préécoute A/B et ANNULER/APPLIQUER.

Règle Liberty n° 1 : **aucun chevauchement**, y compris sur Mac 13 pouces. Règle n° 2 : **accents français intacts** : é, è, ê, à, ç, É. Les infobulles doivent être lisibles.

Le survol et la mesure ne doivent modifier ni les marqueurs, ni le fondu, ni un fichier WAV, ni l'état NON APPLIQUÉ.
