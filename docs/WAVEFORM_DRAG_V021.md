# Liberty ROMpler v0.21 — Déplacer la boucle directement sur la forme d'onde

**Statut : développement sur `feature/v021-waveform-shift-drag-loop`. La v0.20 reste la dernière distribution macOS validée tant que le build v0.21 n'est pas vert.**

## Fonction : Maj + glisser dans la boucle

Dans **SAMPLES > BOUCLES**, la zone de forme d'onde comprise entre DÉBUT et FIN apparaît déjà avec un remplissage coloré.

- **Glisser normalement un marqueur** : déplacer seulement DÉBUT ou FIN, comportement historique inchangé.
- **Maj + glisser à l'intérieur de la zone colorée** : déplacer toute la boucle horizontalement.
- Le décalage est calculé par rapport à la position de la souris **au début du geste**, pour éviter les décalages cumulés et les sauts au premier mouvement.
- DÉBUT et FIN bougent simultanément d'un nombre exact de frames ; **FIN − DÉBUT reste rigoureusement identique**, même quand un bord du WAV est atteint.
- Le fondu anti-clic reste inchangé, et **AUTO ZÉRO n'est pas exécuté après ce glissement couplé** : un recalage automatique d'un seul marqueur casserait le verrouillage de durée.

Aucun bouton, aucun panneau, aucun raccourci antérieur n'est supprimé. Le nouvel usage est aussi expliqué par l'infobulle de l'onglet **BOUCLES**.

## Intégrité et compatibilité

Le mouvement est préparé dans le composant graphique, hors du callback audio. L'algorithme `Source/LoopDragV021.h` est autonome (C++17, pas de JUCE, pas d'allocation) et réutilise les limites du `nudgePair()` de la v0.20.

Le traitement n'écrit ni dans les WAV, ni dans les banques, ni dans les presets. L'utilisateur peut **ANNULER** la modification ; elle n'est enregistrée dans la zone du projet qu'avec **APPLIQUER**. Le zoom, les marques de boucle, la préécoute A/B et le moteur restent inchangés.

Les deux règles Liberty sont respectées : **aucun chevauchement** d'éléments graphiques (zéro contrôle supplémentaire) et **aucun caractère de remplacement des lettres françaises**.

## Validation

- `tests/loop_drag_v021_test.cpp` : continuité du geste, retour au point initial, saturation aux deux frontières, boucles minimales et grands entiers.
- `tests/test_v021_waveform_drag.py` : compilation C++17, contrôle des callbacks souris de l'éditeur JUCE, préservation des fonctions v0.15–v0.20, contrôle des échappements de caractères français.
- Prévalidation Linux : reconstruction, tests unitaires, audit texte et compilation JUCE 8 VST3.
- Livraison macOS : un seul build AU/VST3/Standalone après prévalidation verte, avec ZIP, DMG et archive des sources.
- Vérification manuelle : `docs/GUIDE_TEST_BOUCLES_V021.md`.

**Attention** : le programme de validation n'équivaut pas à un contrôle d'écoute ou d'ergonomie sur un Mac réel. Ces essais devront être effectués après le build.
