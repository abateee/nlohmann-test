# Vision Darts Offline

## 1. Objet du projet

Ce depot contient la V1 hors-ligne du moteur `C++ / OpenCV` pour un systeme de score automatique de flechettes.

Le perimetre actuel est volontairement limite a la phase **offline / replay** :

- pas d'acquisition camera reelle pour le moment
- pas de Raspberry Pi pour le moment
- pas de logique complete de partie `301` dans ce depot
- pas de backend metier ici, seulement un `mock_backend` pour valider les echanges HTTP

Le moteur actuel sait deja faire les choses suivantes :

- charger des scenarios de test a partir de paires d'images `reference + snapshot`
- charger une calibration par homographie
- detecter un impact par difference d'images
- projeter l'impact dans le repere `board_normalized`
- calculer le score d'un tir
- produire un JSON pret pour un backend local
- exposer une API HTTP locale pour piloter le service offline
- envoyer les evenements a un backend local via HTTP

## 2. Etat actuel

L'etat du projet au moment de cette redaction est le suivant :

- le build Windows `Debug` fonctionne
- les executables sont generes dans `build/debug`
- les DLL runtime necessaires sont copiees automatiquement dans `build/debug`
- les tests unitaires passent
- le lot de fixtures offline passe
- l'API locale HTTP est testee
- l'envoi vers le mock backend est teste

Resultats verifies :

- `ctest --preset debug` : OK
- `build/debug/vision_replay.exe fixtures` : OK
- `mock_backend` recoit 9 evenements sur le lot de fixtures courant : OK

## 3. Ce qui est dans le depot

### 3.1 Arborescence principale

- `apps/`
  contient les points d'entree executables
- `build/`
  contient les artefacts de build
- `doc/`
  contient la documentation de travail et les bases de conception
- `fixtures/`
  contient les scenarios offline
- `include/visiondarts/`
  contient les headers du projet
- `src/`
  contient l'implementation
- `tests/`
  contient les tests unitaires
- `tools/`
  contient les scripts de build et d'execution
- `CMakeLists.txt`
  fichier principal de build
- `CMakePresets.json`
  presets CMake
- `vcpkg.json`
  dependances du projet

### 3.2 Sous-systemes techniques

- `core`
  types metier, config, JSON, scoring
- `vision`
  calibration, replay, detection d'impact, fusion
- `api`
  publication HTTP des evenements
- `app`
  orchestration offline et controle de service

## 4. Conventions de travail

- Le repere de sortie officiel est `board_normalized`.
- Le centre de la cible est `(0, 0)`.
- Le rayon exterieur du double vaut `1.0`.
- Les commentaires du code doivent etre rediges en francais.
- La logique de partie `301` n'est pas geree ici.
- Ce depot fournit un moteur `vision + scoring de tir`, pas un backend metier complet.

## 5. Documentation deja presente

La documentation de cadrage principale se trouve dans :

- `doc/basev1/README.md`
- `doc/basev1/01-perimetre-et-responsabilites.md`
- `doc/basev1/02-architecture-cpp-opencv.md`
- `doc/basev1/03-pipeline-d-un-tir.md`
- `doc/basev1/04-api-locale-et-contrat-json.md`
- `doc/basev1/05-validation-et-points-restants.md`

Les dossiers suivants servent de documentation technique locale :

- `doc/opencv`
- `doc/nlhomann`
- `doc/openjson`

## 6. Prerequis techniques

### 6.1 Plateforme actuellement ciblee

La phase actuelle est validee sur Windows.

### 6.2 Outils utilises

Le projet s'appuie sur :

- `CMake`
- `Ninja`
- `Microsoft Build Tools`
- `vcpkg`
- `OpenCV`
- `nlohmann::json`
- `cpp-httplib`

### 6.3 Hypotheses actuelles sur cette machine

Les scripts fournis supposent les chemins suivants :

- `C:\BuildTools\Common7\Tools\VsDevCmd.bat`
- `C:\Users\pcben\tools\cmake\bin\cmake.exe`
- `C:\Users\pcben\tools\vcpkg\installed\x64-windows`

Si ces chemins changent, il faudra ajuster :

- `tools/build_debug.ps1`
- `CMakePresets.json`

## 7. Dependances vcpkg

Le fichier `vcpkg.json` declare :

- `opencv4`
- `nlohmann-json`
- `cpp-httplib`

Le port `opencv4` est volontairement limite au minimum utile pour la phase offline.

## 8. Build du projet

### 8.1 Methode recommandee

Depuis la racine du projet :

```powershell
.\tools\build_debug.ps1
```

Cette commande fait :

- chargement de l'environnement Visual Studio via `VsDevCmd.bat`
- configuration `cmake --preset debug`
- compilation `cmake --build --preset debug`
- copie des DLL runtime necessaires dans `build/debug`

### 8.2 Build avec tests

```powershell
.\tools\build_debug.ps1 -RunTests
```

### 8.3 Build manuel

Si tu veux tout faire toi-meme :

```powershell
cmd /c "call C:\BuildTools\Common7\Tools\VsDevCmd.bat -host_arch=x64 -arch=x64 && C:\Users\pcben\tools\cmake\bin\cmake.exe --preset debug && C:\Users\pcben\tools\cmake\bin\cmake.exe --build --preset debug"
```

### 8.4 Emplacement des binaires

Les executables sortent dans :

```text
build/debug
```

On y trouve notamment :

- `vision_replay.exe`
- `vision_service.exe`
- `mock_backend.exe`
- `vision_calibration_check.exe`
- `visiondarts_tests.exe`

## 9. Si une DLL manque au lancement

Le projet copie maintenant automatiquement les DLL `OpenCV`, `zlib`, `libpng`, `brotli`, etc. dans `build/debug`.

Si un popup du type `zlibd1.dll introuvable` reapparait :

1. relancer `.\tools\build_debug.ps1`
2. verifier que les DLL sont bien presentes dans `build/debug`
3. relancer ensuite l'executable

## 10. Executables fournis

### 10.1 `vision_replay.exe`

Role :

- execute un scenario ou un dossier de scenarios
- produit le JSON calcule
- compare avec `expected.json`
- renvoie un code d'erreur non nul si un scenario ne correspond pas

Usage :

```powershell
.\build\debug\vision_replay.exe fixtures
```

Pour un scenario unique :

```powershell
.\build\debug\vision_replay.exe fixtures\single_20
```

Pour sauvegarder les artefacts de debug :

```powershell
.\build\debug\vision_replay.exe fixtures --debug-out build\debug_output
```

Script raccourci :

```powershell
.\tools\run_replay_debug.ps1
```

Ou :

```powershell
.\tools\run_replay_debug.ps1 -ScenarioPath fixtures\single_20 -DebugOutput build\debug_output
```

### 10.2 `vision_service.exe`

Role :

- lance le service HTTP local offline
- charge une config JSON
- traite les scenarios au travers d'une API de commande
- envoie les evenements vers un backend HTTP local

Usage :

```powershell
.\build\debug\vision_service.exe fixtures\service_config.json
```

### 10.3 `mock_backend.exe`

Role :

- expose `POST /vision/events`
- journalise les evenements recus
- permet de valider l'integration HTTP sans backend reel

Usage recommande :

```powershell
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
```

Remarque :

- utiliser de preference un **chemin relatif** pour le fichier `.jsonl`
- cette commande a ete validee telle quelle

### 10.4 `vision_calibration_check.exe`

Role :

- charge une calibration
- projette des points image en points cible
- sert a verifier rapidement qu'une homographie est coherente

Usage :

```powershell
.\build\debug\vision_calibration_check.exe fixtures\single_20\calibration.json 400 400 400 302
```

## 11. API HTTP locale

Le service offline expose les endpoints suivants :

- `POST /commands/start`
- `POST /commands/stop`
- `POST /commands/reset-reference`
- `POST /commands/calibrate`
- `GET /healthcheck`

### 11.1 `GET /healthcheck`

Exemple :

```powershell
Invoke-RestMethod -Uri http://127.0.0.1:8090/healthcheck
```

Reponse typique :

```json
{
  "service": "vision",
  "status": "ok",
  "running": false,
  "state": "idle",
  "cameras_configured": 1,
  "cameras_opened": 1,
  "calibration_loaded": false,
  "current_scenario": "",
  "last_error": null
}
```

### 11.2 `POST /commands/start`

Exemple :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Effet :

- demarre le traitement du lot de scenarios configure

### 11.3 `POST /commands/stop`

Exemple :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/stop
```

Effet :

- arrete proprement le worker de traitement

### 11.4 `POST /commands/reset-reference`

Exemple :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/reset-reference
```

Effet :

- reinitialise l'etat logique de reference pour le prochain traitement

### 11.5 `POST /commands/calibrate`

Exemple de payload :

```json
{
  "camera_id": 1,
  "offset_angle_deg": 0.0,
  "points_image": [
    { "x": 230.0, "y": 230.0 },
    { "x": 570.0, "y": 230.0 },
    { "x": 570.0, "y": 570.0 },
    { "x": 230.0, "y": 570.0 }
  ],
  "points_board": [
    { "x": -1.0, "y": 1.0 },
    { "x": 1.0, "y": 1.0 },
    { "x": 1.0, "y": -1.0 },
    { "x": -1.0, "y": -1.0 }
  ]
}
```

Exemple PowerShell :

```powershell
$payload = @{
  camera_id = 1
  offset_angle_deg = 0.0
  points_image = @(
    @{ x = 230.0; y = 230.0 }
    @{ x = 570.0; y = 230.0 }
    @{ x = 570.0; y = 570.0 }
    @{ x = 230.0; y = 570.0 }
  )
  points_board = @(
    @{ x = -1.0; y = 1.0 }
    @{ x = 1.0; y = 1.0 }
    @{ x = 1.0; y = -1.0 }
    @{ x = -1.0; y = -1.0 }
  )
} | ConvertTo-Json -Depth 5

Invoke-RestMethod `
  -Method Post `
  -Uri http://127.0.0.1:8090/commands/calibrate `
  -ContentType "application/json" `
  -Body $payload
```

## 12. Envoi des evenements au backend

Le service offline envoie les resultats vers :

```text
POST /vision/events
```

L'URL est definie dans `fixtures/service_config.json`.

Le publisher HTTP est asynchrone :

- les evenements passent par une file interne
- les envois se font dans un thread dedie
- il y a plusieurs tentatives en cas d'echec

## 13. Fichier de configuration du service

Le fichier d'exemple fourni est :

```text
fixtures/service_config.json
```

Structure actuelle :

```json
{
  "execution": {
    "scenario_root": "fixtures",
    "allow_single_source": true,
    "debug_save_intermediates": false,
    "debug_output_root": "build/debug_output",
    "run_all_on_start": true
  },
  "pipeline": {
    "diff_threshold": 30,
    "blur_kernel_size": 5,
    "morph_kernel_size": 3,
    "min_contour_area": 40,
    "max_contour_area": 100000,
    "outlier_threshold": 0.08,
    "quality_floor": 0.2
  },
  "backend": {
    "post_url": "http://127.0.0.1:8080/vision/events",
    "service_host": "127.0.0.1",
    "service_port": 8090,
    "post_timeout_ms": 500,
    "post_retry_count": 3
  }
}
```

## 14. Format des fixtures offline

Chaque scenario est un dossier autonome.

Exemple :

```text
fixtures/single_20/
```

Contenu attendu :

- `reference.png`
- `snapshot.png`
- `calibration.json` ou `calibration.fs.json`
- `scenario.json`
- `expected.json`

### 14.1 `scenario.json`

Contient :

- le nom du scenario
- l'identifiant camera logique
- le masque du plateau
- d'eventuels overrides locaux

Exemple :

```json
{
  "name": "single_20",
  "camera_id": 1,
  "save_debug_images": false,
  "mask": {
    "center_x": 400,
    "center_y": 400,
    "radius_px": 340
  }
}
```

### 14.2 `expected.json`

Contient le resultat attendu.

Le moteur compare par **sous-ensemble JSON** :

- seuls les champs presents dans `expected.json` sont exiges
- les valeurs numeriques acceptent une tolerance

Exemple :

```json
{
  "event": "shot_detected",
  "status": "valid",
  "segment": "S20",
  "ring": "SINGLE",
  "score": 20
}
```

### 14.3 `calibration.json`

Format logique actuel :

```json
{
  "camera_id": 1,
  "offset_angle_deg": 0.0,
  "points_image": [
    { "x": 230.0, "y": 230.0 },
    { "x": 570.0, "y": 230.0 },
    { "x": 570.0, "y": 570.0 },
    { "x": 230.0, "y": 570.0 }
  ],
  "points_board": [
    { "x": -1.0, "y": 1.0 },
    { "x": 1.0, "y": 1.0 },
    { "x": 1.0, "y": -1.0 },
    { "x": -1.0, "y": -1.0 }
  ]
}
```

## 15. Scenarios fournis

Le lot actuel contient :

- `single_20`
- `double_20`
- `triple_20`
- `outer_bull`
- `inner_bull`
- `miss`
- `no_change`
- `noise_only`
- `ambiguous_contour`

## 16. Pipeline de traitement offline

Le pipeline applique les etapes suivantes :

1. chargement de `reference.png` et `snapshot.png`
2. chargement de la calibration
3. conversion en niveaux de gris
4. flou gaussien
5. difference absolue `reference / snapshot`
6. seuillage
7. morphologie
8. restriction au masque du plateau
9. extraction des contours
10. choix du meilleur candidat
11. projection du point image en `board_normalized`
12. fusion des impacts
13. scoring
14. emission d'un JSON final

## 17. Heuristique actuelle de detection

Le detecteur V1 choisit un contour a partir de :

- son aire
- sa longueur apparente
- son elongation
- sa position plausible sur le plateau

Le detecteur gere maintenant explicitement un cas ambigu :

- si deux candidats ont des scores tres proches
- et s'ils sont suffisamment eloignes
- alors l'evenement devient `shot_invalid`

Cela a ete ajoute pour faire passer correctement le scenario `ambiguous_contour`.

## 18. Fusion actuelle

Le moteur de fusion est deja present meme si la phase actuelle est mono-source logique.

Politique actuelle :

- `0` impact valide : `shot_invalid`
- `1` impact valide : autorise en mode offline si `allow_single_source = true`
- `2+` impacts valides : regroupement par proximite et moyenne ponderee

## 19. Score et repere cible

Le `ScoreEngine` sait gerer :

- `MISS`
- `SINGLE`
- `DOUBLE`
- `TRIPLE`
- `OUTER_BULL`
- `INNER_BULL`

Le score est calcule a partir du point `board_normalized`.

## 20. Format JSON emis

### 20.1 Exemple `shot_detected`

```json
{
  "event": "shot_detected",
  "schema_version": 1,
  "shot_id": "shot-000008",
  "timestamp_ms": 1775248703198,
  "status": "valid",
  "segment": "S20",
  "score": 20,
  "sector": 20,
  "ring": "SINGLE",
  "multiplier": 1,
  "board_point": {
    "x": -0.002941176470588447,
    "y": 0.28823529411764726,
    "space": "board_normalized"
  },
  "confidence": 0.9279374250676375,
  "processing_ms": 638,
  "cameras_expected": 1,
  "cameras_used": 1,
  "camera_impacts": [
    {
      "camera_id": 1,
      "x": -0.002941176470588447,
      "y": 0.28823529411764726,
      "quality": 0.9279374250676375,
      "used_in_fusion": true,
      "valid": true,
      "reason": ""
    }
  ]
}
```

### 20.2 Exemple `shot_invalid`

```json
{
  "event": "shot_invalid",
  "schema_version": 1,
  "shot_id": "shot-000001",
  "timestamp_ms": 1775248698029,
  "status": "invalid",
  "segment": "MISS",
  "score": 0,
  "sector": null,
  "ring": "MISS",
  "multiplier": null,
  "board_point": {
    "x": 0.0,
    "y": 0.0,
    "space": "board_normalized"
  },
  "confidence": 0.0,
  "processing_ms": 735,
  "cameras_expected": 1,
  "cameras_used": 0,
  "camera_impacts": [
    {
      "camera_id": 1,
      "x": 0.0,
      "y": 0.0,
      "quality": 0.0,
      "used_in_fusion": false,
      "valid": false,
      "reason": "ambiguous_contour"
    }
  ],
  "reason": "no_valid_impacts"
}
```

## 21. Commandes de validation recommandees

### 21.1 Validation rapide

```powershell
.\tools\build_debug.ps1 -RunTests
.\build\debug\vision_replay.exe fixtures
```

### 21.2 Validation API locale

Terminal 1 :

```powershell
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
```

Terminal 2 :

```powershell
.\build\debug\vision_service.exe fixtures\service_config.json
```

Terminal 3 :

```powershell
Invoke-RestMethod -Uri http://127.0.0.1:8090/healthcheck
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/stop
```

Ensuite verifier :

- `build/mock_backend_events.jsonl`
- ou le stdout du `mock_backend`

## 22. Debug visuel

Quand le mode debug est active, le moteur peut sauvegarder :

- `reference.png`
- `snapshot.png`
- `gray_reference.png`
- `gray_snapshot.png`
- `diff.png`
- `binary_mask.png`
- `annotated.png`
- `debug_log.txt`

Par defaut, la sortie est placee dans :

```text
build/debug_output
```

## 23. Fichiers importants du code

### 23.1 Noyau metier

- `include/visiondarts/core/types.hpp`
- `src/core/types.cpp`
- `src/core/config.cpp`
- `src/core/json_utils.cpp`
- `src/core/board_model.cpp`

### 23.2 Vision

- `include/visiondarts/vision/calibration.hpp`
- `src/vision/calibration.cpp`
- `src/vision/replay.cpp`
- `src/vision/impact_detector.cpp`
- `src/vision/fusion_engine.cpp`

### 23.3 Application

- `src/app/offline_engine.cpp`
- `src/app/service_controller.cpp`
- `src/api/event_publisher.cpp`

## 24. Limitations actuelles

- pas de camera physique
- pas de synchro multi-camera reelle
- pas de `CameraFrameSource`
- pas de Raspberry Pi
- pas de logique complete de partie `301`
- heuristiques de vision encore simples
- fixtures surtout synthetiques a ce stade

## 25. Suite logique du projet

Les prochaines etapes naturelles seront :

- enrichir les fixtures
- durcir le detecteur sur de vraies images
- introduire l'acquisition camera reelle
- valider la multi-camera
- integrer le backend reel du collegue
- preparer la cible Raspberry Pi

## 26. Resume court

Si tu veux juste lancer le projet rapidement :

```powershell
.\tools\build_debug.ps1 -RunTests
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
.\build\debug\vision_service.exe fixtures\service_config.json
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Le moteur traitera alors le lot de fixtures offline et enverra les JSON de tir au mock backend local.
