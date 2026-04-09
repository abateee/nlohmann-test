# Pipeline d'un tir

## 1. Demarrage

Au demarrage du service :

1. charger la configuration locale
2. demarrer l'API locale de commande
3. attendre la commande `start`
4. ouvrir toutes les cameras
5. verifier la disponibilite des calibrations
6. si la calibration manque, publier `calibration_required`
7. sinon capturer une reference stable initiale

## 2. Calibration

La calibration est `markerless`.

Base V1 retenue :

- une homographie par camera
- points image choisis manuellement
- points cible connus dans le repere du plateau
- offset angulaire complementaire si necessaire pour l'orientation

Sortie attendue :

- une `CalibrationCamera` par camera
- stockee localement pour les prochains demarrages

## 3. Boucle normale de surveillance

Tant que le service est actif :

1. capturer les frames des cameras
2. faire une detection legere de changement
3. si aucun changement utile n'est detecte, rester en preview
4. si un changement est detecte, entrer en phase de stabilisation

## 4. Stabilisation

Apres un mouvement :

1. attendre que la scene redevienne stable
2. valider cette stabilite sur `N` frames consecutives
3. prendre alors le snapshot post-tir

But :

- eviter de traiter une image en plein mouvement
- diminuer les faux positifs

## 5. Detection d'impact par camera

Pour chaque camera exploitable :

1. prendre la reference correspondante
2. comparer reference et snapshot
3. produire une image de difference
4. segmenter la zone nouvelle
5. extraire un candidat impact
6. lui associer une mesure de qualite

Le detail exact de l'algorithme reste libre en implementation, mais la sortie logique doit rester :

- un point image candidat
- une qualite
- un statut valide ou non

## 6. Projection sur le plan cible

Pour chaque impact camera valide :

1. appliquer la calibration de la camera
2. projeter le point image sur le plan 2D de la cible
3. obtenir un `point_cible`

Important :

la base V1 parle d'un plan de cible 2D via homographie, pas d'une reconstruction 3D complete.

## 7. Fusion multi-camera

La fusion doit etre robuste et simple.

Politique V1 recommandee :

1. retirer les impacts de trop mauvaise qualite
2. comparer les impacts projetes entre eux
3. rejeter les outliers trop eloignes du groupe principal
4. si au moins `2` impacts restent coherents, calculer un point final fusionne
5. produire une confiance finale

Fusion recommandee :

- moyenne ponderee par `quality`

Conditions de sortie :

- `valid` si au moins deux cameras coherentes
- `uncertain` ou `invalid` sinon

## 8. Calcul du score

A partir du point final :

1. calculer le rayon `r`
2. calculer l'angle `theta`
3. determiner l'anneau
4. determiner le secteur
5. deduire le multiplicateur
6. produire la valeur du tir

Exemples :

- `TRIPLE 20 => 60`
- `DOUBLE 16 => 32`
- `INNER_BULL => 50`
- `OUTER_BULL => 25`

Hypothese V1 retenue :

- le point final est exprime en repere `board_normalized`
- le centre du plateau vaut `(0, 0)`
- le rayon du `double outer` vaut `1.0`

## 9. Emission du resultat

Une fois le tir valide :

1. generer un `shot_id`
2. construire le JSON final
3. faire un `POST` HTTP local vers le backend
4. journaliser le resultat en log local si besoin

Le JSON doit etre exploitable sans reinterpretation complexe cote backend.

## 10. Mise a jour de la reference

La reference doit etre geree avec soin.

Recommandation V1 forte :

- capturer une nouvelle reference stable apres chaque tir valide

Raison :

si une fleche reste plantee sur la cible, garder l'ancienne reference pollue la detection des tirs suivants.

Donc :

- au demarrage : reference initiale
- apres `reset_reference` : reference forcee
- apres chaque tir valide : reference post-tir stable

## 11. Cas d'erreur utiles

Les cas suivants doivent produire un evenement exploitable :

- camera indisponible
- calibration absente
- aucune camera coherente
- impact trop incertain
- echec d'envoi HTTP

## 12. Resultat attendu du pipeline

Le pipeline complet doit produire l'un de ces etats :

- `shot_detected`
- `shot_invalid`
- `vision_error`
- `calibration_required`
