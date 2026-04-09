# API locale et contrat JSON

## 1. Principe general

Le service vision :

- recoit des commandes HTTP locales
- envoie des evenements HTTP locaux vers le backend

Le transport est local au `Raspberry Pi 4`.

## 2. Commandes entrantes

### `POST /commands/start`

But :

- demarrer la boucle vision

Exemple de reponse :

```json
{
  "accepted": true,
  "service": "vision",
  "state": "running"
}
```

### `POST /commands/stop`

But :

- arreter la boucle vision proprement

### `POST /commands/reset-reference`

But :

- forcer une nouvelle capture de reference stable

### `POST /commands/calibrate`

But :

- lancer ou appliquer une calibration

Exemple de payload minimal :

```json
{
  "camera_id": 1,
  "points_image": [
    {"x": 122.4, "y": 81.7},
    {"x": 534.9, "y": 96.5},
    {"x": 548.2, "y": 501.1},
    {"x": 118.6, "y": 488.9}
  ],
  "points_board": [
    {"x": -0.8, "y": 0.8},
    {"x": 0.8, "y": 0.8},
    {"x": 0.8, "y": -0.8},
    {"x": -0.8, "y": -0.8}
  ],
  "offset_angle_deg": 0.0
}
```

Remarque :

le point important n'est pas la forme exacte des valeurs ci-dessus, mais le contrat logique :

- un identifiant de camera
- des points image
- les points cibles correspondants
- un offset d'orientation si necessaire

### `GET /healthcheck`

But :

- connaitre l'etat du service

Exemple de reponse :

```json
{
  "service": "vision",
  "status": "ok",
  "running": true,
  "cameras_configured": 3,
  "cameras_opened": 3,
  "calibration_loaded": true,
  "last_error": null
}
```

## 3. Evenements sortants

Le backend doit pouvoir recevoir au minimum :

- `shot_detected`
- `shot_invalid`
- `vision_error`
- `calibration_required`

Convention V1 recommandee :

- le service vision poste tous ses evenements vers un endpoint unique
- endpoint backend recommande : `POST /vision/events`
- URL complete typique : `http://127.0.0.1:8080/vision/events`

Avantage :

- un seul point d'entree a maintenir cote backend
- le type exact d'evenement reste transporte par le champ `event`

## 4. Contrat JSON V1 pour un tir valide

Exemple de base retenu :

```json
{
  "event": "shot_detected",
  "schema_version": 1,
  "shot_id": "shot-000123",
  "timestamp_ms": 1760000000123,
  "status": "valid",
  "segment": "T20",
  "score": 60,
  "sector": 20,
  "ring": "TRIPLE",
  "multiplier": 3,
  "board_point": {
    "x": 0.123,
    "y": -0.456,
    "space": "board_normalized"
  },
  "confidence": 0.93,
  "processing_ms": 84,
  "cameras_expected": 3,
  "cameras_used": 3,
  "camera_impacts": [
    {
      "camera_id": 1,
      "x": 0.120,
      "y": -0.450,
      "quality": 0.91,
      "used_in_fusion": true
    },
    {
      "camera_id": 2,
      "x": 0.126,
      "y": -0.458,
      "quality": 0.89,
      "used_in_fusion": true
    },
    {
      "camera_id": 3,
      "x": 0.123,
      "y": -0.461,
      "quality": 0.95,
      "used_in_fusion": true
    }
  ]
}
```

## 5. Sens des champs

- `event` : type d'evenement
- `schema_version` : version du contrat JSON
- `shot_id` : identifiant unique du tir
- `timestamp_ms` : horodatage du tir
- `status` : `valid`, `uncertain` ou `invalid`
- `segment` : etiquette lisible telle que `T20`, `D16`, `S5`, `OUTER_BULL`, `INNER_BULL`, `MISS`
- `score` : score du tir uniquement
- `sector` : secteur numerique, ou `null` pour `MISS` et les bulls
- `ring` : `MISS`, `SINGLE`, `DOUBLE`, `TRIPLE`, `OUTER_BULL`, `INNER_BULL`
- `multiplier` : `1`, `2`, `3` pour les secteurs numerotes, ou `null` pour `MISS` et les bulls
- `board_point` : coordonnees du point final dans le repere `board_normalized`
- `confidence` : confiance finale du systeme
- `processing_ms` : duree de traitement
- `cameras_expected` : nombre de cameras attendues
- `cameras_used` : nombre de cameras ayant contribue a la fusion
- `camera_impacts` : details de diagnostic par camera

## 6. Contrat JSON V1 pour un tir invalide

```json
{
  "event": "shot_invalid",
  "schema_version": 1,
  "shot_id": "shot-000124",
  "timestamp_ms": 1760000001123,
  "status": "invalid",
  "reason": "not_enough_consistent_cameras",
  "confidence": 0.27,
  "cameras_expected": 3,
  "cameras_used": 1
}
```

## 7. Contrat JSON V1 pour une erreur vision

```json
{
  "event": "vision_error",
  "schema_version": 1,
  "timestamp_ms": 1760000002123,
  "code": "camera_open_failed",
  "message": "Unable to open camera 2"
}
```

## 8. Contrat JSON V1 pour une calibration manquante

```json
{
  "event": "calibration_required",
  "schema_version": 1,
  "timestamp_ms": 1760000003123,
  "missing_camera_ids": [1, 2, 3]
}
```

## 9. Recommandations de contrat

- garder `schema_version` des la V1
- garder des enums texte lisibles plutot que des codes opaques
- separer clairement les erreurs vision des tirs invalides
- ne pas obliger le backend a refaire le calcul du score
- conserver les details `camera_impacts` pour le debug

## 10. Repere V1 retenu pour `board_point`

Pour la V1, le repere retenu est :

- centre de la cible = `(0, 0)`
- coordonnees 2D normalisees
- rayon du `double outer` = `1.0`

Ce choix simplifie :

- la calibration
- la fusion multi-camera
- les tests
- le debug
- l'interoperation avec le backend

Si vous voulez un jour ajouter des millimetres reels, il sera plus propre de les ajouter en champs supplementaires plutot que de casser la V1.
