# Liberty ROMpler v0.27 — Recette macOS du retour au cadrage

À tester après le build macOS v0.27 dans SAMPLES > BOUCLES.

1. Charger un WAV long. Choisir un zoom et une position visibles, puis zoomer avec la molette. **Maj + double-clic** doit restaurer l'ancien zoom et l'ancienne position.
2. Refaire **Maj + double-clic** : la vue que l'on vient de quitter doit réapparaître. Les deux cadrages doivent pouvoir alterner sans dérive.
3. Faire **Option + glisser** pour déplacer la vue, puis **Maj + double-clic** : revenir à la position de départ du glissement, et non à l'une des positions intermédiaires de la souris.
4. Tester après **Option + molette**, **Maj + molette**, **Option + double-clic** (cadrage boucle), **Commande + double-clic** (centrage), curseur ZOOM, curseur POSITION et le bouton TOUT.
5. Charger un **autre sample**, puis tenter Maj + double-clic : le cadrage de l'ancien sample ne doit pas être restauré.
6. Avec tout le WAV visible, effectuer Maj + double-clic plusieurs fois : aucune sortie de bornes, aucun crash.
7. Vérifier que **DÉBUT, FIN et FONDU** restent inchangés par ce raccourci et que les modifications ne deviennent pas NON APPLIQUÉES.

Vérifier les gestes existants : double-clic normal, Option + double-clic, Commande + double-clic, Maj + glisser dans la boucle, déplacement individuel des marqueurs, ZÉRO, PAIRE, SAUT MIN, ±1/±10 et préécoute A/B. Aucun WAV ou preset ne doit être réécrit.

**Règles Liberty :** zéro chevauchement de composants à la taille d'un écran 13 pouces, caractères accentués é, è, ê, à, ç affichés normalement. Tester en Standalone puis si possible en AU et VST3.
