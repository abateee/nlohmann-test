# Validation et points restants

## 1. Ce qui est fige dans cette base V1

- le moteur vision est separe de la logique complete de partie
- la localisation finale repose sur l'homographie
- le backend gere `301`, les joueurs, les tours, l'UI et la base
- le service vision envoie un JSON final deja score
- le service vision expose aussi des commandes locales
- `SQLite` reste cote backend
- une mise a jour de reference apres tir valide est recommandee
- une degradation `3 cameras -> 2 cameras` est recommandee plutot qu'un rejet systematique

## 2. Ce qui est volontairement laisse ouvert

- la valeur exacte du `seuilOutlier`
- la formule exacte de `confidence`
- l'algorithme precis de segmentation d'impact
- le format final du payload de calibration
- le choix de la bibliotheque HTTP C++

## 3. Recommandations fortes

### Politique d'acceptation de tir

Base recommandee :

- `valid` si au moins `2` cameras sont coherentes
- `uncertain` si un seul impact existe mais avec qualite moyenne
- `invalid` si aucune estimation fiable n'est disponible

### Mise a jour de reference

Base recommandee :

- reference initiale au demarrage
- reference forcee sur `reset_reference`
- reference renouvelee apres chaque tir valide une fois la scene stabilisee

### Gestion du temps

Base recommandee :

- utiliser un horodatage monotone interne pour mesurer `processing_ms`
- utiliser aussi un horodatage de service pour les evenements JSON

### Repere de sortie

Base retenue pour la V1 :

- `board_point` en coordonnees `board_normalized`
- centre du plateau en `(0, 0)`
- rayon du `double outer` egal a `1.0`

## 4. Check-list de validation fonctionnelle

Avant de considerer la V1 utilisable, il faut verifier au minimum :

- `healthcheck` repond correctement
- les `3` cameras s'ouvrent avec les bons identifiants
- une calibration peut etre chargee et relue
- la detection reste en preview sans faux positif massif
- un tir unique est detecte et score correctement
- l'envoi HTTP vers le backend fonctionne
- la reference se met bien a jour apres tir
- un cas degrade a `2 cameras` reste exploitable
- un cas sans calibration publie `calibration_required`

## 5. Risques techniques principaux

- eclairage instable
- exposition automatique trop variable
- mauvaise calibration
- presence de fleches deja plantees sur la cible
- latence reseau locale ou backend qui repond lentement
- mauvaise politique d'outlier

## 6. Position finale

Pour moi, cette base est suffisamment propre pour commencer l'implementation sans partir sur de mauvaises hypotheses.

Elle fixe :

- le bon perimetre
- le bon decoupage avec le backend
- le bon pipeline logique
- un contrat JSON exploitable

Les points restants sont des points normaux de tuning, pas des flous d'architecture.
