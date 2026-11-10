# Liberty ROMpler v0.19 — Contrôle macOS de la recherche PAIRE

À réaliser uniquement après un build macOS v0.19 réussi, en Standalone puis si possible en AU et VST3.

## Déplacement simultané des marqueurs

1. Charger un preset, ouvrir **SAMPLES > BOUCLES**, activer une boucle et relever DÉBUT, FIN et FIN − DÉBUT en frames.
2. Cliquer normalement sur ZÉRO DÉBUT puis ZÉRO FIN : le comportement historique de recherche par zéro doit être conservé.
3. Faire Option + clic : le mode SAUT MIN de la v0.18 doit rester opérationnel sur un seul marqueur.
4. Cliquer sur ANNULER pour restaurer les positions enregistrées, puis faire **Maj + Option + clic** sur ZÉRO DÉBUT ou ZÉRO FIN.
5. Relever le décalage PAIRE affiché. **DÉBUT et FIN doivent avoir avancé ou reculé exactement du même nombre de frames**, sans changer FIN − DÉBUT.
6. Vérifier qu'un point déjà optimal ne se déplace pas.
7. Tester le bouton **ANNULER** : le couple de marqueurs doit revenir à l'état appliqué. Après une nouvelle proposition, utiliser APPLIQUER puis sauvegarder et rouvrir le projet pour contrôler la persistance.

## Tests audio et limites

- Vérifier les WAV mono et stéréo, ainsi qu'un WAV long.
- Vérifier les boucles proches du début et de la fin du fichier.
- Vérifier que la durée minimale reste respectée et que le fondu ne change pas.
- Comparer à l'oreille les modes PRÉÉCOUTE A (Maj) et B, court et long si nécessaire.
- Aucun changement de WAV, de banque ou de preset par la recherche.
- Aucune opération supplémentaire dans le callback audio temps réel.

## Deux règles absolues

**Aucun chevauchement** de boutons, menus, repères ou libellés, même sur écran 13 pouces.

**Aucun symbole incorrect à la place des accents français** : é, è, ê, à, ç.

Une mesure numérique plus faible du saut brut ne garantit pas l'absence de clic perceptible ; l'écoute doit confirmer l'amélioration.
