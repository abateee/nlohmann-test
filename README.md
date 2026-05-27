# Vision Darts Offline + Live Windows

## 1. Objet du projet

Ce depot contient le moteur `C++ / OpenCV` pour un systeme de score automatique de flechettes.

La branche `feature/live-camera-windows` ajoute une V1 d'acquisition camera reelle Windows, tout en conservant le mode **offline / replay** existant.

Le perimetre actuel est :

- mode `replay` pour rejouer les fixtures existantes
- mode `live` Windows pour ouvrir 1 a 3 cameras USB via OpenCV
- calibration camera par fichier JSON
- service HTTP local pour piloter le moteur
- publication des tirs vers un backend HTTP local, notamment Flechette via `POST /vision/events`
- pas de logique complete de partie `301` dans ce depot
- pas de backend metier ici, seulement un `mock_backend` pour valider les echanges HTTP quand Flechette n'est pas lance

Le moteur actuel sait deja faire les choses suivantes :

- charger des scenarios de test a partir de paires d'images `reference + snapshot`
- ouvrir des cameras USB configurees en mode live Windows
- capturer une reference live par camera
- charger une calibration par homographie
- detecter un impact par difference d'images
- projeter l'impact dans le repere `board_normalized`
- calculer le score d'un tir
- produire un JSON pret pour un backend local
- exposer une API HTTP locale pour piloter le service
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
- le mode live Windows compile
- l'outil de calibration UI Windows compile
- l'integration HTTP vers Flechette est compatible avec `POST http://127.0.0.1:3010/vision/events`

Points non encore valides physiquement :

- ouverture reelle de 1 a 3 cameras USB sur poste Windows
- qualite de detection sur vraies images de cible
- robustesse de la stabilisation apres tir reel
- execution sur Raspberry Pi

Resultats verifies :

- `ctest --preset debug` : OK
- `build/debug/vision_replay.exe fixtures` : OK
- `mock_backend` recoit 9 evenements sur le lot de fixtures courant : OK
- `build/debug/vision_live_calibrate_ui.exe` affiche son usage : OK

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
  calibration, replay, acquisition camera live, detection d'impact, fusion
- `api`
  publication HTTP des evenements
- `app`
  orchestration replay/live et controle de service

## 4. Conventions de travail

- Le repere de sortie officiel est `board_normalized`.
- Le centre de la cible est `(0, 0)`.
- Le rayon exterieur du double vaut `1.0`.
- Les commentaires du code doivent etre rediges en francais.
- La logique de partie `301` n'est pas geree ici.
- Ce depot fournit un moteur `vision + scoring de tir`, pas un backend metier complet.
- Le mode `replay` doit rester fonctionnel pendant les evolutions du mode `live`.

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

### 6.3 Installation Windows

Le script de verification local est :

```powershell
.\tools\setup_windows.ps1
```

Pour installer les dependances declarees dans `vcpkg.json` :

```powershell
.\tools\setup_windows.ps1 -InstallDeps
```

Il verifie :

- Visual Studio Build Tools
- CMake
- Ninja
- `VCPKG_ROOT`
- le triplet `x64-windows`

Les chemins peuvent etre fournis par variables d'environnement :

```powershell
$env:VISIONDARTS_VSDEVCMD="C:\...\VsDevCmd.bat"
$env:VISIONDARTS_CMAKE="C:\...\cmake.exe"
$env:VCPKG_ROOT="C:\...\vcpkg"
```

`CMakePresets.json` utilise `VCPKG_ROOT` pour trouver OpenCV et les dependances vcpkg.

## 7. Dependances vcpkg

Le fichier `vcpkg.json` declare :

- `opencv4`
- `nlohmann-json`
- `cpp-httplib`

Le port `opencv4` inclut les modules utilises par le replay et par le live Windows : `core`, `imgproc`, `imgcodecs`, `calib3d` et `videoio`.

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
- `vision_camera_diagnostics.exe`
- `vision_live_calibrate.exe`
- `vision_live_calibrate_ui.exe`
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

- lance le service HTTP local
- charge une config JSON
- traite soit les scenarios replay, soit la boucle camera live selon `execution.mode`
- envoie les evenements vers un backend HTTP local

Usage replay :

```powershell
.\build\debug\vision_service.exe fixtures\service_config.json
```

Usage live Windows :

```powershell
.\build\debug\vision_service.exe config\live_windows.json
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

### 10.5 `vision_camera_diagnostics.exe`

Role :

- verifier que Windows/OpenCV voit les cameras USB
- tester les cameras configurees dans `config/live_windows.json`
- capturer une image par camera dans `build/camera_diagnostics`
- produire un rapport `camera_diagnostics.json`

Tester la config 3 cameras :

```powershell
.\build\debug\vision_camera_diagnostics.exe config\live_windows.json
```

Scanner les index OpenCV de `0` a `10` :

```powershell
.\build\debug\vision_camera_diagnostics.exe --scan 10
```

Tester la config et scanner en meme temps :

```powershell
.\build\debug\vision_camera_diagnostics.exe config\live_windows.json --scan 10
```

### 10.6 `vision_live_calibrate_ui.exe`

Role :

- ouvre une camera configuree dans `config/live_windows.json`
- affiche le flux dans une fenetre Windows native
- permet de cliquer les 4 points de calibration
- sauvegarde la calibration dans le fichier JSON de la camera

Ordre des points :

1. haut double
2. droite double
3. bas double
4. gauche double

Touches :

- clic gauche : ajouter un point
- `U` : annuler le dernier point
- `R` : recommencer
- `S` : sauvegarder quand 4 points sont poses
- `Q` ou `ESC` : quitter

Calibrer une camera :

```powershell
.\build\debug\vision_live_calibrate_ui.exe config\live_windows.json 1
```

Calibrer toutes les cameras activees :

```powershell
.\build\debug\vision_live_calibrate_ui.exe config\live_windows.json --all
```

### 10.7 `vision_live_calibrate.exe`

Role :

- outil CLI de secours pour capturer une image de calibration
- demander les coordonnees des 4 points dans le terminal
- sauvegarder la calibration sans utiliser l'UI souris

Usage :

```powershell
.\build\debug\vision_live_calibrate.exe config\live_windows.json 1
```

## 11. API HTTP locale

Le service expose les endpoints suivants en mode `replay` et en mode `live` :

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
  "mode": "live",
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

- en mode `replay`, demarre le traitement du lot de scenarios configure
- en mode `live`, ouvre les cameras si besoin, capture une reference stable et demarre la boucle de surveillance

### 11.3 `POST /commands/stop`

Exemple :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/stop
```

Effet :

- arrete proprement le worker de traitement
- en mode `live`, libere les cameras

### 11.4 `POST /commands/reset-reference`

Exemple :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/reset-reference
```

Effet :

- en mode `live`, recapture une reference stable
- en mode `replay`, l'endpoint est conserve pour compatibilite mais repond `501`
- le payload retourne `accepted=false` et `error="unsupported_in_offline_mode"`

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

Remarque :

- un JSON mal forme retourne maintenant `HTTP 400`
- le payload d'erreur contient `accepted=false` et `error="invalid_json"`
- en mode `live`, la calibration est sauvegardee dans le fichier `calibration_path` de la camera concernee
- en mode `replay`, la commande conserve le comportement de compatibilite du service offline

## 12. Envoi des evenements au backend

Le service envoie les resultats vers :

```text
POST /vision/events
```

L'URL est definie dans la configuration :

- `fixtures/service_config.json` pour le mode replay
- `config/live_windows.json` pour le mode live Windows

Pour l'integration Flechette, l'URL attendue est :

```text
http://127.0.0.1:3010/vision/events
```

Le publisher HTTP est asynchrone :

- les evenements passent par une file interne
- les envois se font dans un thread dedie
- il y a plusieurs tentatives en cas d'echec

## 13. Fichiers de configuration du service

### 13.1 Config replay

Le fichier d'exemple replay est :

```text
fixtures/service_config.json
```

Structure :

```json
{
  "execution": {
    "mode": "replay",
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
    "post_url": "http://127.0.0.1:3010/vision/events",
    "service_host": "127.0.0.1",
    "service_port": 8090,
    "post_timeout_ms": 500,
    "post_retry_count": 3
  }
}
```

### 13.2 Config live Windows

Le fichier d'exemple live Windows est :

```text
config/live_windows.json
```

La config fournie dans cette branche est prete pour 3 cameras USB Windows :

- camera logique `1` sur `device_index` 0
- camera logique `2` sur `device_index` 1
- camera logique `3` sur `device_index` 2
- calibrations separees dans `config/calibration-camera-1.json`, `2` et `3`

Exemple minimal 1 camera :

```json
{
  "execution": {
    "mode": "live",
    "run_all_on_start": false
  },
  "live": {
    "reference_stability_frames": 5,
    "shot_change_threshold": 8.0,
    "stabilization_frames": 5,
    "loop_sleep_ms": 50,
    "min_ms_between_shots": 750
  },
  "cameras": [
    {
      "camera_id": 1,
      "device_index": 0,
      "width": 1280,
      "height": 720,
      "fps": 30,
      "calibration_path": "config/calibration-camera-1.json",
      "enabled": true
    }
  ],
  "backend": {
    "post_url": "http://127.0.0.1:3010/vision/events",
    "service_host": "127.0.0.1",
    "service_port": 8090,
    "post_timeout_ms": 500,
    "post_retry_count": 3
  }
}
```

Exemple 3 cameras :

```json
{
  "cameras": [
    {
      "camera_id": 1,
      "device_index": 0,
      "width": 1280,
      "height": 720,
      "fps": 30,
      "calibration_path": "config/calibration-camera-1.json",
      "enabled": true
    },
    {
      "camera_id": 2,
      "device_index": 1,
      "width": 1280,
      "height": 720,
      "fps": 30,
      "calibration_path": "config/calibration-camera-2.json",
      "enabled": true
    },
    {
      "camera_id": 3,
      "device_index": 2,
      "width": 1280,
      "height": 720,
      "fps": 30,
      "calibration_path": "config/calibration-camera-3.json",
      "enabled": true
    }
  ]
}
```

### 13.3 Champs live importants

- `execution.mode`
  vaut `replay` ou `live`
- `execution.run_all_on_start`
  peut lancer le traitement automatiquement au demarrage du service
- `live.reference_stability_frames`
  nombre de frames utilisees pour capturer une reference stable
- `live.shot_change_threshold`
  seuil de changement global qui declenche l'analyse d'un tir
- `live.stabilization_frames`
  nombre de frames attendues apres changement avant de figer le snapshot
- `live.loop_sleep_ms`
  pause entre deux cycles de surveillance
- `live.min_ms_between_shots`
  garde-fou pour eviter plusieurs tirs detectes sur le meme changement
- `cameras[].device_index`
  index OpenCV de la camera USB Windows
- `cameras[].calibration_path`
  fichier JSON lu et ecrit par la calibration live
- en mode `live`, la config doit avoir entre 1 et 3 cameras actives
- les `camera_id` actifs doivent etre uniques
- les `device_index` actifs doivent etre uniques

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
- `expected.json` pour le mode replay avec validation

Remarque :

- `expected.json` est optionnel pour le service offline quand `compare_expected = false`

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

## 16. Pipeline de traitement

### 16.1 Pipeline replay

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

### 16.2 Pipeline live Windows

Le pipeline live applique les etapes suivantes :

1. ouverture des cameras activees dans `config/live_windows.json`
2. chargement des calibrations JSON par camera
3. capture d'une reference stable par camera
4. surveillance automatique des frames courantes
5. detection d'un changement significatif par difference reference/snapshot
6. attente de stabilisation de la scene
7. detection d'impact par camera
8. projection des impacts valides en `board_normalized`
9. fusion multi-camera
10. scoring
11. publication du JSON `shot_detected` ou `shot_invalid`
12. attente de retour stable, puis renouvellement de la reference

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

### 21.3 Validation live Windows avec Flechette

Avant de calibrer ou lancer le service live, verifier que les cameras sont bien visibles :

```powershell
.\build\debug\vision_camera_diagnostics.exe config\live_windows.json
```

Le rapport et les captures sont ecrits dans :

```text
build/camera_diagnostics
```

Terminal 1, demarrer Flechette :

```powershell
cd "C:\Users\pcben\Desktop\Flechettes API\flechette\API\joueur"
npm start
```

Terminal 2, lancer le service vision live :

```powershell
cd "C:\Users\pcben\Desktop\nlohmann test"
.\build\debug\vision_service.exe config\live_windows.json
```

Ou via le script raccourci :

```powershell
.\tools\run_live_windows.ps1 -Config config\live_windows.json -Diagnostics
```

Terminal 3, verifier puis demarrer :

```powershell
Invoke-RestMethod -Uri http://127.0.0.1:8090/healthcheck
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/reset-reference
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Verifier cote Flechette :

- `GET /api/vision/health` doit repondre cote backend Flechette
- un tir recu doit creer une ligne dans `lancers`
- `payload_json` doit contenir le JSON complet emis par ce service

### 21.4 Calibration live Windows

Apres branchement des cameras, calibrer chaque camera :

```powershell
.\build\debug\vision_live_calibrate_ui.exe config\live_windows.json --all
```

Ou une seule camera :

```powershell
.\build\debug\vision_live_calibrate_ui.exe config\live_windows.json 1
```

Ensuite verifier que les fichiers suivants existent selon la config :

```text
config/calibration-camera-1.json
config/calibration-camera-2.json
config/calibration-camera-3.json
```

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
- `include/visiondarts/vision/live_camera_source.hpp`
- `src/vision/calibration.cpp`
- `src/vision/live_camera_source.cpp`
- `src/vision/replay.cpp`
- `src/vision/impact_detector.cpp`
- `src/vision/fusion_engine.cpp`

### 23.3 Application

- `include/visiondarts/app/live_engine.hpp`
- `src/app/live_engine.cpp`
- `src/app/offline_engine.cpp`
- `src/app/service_controller.cpp`
- `src/api/event_publisher.cpp`

## 24. Limitations actuelles

- camera live Windows implementee mais pas encore validee physiquement avec le materiel final
- support cible V1 de 1 a 3 cameras USB configurees
- pas de synchronisation materielle entre cameras
- pas de Raspberry Pi
- pas de logique complete de partie `301`
- heuristiques de vision encore simples
- fixtures surtout synthetiques a ce stade
- calibration manuelle/semi-automatique par clic souris, pas encore auto-calibration complete

## 25. Suite logique du projet

Les prochaines etapes naturelles seront :

- enrichir les fixtures
- durcir le detecteur sur de vraies images
- valider l'acquisition camera reelle sur Windows
- valider la multi-camera 2 puis 3 cameras
- mesurer les seuils live sur une vraie cible
- integrer et tester le flux complet avec Flechette en partie 301
- preparer la cible Raspberry Pi

## 26. Resume court

Si tu veux juste valider le replay rapidement :

```powershell
.\tools\build_debug.ps1 -RunTests
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
.\build\debug\vision_service.exe fixtures\service_config.json
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Le moteur traitera alors le lot de fixtures offline et enverra les JSON de tir au mock backend local.

Si tu veux lancer le live Windows :

```powershell
.\tools\build_debug.ps1 -RunTests
.\build\debug\vision_live_calibrate_ui.exe config\live_windows.json --all
.\build\debug\vision_service.exe config\live_windows.json
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Le service ouvrira les cameras configurees, utilisera les calibrations JSON et publiera les tirs vers l'URL `backend.post_url`.
