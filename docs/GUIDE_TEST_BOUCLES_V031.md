# Liberty ROMpler v0.31 — Test macOS des positions DÉBUT / FIN

À réaliser après installation du build v0.31, dans **SAMPLES > BOUCLES**.

1. Charger un WAV à **48 kHz** et activer une boucle avec DÉBUT = 24000 et FIN = 48000.
2. Survoler le bouton **ZÉRO DÉBUT** : l'infobulle doit indiquer DÉBUT 24000 frames et **500.000 ms** depuis le début du WAV.
3. Survoler **ZÉRO FIN** : l'infobulle doit indiquer FIN 48000 frames et **1000.000 ms** depuis le début du WAV.
4. Modifier DÉBUT et FIN à la souris puis avec les boutons ±1/±10 : les valeurs doivent être mises à jour, sans conserver celles d'une ancienne zone.
5. Charger un WAV à **44,1 kHz** et tester FIN à 44100 frames : la valeur est **1000.000 ms**. Tester si possible un WAV à 96 ou 192 kHz.
6. Désactiver la boucle : les valeurs de marqueur ne doivent pas être présentées comme des positions actives. Changer de sample : aucune valeur de l'ancien WAV ne doit rester.
7. Contrôler la ligne de durée de boucle (v0.30) et l'infobulle de durée du fondu : les valeurs et unités restent cohérentes.
8. Tester les gestes historiques sur les boutons ZÉRO : clic, Option + clic, Maj + Option + clic, Commande + clic et Commande + Option + clic. Les infobulles doivent conserver leur aide.
9. Vérifier que le nouveau survol seul **ne modifie ni DÉBUT, ni FIN, ni FONDU**, n'active pas NON APPLIQUÉ et ne réécrit aucun WAV.

Règle 1 : **aucun chevauchement** de textes, boutons ou menus sur un écran Mac de 13 pouces. Règle 2 : **accents français lisibles** : é, è, ê, à, ç et É.

Contrôler dans l'application Standalone puis dans un DAW hôte AU/VST3 si possible.
