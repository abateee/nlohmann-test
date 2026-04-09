# Architecture C++ / OpenCV

## 1. Vue logique

Le service peut etre decoupe en 6 blocs :

1. acquisition camera
2. detection de mouvement et stabilisation
3. gestion des references
4. calibration et projection
5. fusion d'impacts et calcul du score
6. serialisation JSON et transport HTTP local

Le tout est orchestre par `ServiceVisionDarts`.

## 2. Classes principales retenues

### `ServiceVisionDarts`

Responsabilite :

- orchestrer le cycle global
- maintenir l'etat du service
- appeler les autres modules dans le bon ordre

Etat minimal recommande :

- service actif ou non
- cameras ouvertes ou non
- calibration chargee ou non
- derniere erreur
- compteur de tir ou generateur de `shot_id`

### `GestionnaireCameras`

Responsabilite :

- posseder les objets `Camera`
- ouvrir et fermer toutes les cameras
- capturer des frames de facon homogene

Recommandation V1 :

- utiliser `grab()` sur toutes les cameras
- puis `retrieve()` sur toutes les cameras

Cela colle a la documentation OpenCV pour rapprocher temporellement des captures provenant de plusieurs cameras sans devoir decoder chaque image avant d'avoir interroge les autres.

### `Camera`

Responsabilite :

- encapsuler un device physique
- exposer l'identite, la resolution, le `fps` et le chemin du device
- produire une `ImageFrame`

### `ImageFrame`

Responsabilite :

- transporter une image et ses metadonnees

Champs utiles :

- `timestamp_ms`
- `camera_id`
- `cv::Mat mat`
- eventuellement `sequence_id` ou `monotonic_ts`

### `DetecteurPersistance`

Responsabilite :

- faire une detection legere de changement
- distinguer `mouvement` et `stabilisation`
- decider quand prendre le snapshot post-tir

Idee V1 :

- preview grayscale reduite
- difference simple par rapport a la reference
- compteur de frames stables consecutives

### `GestionnaireReference`

Responsabilite :

- stocker la frame de reference par camera
- fournir une reference coherente pour la detection d'impact
- rafraichir la reference quand il faut

### `CalibrateurMarkerless`

Responsabilite :

- convertir des points image en projection sur le plan de la cible
- calculer une homographie par camera

Base technique :

- si la calibration est strictement manuelle avec `4` points propres : `cv::getPerspectiveTransform` est suffisant
- si vous voulez accepter `4 points ou plus` avec robustesse : `cv::findHomography` est plus souple et peut utiliser `RANSAC`

### `StockageCalibration`

Responsabilite :

- charger et sauver les calibrations

Recommandation V1 :

- utiliser `cv::FileStorage` pour les objets purement OpenCV tels que matrices et homographies
- utiliser `nlohmann::json` pour le document applicatif global et les payloads HTTP

### `GestionnaireImpacts`

Responsabilite :

- detecter un impact par camera
- produire une liste `ImpactCamera`
- fusionner vers un `ImpactFinal`

Cette classe centralise bien la logique que les diagrammes faisaient d'abord porter a `DetecteurImpact` puis `FusionImpacts`.

### `CalculScore`

Responsabilite :

- convertir un point cible 2D en zone de score
- produire un `ResultatTir`

### `ServeurAPI`

Responsabilite :

- recevoir les commandes locales entrantes
- publier les evenements vers le backend

En pratique, cette brique peut contenir :

- un mini serveur HTTP local
- un client HTTP local pour `POST` vers le backend

## 3. Modele de donnees utile

### `ImpactCamera`

Champs minimum :

- `camera_id`
- `point_cible` en repere `board_normalized`
- `quality`

### `ImpactFinal`

Champs minimum :

- `point_cible` en repere `board_normalized`
- `confidence`

### `ResultatTir`

Champs minimum :

- `shot_id`
- `timestamp_ms`
- `x`
- `y`
- `anneau`
- `secteur`
- `multiplier`
- `score`
- `confidence`

## 4. Decoupage d'execution recommande

Une architecture simple et robuste pour la V1 :

- un thread principal de service
- un thread de boucle vision
- un thread d'API locale
- eventuellement une petite file d'envoi HTTP sortant

But :

- ne pas bloquer la capture si un `POST` HTTP prend du temps
- garder des commandes locales reactives

## 5. Politique multi-camera recommandee

Il ne faut pas modeliser les `3` cameras comme "tout ou rien" tant que ce n'est pas impose par une exigence metier.

Politique V1 recommandee :

- capturer toutes les cameras configurees
- calculer un impact par camera quand possible
- rejeter les cameras incoherentes via une logique d'outlier
- accepter un tir si au moins `2` cameras restent coherentes
- marquer la confiance plus basse si une camera manque ou est rejetee

Cette politique est plus robuste qu'un rejet systematique du tir des qu'une camera est faible.

## 6. Repartition OpenCV / JSON

Le partage de responsabilites doit rester propre :

- `OpenCV` gere les frames, les differences, les masques, les contours, les homographies et les structures vision
- `nlohmann::json` gere les commandes, les reponses, les evenements, la config et les resultats metier

## 7. Point d'attention pratique

Les exemples locaux du dossier `doc` sont utiles pour comprendre la direction du projet, mais ne doivent pas etre traites comme une specification stricte ou comme des exemples compilables sans relecture.
