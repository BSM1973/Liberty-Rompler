# Liberty ROMpler v0.21 — Recette macOS du glissement de boucle

Cette recette vérifie la v0.21 **après la compilation et l'installation** ; elle ne doit pas modifier les fichiers WAV.

## Glissement de la zone de boucle

1. Dans l'application Standalone, ouvrir **SAMPLES > BOUCLES** puis sélectionner une zone WAV avec boucle active.
2. Noter les nombres de frames DÉBUT, FIN, la différence **FIN − DÉBUT** et la valeur de FONDU.
3. Vérifier le comportement historique : glisser le marqueur DÉBUT ou FIN **sans Maj** ne déplace qu'un seul marqueur.
4. Maintenir **Maj**, cliquer à l'intérieur de la zone colorée **entre les deux marqueurs** et faire glisser horizontalement.
5. Vérifier que les deux nombres DÉBUT et FIN augmentent ou diminuent **du même nombre de frames**, et que la longueur ainsi que le fondu restent identiques.
6. Pendant le même geste, déplacer la souris vers la droite puis vers la gauche jusqu'à la position de départ : les valeurs doivent revenir aux valeurs initiales, sans dérive cumulée ni déplacement au clic initial.
7. Amener la boucle contre les deux bords du WAV : ni DÉBUT ni FIN ne doivent dépasser le fichier ou changer la longueur de boucle.
8. Cliquer sur **ANNULER**, vérifier le retour aux valeurs enregistrées. Réessayer, puis **APPLIQUER** et sauvegarder le projet pour tester la persistance.

## Régressions

- Une boucle très courte ou visuellement comprimée doit rester éditable via les boutons ±1 de la v0.20, même si la zone intérieure n'est pas assez grande pour un glissement précis.
- Les commandes **Option + clic**, **Maj + clic**, **ZÉRO**, **SAUT MIN** et **PAIRE** restent opérationnelles.
- Les préécoutes A/B courte et longue restent audibles, et la valeur de fondu ne change pas lors du glissement couplé.
- Même si AUTO ZÉRO est activé, un glissement couplé ne doit pas recalculer les marqueurs individuellement.
- Le zoom et le déplacement de la vue doivent rester utilisables.
- Tester si possible Standalone, AU et VST3, avec WAV mono et stéréo.

## Deux règles absolues

**Aucun chevauchement** des boutons, valeurs, textes ou marqueurs, notamment sur un écran Mac de 13 pouces.

**Aucun caractère incorrect** à la place des accents français : é, è, ê, à, ç.

Si un problème apparaît, noter le format, la fréquence du sample WAV, les marqueurs, le niveau de zoom et le geste effectué.
