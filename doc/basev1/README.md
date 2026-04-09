# Base V1 - Service Vision Darts

Ce dossier fige une base technique V1 pour la partie `C++ / OpenCV` du projet DARTS.

Cette base a ete redigee a partir :

- des diagrammes UML du dossier `C:\Users\pcben\Downloads\DIAGRAMMES`
- des dossiers locaux `opencv`, `nlhomann` et `openjson`
- d'une verification complementaire sur des sources officielles OpenCV et `nlohmann::json`

Date de validation de cette base : `2026-04-03`

## Objet exact de cette base

Le service `ServiceVisionDarts` :

- ouvre `1 a 3` cameras
- charge la calibration
- surveille la cible en preview
- detecte un tir
- attend la stabilisation
- detecte l'impact par camera
- projette les impacts sur le plan de la cible par homographie
- fusionne les mesures
- calcule le score du tir
- envoie un JSON pret au backend local
- expose aussi une petite API locale de commandes

Le service ne gere pas :

- la logique complete de partie `301`
- le joueur courant
- le passage de tour
- l'historique
- `SQLite`
- `Electron`

Ces responsabilites restent cote backend / UI.

## Plan du dossier

- `01-perimetre-et-responsabilites.md`
- `02-architecture-cpp-opencv.md`
- `03-pipeline-d-un-tir.md`
- `04-api-locale-et-contrat-json.md`
- `05-validation-et-points-restants.md`
- `sources-officielles.md`

## Resume decisionnel V1

- base spatiale retenue : `homographie`, pas triangulation 3D
- synchronisation camera : `assez proche temporellement`, sans exigence de synchro materielle
- strategie de capture recommandee : `grab()` sur toutes les cameras, puis `retrieve()`
- sortie du service : un evenement JSON deja score et exploitable
- transport vers backend : `HTTP local`
- stockage base de donnees : cote backend uniquement
- mode de jeu cible : `301`, mais gere cote backend

## Statut

Pour moi, cette base V1 est coherente avec les diagrammes, avec le decoupage entre `OpenCV` et `nlohmann::json`, et avec ton perimetre reel de travail.
