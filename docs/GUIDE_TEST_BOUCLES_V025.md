# Liberty ROMpler v0.25 — Contrôle sur macOS

Ce guide concerne la v0.25 après build et installation. Aucun WAV, preset ou projet ne doit être modifié par la navigation.

## Maj + molette

1. Ouvrir l'application Standalone puis **SAMPLES > BOUCLES** avec un WAV long et une boucle active.
2. Activer un zoom intermédiaire, noter DÉBUT, FIN, FONDU, le zoom et la valeur POSITION.
3. Sur la forme d'onde, maintenir **Maj** et faire tourner la molette verticale vers le haut : l'affichage doit naviguer vers le début du WAV.
4. Faire tourner la molette vers le bas : l'affichage doit naviguer vers la fin du WAV, et la barre POSITION doit suivre.
5. Tester un trackpad Mac avec Maj et défilement vertical, puis une souris à molette. Certains pilotes convertissent le déplacement en horizontal ; dans les deux cas la navigation doit fonctionner.
6. Aller au début et à la fin du fichier : aucun dépassement ni clignotement.
7. Revenir au zoom 1× (tout le WAV visible) : Maj + molette ne doit plus déplacer la fenêtre.
8. Refaire avec la boucle désactivée : naviguer doit rester possible sans modifier les marqueurs.

## Vérification des gestes historiques

- Molette sans Maj = zoom classique.
- Option + molette = zoom sous le curseur.
- Molette horizontale = navigation historique.
- Option + glisser = déplacement de la vue.
- Maj + glisser à l'intérieur de la boucle = déplacement couplé de DÉBUT et FIN.
- Double-clic normal = tout le WAV ; Option + double-clic = cadrage boucle.
- Boutons ±1/±10, SAUT MIN, PAIRE, ZÉRO et préécoute A/B restent fonctionnels.
- Après une simple navigation, DÉBUT, FIN et FONDU ne changent pas ; ANNULER/APPLIQUER ne sont pas activés et AUTO ZÉRO ne déclenche aucun recalage.

## Deux règles incontournables

**Aucun chevauchement** de textes, menus, boutons et repères, notamment sur écran 13 pouces.

**Aucun symbole à la place des accents français** : é, è, ê, à, ç.

Réaliser ces essais en Standalone, puis si possible dans un DAW en AU et VST3. Pour tout écart, noter le périphérique de défilement, le zoom, la longueur du WAV et la direction de la molette.
