# SAMPLES v0.12 — Fondu anti-clic et poignées de boucle

L'onglet SAMPLES conserve la gestion non destructive du mapping, des couches de
vélocité et des boucles de la v0.11. Cette mise à jour ajoute un **FONDU
(FRAMES)** de zéro à 8192 frames, limité à la moitié de la longueur réelle de
la boucle. La valeur **0** garde exactement le moteur sonore des anciennes
sessions.

Dans **BOUCLES**, cochez **Boucle activée** et déplacez les deux marqueurs
sur la forme d'onde. Le marqueur le plus proche du clic est déplacé ; les points
restent ordonnés et séparés d'au moins onze frames. Le segment violet indique
la partie finale utilisée par le fondu. Réglez le fondu depuis le curseur pour
ajuster la transition, puis cliquez sur **APPLIQUER**. Le bouton **RESTAURER**
revient aux valeurs enregistrées dans la banque. Les WAV et les manifests ne
sont jamais modifiés.

Le moteur superpose linéairement la fin de la boucle et la portion initiale
correspondante. Après le passage du point de fin, la lecture reprend *après*
la portion déjà jouée par le fondu pour éviter une répétition. Le fondu est
réalisé sans allocations dans le thread audio et conserve les deux canaux
stéréo. Les réglages sont mémorisés dans le DAW sous `loopCrossfadeFrames` ; les
projets antérieurs sans ce champ chargent la valeur neutre 0.

**Limites :** fondu linéaire, pas de détection automatique des passages par zéro ;
les résultats dépendent du signal et de la durée de la boucle. L'application
d'une modification recharge le sample et peut couper une note en cours.
Vérifier à l'écoute sur un son tenu, dans un DAW AU et VST3. Le script de tests
vérifie les bornes, le calcul du fondu, les positions après boucle, les textes
UTF-8 français et les zones sans chevauchement.
