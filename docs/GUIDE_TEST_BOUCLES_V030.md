# Liberty ROMpler v0.30 — Contrôle des durées de boucles sur macOS

À réaliser dans **SAMPLES > BOUCLES**, après compilation et installation de la v0.30, en Standalone puis si possible en AU et VST3.

1. Charger un WAV de **48 kHz** et définir DÉBUT = 24000, FIN = 48000 (durée 24000 frames). La ligne existante sous la forme d'onde doit indiquer **500.00 ms**.
2. Passer la souris sur **Boucle activée** : l'infobulle doit présenter le nombre de frames, la durée en ms, les frames du FONDU et la durée du FONDU en ms.
3. Définir un FONDU de 2400 frames : à 48 kHz, il doit correspondre à **50.000 ms**.
4. Faire glisser DÉBUT ou FIN, utiliser les pas ±1/±10, puis déplacer les deux marqueurs ensemble : les durées doivent changer correctement, sauf quand seule la boucle entière se déplace à durée fixe.
5. Charger un WAV à **44,1 kHz** et vérifier qu'une boucle de 44100 frames indique **1000.00 ms**. Tester si possible aussi des WAV à 96 et 192 kHz.
6. Désactiver la boucle : la ligne d'aide historique « Glisser les marqueurs ou régler le fondu » doit réapparaître. Réactiver : les durées doivent revenir.
7. Vérifier que FIN correspond à une limite **exclusive** : la durée est FIN - DÉBUT (pas +1).
8. Vérifier qu'aucune valeur de durée n'est affichée lorsque le WAV est absent ou invalide.

## Régressions et nos deux règles

Tous les gestes de navigation, zoom de précision sur DÉBUT/FIN (v0.29), historique de la vue, ZÉRO, SAUT MIN, PAIRE, préécoute A/B et ANNULER/APPLIQUER doivent rester disponibles. L'affichage de durée n'a pas le droit de rendre le projet NON APPLIQUÉ ni d'écrire un WAV.

**Aucun chevauchement** de texte, bouton, curseur ou fenêtre sur écran 13 pouces ; **aucune lettre accentuée remplacée par un symbole** (é, è, ê, à, ç). Contrôler surtout l'infobulle et la ligne sous la forme d'onde.
