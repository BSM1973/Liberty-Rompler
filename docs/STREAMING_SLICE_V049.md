# Liberty ROMpler v0.49 — Fiabilisation SLICE / streaming

## Causes constatées en v0.48
Le banc macOS v0.48 affiche 1 926 blocs sous-alimentés pour SLICE, avec 917 835 ratés de cache pendant le scénario ; 7 sous-alimentations et 9 blocs dépassant le budget pour 64 voix. Un build vert ne garantit pas le bon comportement d'un disque lent ni l'absence de clics dans un DAW.

## Changements dans le moteur (sans nouveau bouton)
- Pour les WAV longs de la zone SLICE principale uniquement de chaque couche, prélecture des sept têtes de tranches éloignées, jusqu'à 3,25 secondes et 147 456 frames par tête, pendant la préparation du preset. Le premier tronçon dispose aussi d’une attaque RAM bornée à 3,25 s ou 147 456 frames. Ces petites fenêtres immuables sont consultables sans I/O, allocation ni mutex dans le callback.
- Lecture interpolée des tranches directement depuis les fenêtres préparées au moment de leur déclenchement, avant de reprendre la lecture par les pages streaming. Les sons des autres zones et les presets sans SLICE ne sont pas touchés.
- Accélération du cache PCM pour les accès successifs à une même page : un indice atomique opportuniste évite la recherche complète sur 32 pages lorsque possible. Le protocole d'épinglage des lecteurs reste obligatoire pour éviter les lectures déchirées lors des évictions.
- Aucun changement des banques v1, des paramètres enregistrés, de la navigation ni des traitements granulaire et spectral.

## Qualification nécessaire
La prélecture est un compromis borné (au plus environ 34 Mio pour les huit têtes de quatre zones pilotes à 44,1 kHz), pas une garantie contre toutes les latences disque : pour une lecture soutenue sur un support réellement lent, la profondeur du cache et l'anticipation devront être testées. En v0.49, comparer les rapports poly64, SLICE, switch à la v0.48 (CPU, RAM, défauts, durée de callback). Examiner les anciens presets AU/VST3 et tester un SSD interne et un support USB lent. Mesurer spécifiquement l'activation de SLICE, les démarrages sur les touches 36-43, boucles et changements de preset.

L'échec du benchmark SLICE doit rester visible et ne doit pas être qualifié de succès musical sans essai dans un vrai hôte audio. Conserver thème bleu nuit, onze onglets, caractères accentués et aucune superposition.
