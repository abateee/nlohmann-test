# Vision Darts Offline - Memo Rapide

## Build

Depuis la racine du projet :

```powershell
.\tools\build_debug.ps1 -RunTests
```

Cette commande :

- configure le projet
- compile les binaires
- copie les DLL runtime dans `build/debug`
- lance les tests unitaires

## Lancer le replay offline

Pour executer tous les scenarios :

```powershell
.\build\debug\vision_replay.exe fixtures
```

Pour un scenario unique :

```powershell
.\build\debug\vision_replay.exe fixtures\single_20
```

Pour generer les images de debug :

```powershell
.\tools\run_replay_debug.ps1 -ScenarioPath fixtures -DebugOutput build\debug_output
```

## Lancer le mock backend

```powershell
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
```

## Lancer le service HTTP local

Dans un autre terminal :

```powershell
.\build\debug\vision_service.exe fixtures\service_config.json
```

## Commander le service

Verifier l'etat :

```powershell
Invoke-RestMethod -Uri http://127.0.0.1:8090/healthcheck
```

Demarrer le traitement :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

Arreter le traitement :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/stop
```

Reinitialiser la reference :

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/reset-reference
```

## Verifier que le backend recoit bien les evenements

Regarder :

- `build/mock_backend_events.jsonl`
- ou le terminal du `mock_backend`

Le lot actuel de fixtures doit produire `9` evenements.

## Si une DLL manque

Relancer simplement :

```powershell
.\tools\build_debug.ps1
```

Les DLL utiles sont recopies dans `build/debug`.

## Fichiers importants

- doc complete : `README.md`
- config service : `fixtures/service_config.json`
- scenarios : `fixtures/`
- sortie debug images : `build/debug_output`

## Rappel de perimetre

Cette version est :

- offline uniquement
- sans camera reelle
- sans Raspberry Pi
- sans logique complete de partie `301`

Le depot fournit le moteur `vision + scoring de tir + JSON + API locale`.
