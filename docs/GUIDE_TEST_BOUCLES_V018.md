# Liberty ROMpler v0.18 — Recette macOS du raccord assisté

Cette recette s'applique après le build macOS v0.18 et son installation. Les WAV et les banques doivent rester intacts.

## Test des deux boutons ZÉRO

1. Dans SAMPLES > BOUCLES, sélectionner un preset dont la boucle est active, puis noter les valeurs de DÉBUT et FIN.
2. Cliquer sur **ZÉRO DÉBUT** : le calage sur un passage par zéro doit fonctionner comme dans la v0.17.
3. Cliquer sur **ANNULER**, puis faire **Option + clic sur ZÉRO DÉBUT**. Observer la proposition SAUT MIN. Le marqueur peut bouger de 0 à 512 frames au maximum, sans dépasser les limites légales de la boucle.
4. Tester **Option + clic sur ZÉRO FIN**. Le marqueur FIN doit conserver sa définition **exclusive** ; la recherche compare la dernière frame réellement jouée au premier échantillon de début.
5. Dans une boucle déjà optimale, SAUT MIN doit indiquer qu'aucun meilleur point n'a été trouvé, et ne rien déplacer.
6. Cliquer sur **ANNULER** : les positions enregistrées doivent réapparaître.
7. Répéter, écouter avec PRÉÉCOUTE (B, fondu) et Maj + PRÉÉCOUTE (A, sans fondu), puis seulement cliquer sur **APPLIQUER** si le nouveau point convient.

## Scénarios complémentaires

- WAV mono et stéréo, dont une boucle avec un raccord gauche/droite différent.
- Marqueurs situés près du début et de la fin du fichier.
- Boucles courtes : la durée minimale de 11 frames doit toujours être respectée.
- Fichiers WAV longs : la recherche doit rester limitée aux fenêtres proches du raccord.
- Réouverture du projet : les nouveaux points ne doivent persister que si APPLIQUER et la sauvegarde du projet ont réellement été utilisés.

## Règles visuelles et audio

- Aucun chevauchement de texte, bouton, menu ou graphique, en particulier sur un Mac de 13 pouces.
- Aucun caractère erroné à la place des lettres accentuées françaises (é, è, ê, à, ç).
- AUTO ZÉRO doit rester désactivé par défaut pour les anciennes sessions.
- Aucun décodage ou calcul nouveau dans `processBlock()`. Les fichiers WAV ne doivent jamais être modifiés.
- Le résultat de SAUT MIN est une mesure du **saut brut d'amplitude**, pas une garantie d'absence de clic. Confirmer à l'oreille.

En cas de problème, relever le format (Standalone, AU ou VST3), le fichier WAV, les frames initiales/finales et l'action exacte.
