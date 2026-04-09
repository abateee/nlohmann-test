# Bugfind 1

Audit realise sur la codebase presente dans `c:\Users\pcben\Desktop\nlohmann test`.

Commandes executees pendant la revue :

- `.\build\debug\visiondarts_tests.exe`
- `ctest --test-dir build\debug --output-on-failure`
- `.\build\debug\vision_replay.exe fixtures`
- reproductions ciblees sur des copies temporaires de fixtures et sur `vision_service.exe`

Etat de base constate :

- les tests unitaires passent
- le replay du lot `fixtures` passe
- plusieurs bugs importants restent hors couverture de test

## Findings

### 1. Eleve - un `expected.json` manquant desactive silencieusement la validation d'un scenario

Evidence code :

- `src/vision/replay.cpp:50-57` remplace un `expected.json` absent par un objet JSON vide
- `src/app/offline_engine.cpp:144-151` ne compare plus rien si `expected_json` est vide
- `README.md:478-484` documente pourtant `expected.json` comme contenu attendu du scenario

Impact :

Un scenario casse ou incomplet peut passer au vert sans aucune verification metier. C'est un faux positif direct dans le harness de replay.

Reproduction :

J'ai copie `fixtures\single_20`, supprime `expected.json`, puis execute `.\build\debug\vision_replay.exe <copie>`. Le scenario est sorti en succes avec `EXIT=0`.

Correction conseillee :

- faire echouer `load_scenario` si `expected.json` manque en mode replay de validation
- ou ajouter un mode explicite `--allow-missing-expected` si ce comportement est volontaire

### 2. Eleve - une calibration corrompue est annoncee comme `calibration_required` au lieu d'une vraie erreur

Evidence code :

- `src/app/offline_engine.cpp:55-71` attrape tous les `std::exception` pendant le chargement de calibration puis fabrique un evenement `calibration_required`
- `src/vision/calibration.cpp:31-67` peut jeter pour plusieurs causes differentes : fichier absent, JSON invalide, FileStorage illisible
- `doc/basev1/03-pipeline-d-un-tir.md:9-13` reserve `calibration_required` au cas ou la calibration manque

Impact :

Une calibration presente mais invalide est diagnostiquee comme "manquante". Le client recoit la mauvaise action corrective, et l'erreur reelle disparait du flux.

Reproduction :

J'ai corrompu temporairement `calibration.json` dans une copie de `single_20`. `vision_replay.exe` a emis :

```json
{
  "event": "calibration_required",
  "missing_camera_ids": [1]
}
```

alors que la cause etait un JSON tronque, pas une calibration absente.

Correction conseillee :

- ne convertir en `calibration_required` que les cas "fichier absent / calibration non fournie"
- pour les autres cas, emettre `vision_error` avec le message source

### 3. Moyen - `offset_angle_deg` est expose partout mais n'a aucun effet

Evidence code :

- `src/vision/calibration.cpp:47-51`, `src/vision/calibration.cpp:62`, `src/vision/calibration.cpp:77-79`, `src/vision/calibration.cpp:95-97` lisent, stockent et sauvegardent `offset_angle_deg`
- `src/vision/calibration.cpp:110-149` projette les points sans jamais l'utiliser
- `src/app/service_controller.cpp:108-112` accepte aussi ce champ via l'API `/commands/calibrate`

Impact :

Le contrat public laisse croire qu'un decalage angulaire camera est pris en charge, alors que le moteur l'ignore totalement. Une calibration avec rotation logique non nulle produira donc des scores faux sans signaler l'ecart.

Reproduction :

J'ai genere deux calibrations identiques sauf `offset_angle_deg = 0` puis `90`. `vision_calibration_check.exe` a retourne exactement la meme projection pour les deux.

Correction conseillee :

- soit appliquer reellement la rotation dans les projections et le scoring
- soit supprimer ce champ du contrat tant qu'il n'est pas supporte

### 4. Moyen - `POST /commands/calibrate` laisse remonter un JSON invalide en HTTP 500 brut

Evidence code :

- `apps/vision_service.cpp:34-36` parse `request.body` directement avec `nlohmann::json::parse`
- l'exception n'est pas transformee en reponse JSON de validation

Impact :

Un client qui envoie un payload mal forme recoit un `500 Internal Server Error` au lieu d'une reponse `4xx` exploitable. L'API locale est fragile sur le chemin d'erreur le plus simple.

Reproduction :

Un `POST /commands/calibrate` avec le body `{bad json` a retourne `STATUS=500` tandis que le service restait vivant.

Correction conseillee :

- entourer le parse du body par un `try/catch` local a la route
- renvoyer un `400` avec un JSON du type `{ "accepted": false, "error": "invalid_json" }`

### 5. Moyen - deux instances de `vision_service` peuvent se mettre en ecoute sur le meme port sans alerte

Evidence code :

- `apps/vision_service.cpp:43-44` affiche l'URL puis ignore la valeur de retour de `server.listen(...)`

Impact :

Le binaire ne protege pas le demarrage contre un service deja present. En pratique, deux processus peuvent rester actifs sur `127.0.0.1:8090`, ce qui ouvre la porte a des traitements concurrents ou non deterministes.

Reproduction :

Apres avoir lance `vision_service.exe` deux fois avec la meme config, `netstat -ano` montrait deux PIDs distincts en `LISTENING` sur `127.0.0.1:8090`.

Correction conseillee :

- verifier explicitement le retour de `listen`
- journaliser une erreur fatale et sortir avec un code non nul si le bind echoue ou si un autre processus occupe deja le service

### 6. Moyen - les valeurs par defaut backend sont incoherentes selon le codepath

Evidence code :

- `include/visiondarts/core/config.hpp:34-38` initialise par defaut `post_url = http://127.0.0.1:3000/api/game301/joueurs` et `service_port = 3000`
- `src/core/config.cpp:44-48` fournit d'autres valeurs de secours lors du parsing JSON : `post_url = http://127.0.0.1:8080/vision/events` et `service_port = 8090`
- `tools/generate_fixtures.ps1:288-292` regenere encore le couple `3000` / `/api/game301/joueurs`
- `README.md:458-462` documente `8080` / `8090`

Impact :

Le comportement par defaut change selon qu'on demarre sans fichier, avec un fichier partiel, ou avec un fichier regenere par l'outil. C'est une source directe d'erreurs d'integration et de confusion.

Correction conseillee :

- choisir une seule verite pour les defaults
- la centraliser dans une seule couche
- aligner le script de generation et la documentation sur cette meme source

### 7. Faible a moyen - `POST /commands/reset-reference` est un no-op alors que le contrat promet un reset effectif

Evidence code :

- `src/app/service_controller.cpp:67-74` renvoie juste un message de succes
- `doc/basev1/04-api-locale-et-contrat-json.md:36-40` annonce pourtant "forcer une nouvelle capture de reference stable"
- `doc/basev1/05-validation-et-points-restants.md:36-38` attend aussi une "reference forcee sur reset_reference"

Impact :

Le client recoit `accepted: true` pour une commande qui ne change aucun etat interne. L'API promet une action qui n'existe pas.

Correction conseillee :

- soit implementer un vrai drapeau / cycle de recapture de reference
- soit retirer cet endpoint de la V1 offline tant qu'il n'a pas d'effet

## Couverture de test manquante

Les tests actuels ne couvrent pas :

- l'absence de `expected.json`
- la corruption de `calibration.json`
- les payloads HTTP invalides
- le multi-demarrage du service
- les champs de contrat "dead" comme `offset_angle_deg`

Ajouter ces cas en regression empecherait que ces bugs restent invisibles alors que `ctest` et `vision_replay.exe fixtures` sont au vert.
