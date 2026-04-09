# Perimetre et responsabilites

## 1. But du service

Le but du service `ServiceVisionDarts` est simple :

transformer un tir reel detecte sur une cible physique en un evenement JSON fiable, score et directement exploitable par le backend local.

## 2. Ce que la partie C++ / OpenCV doit faire

Le service vision doit :

- ouvrir et surveiller `1 a 3` cameras
- charger et sauvegarder les calibrations utiles
- maintenir une reference visuelle par camera
- detecter un changement sur la cible
- attendre la stabilisation apres impact
- produire un snapshot post-tir
- detecter un impact pour chaque camera exploitable
- projeter chaque impact en coordonnees cible 2D via homographie
- fusionner les estimations multi-camera
- convertir l'impact final en score de flechettes
- calculer un niveau de confiance
- envoyer un JSON propre au backend local
- recevoir des commandes locales telles que `start`, `stop`, `calibrate`, `reset_reference`, `healthcheck`

## 3. Ce que la partie C++ / OpenCV ne doit pas faire

Le service vision ne doit pas gerer :

- le choix du mode de jeu
- la logique complete de partie `301`
- le score restant global de la partie
- le joueur courant
- le passage de tour
- l'historique des manches
- l'ecriture en base `SQLite`
- l'interface `Electron`

## 4. Frontiere exacte avec le backend

Le service vision fournit :

- un tir detecte
- un score de tir
- une confiance
- des metadonnees utiles de diagnostic
- des evenements d'erreur ou de calibration

Le backend gere :

- les regles `301`
- la mise a jour du score restant
- les joueurs
- les tours
- l'affichage
- la persistence en base

## 5. Regle de responsabilite metier

La regle a retenir est :

le service vision produit le `score du tir`, mais pas l'etat complet de la partie.

Exemple :

- le service vision envoie `TRIPLE 20 => 60`
- le backend decide ensuite comment ce `60` impacte la partie `301`

## 6. Interfaces du service

Le service a trois interfaces utiles :

- entrees camera
- API locale de commande
- API locale de publication vers le backend

Il a aussi un stockage local technique pour :

- les calibrations
- la configuration d'execution
- les logs eventuels

## 7. Critere de succes V1

La V1 est consideree comme bonne si elle sait :

- demarrer proprement
- verifier l'etat des cameras
- charger une calibration valide
- detecter un tir sans faux positif excessif
- calculer un score de tir coherent
- envoyer un JSON lisible, stable et exploitable
- rester compatible avec un backend local qui gere le reste du projet
