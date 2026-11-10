# Liberty ROMpler v0.15 — Recette macOS de l'éditeur SAMPLES

Ce guide s'applique **uniquement après la compilation et l'installation de la v0.15**. Les composants v0.15 ne sont pas présents dans le plugin v0.14.1 livré précédemment.

## 1. Interface et précision à la frame

1. Lancez la version Standalone sur Mac Apple Silicon.
2. Chargez une banque et sélectionnez un preset contenant un son tenu.
3. Ouvrez **SAMPLES > BOUCLES**, activez la boucle et sélectionnez une zone WAV.
4. Relevez les valeurs **DÉBUT (FRAME)** et **FIN (FRAME)**.
5. Cliquez sur les boutons **-1** et **+1** respectifs : chaque clic doit déplacer uniquement le marqueur correspondant d'une frame.
6. Saisissez directement une valeur dans les champs numériques existants. Vérifiez les bornes et la durée minimale de la boucle.
7. Cliquez sur **ANNULER** : les valeurs non appliquées doivent être rétablies. Réessayez puis cliquez sur **APPLIQUER**.

## 2. Préécoute du raccord

1. Désactivez **AUTO ZÉRO** et réglez d'abord **FONDU** à 0 frame.
2. Cliquez sur **PRÉÉCOUTE**. L'extrait doit jouer une fois, sans déclencher de note MIDI ni modifier le WAV source.
3. Essayez des valeurs de fondu progressives, par exemple 64, 256 et 1024 frames (selon la longueur de la boucle). Réécoutez le raccord.
4. Comparez mono et stéréo. Vérifiez que les canaux ne sont pas mélangés ni inversés.
5. Reproduisez avec un WAV à 44,1 kHz dans une session à 48 kHz si possible. Le son doit garder une durée et une hauteur cohérentes.
6. Vérifiez la lecture dans AU et VST3, sans décrochage notable à l'arrêt ou après un changement de preset.

## 3. Repères visuels du fondu

1. Zoomez jusqu'à 16× et 64× ; déplacez **POSITION**.
2. Vérifiez que les deux bandes de fondu colorées restent attachées au début et à la fin de la boucle.
3. Vérifiez qu'aucune bande ni marqueur ne déborde du cadre de la forme d'onde.
4. Réglez le fondu à 0 : les bandes doivent disparaître.
5. Redimensionnez l'interface en petit et en grand. **Aucun chevauchement de boutons, textes ou menus n'est accepté.**

## 4. Compatibilité et règles Liberty

- Les caractères français **é, è, ê, à, ç** doivent rester corrects ; aucun symbole ne doit les remplacer.
- Les anciennes sessions doivent se rouvrir sans migration destructrice des banques ou des WAV.
- **AUTO ZÉRO** doit rester désactivé par défaut pour les projets antérieurs.
- Après sauvegarde, quittez le DAW puis rouvrez le projet : les positions de boucle et le fondu doivent être restaurés.

En cas d'anomalie, noter le format (Standalone, AU ou VST3), la version du DAW, le fichier WAV, la fréquence d'échantillonnage, le zoom et la valeur du fondu.
