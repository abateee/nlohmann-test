# Commandes Utiles

## Build + tests

```powershell
.\tools\build_debug.ps1 -RunTests
```

## Build seul

```powershell
.\tools\build_debug.ps1
```

## Replay de tous les scenarios

```powershell
.\build\debug\vision_replay.exe fixtures
```

## Replay d'un scenario

```powershell
.\build\debug\vision_replay.exe fixtures\single_20
```

## Replay avec artefacts debug

```powershell
.\tools\run_replay_debug.ps1 -ScenarioPath fixtures -DebugOutput build\debug_output
```

## Lancer le mock backend

```powershell
.\build\debug\mock_backend.exe 8080 build\mock_backend_events.jsonl
```

## Lancer le service offline

```powershell
.\build\debug\vision_service.exe fixtures\service_config.json
```

## Healthcheck

```powershell
Invoke-RestMethod -Uri http://127.0.0.1:8090/healthcheck
```

## Start

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/start
```

## Stop

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/stop
```

## Reset reference

```powershell
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8090/commands/reset-reference
```

## Calibrate

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

## Verification calibration

```powershell
.\build\debug\vision_calibration_check.exe fixtures\single_20\calibration.json 400 400 400 302
```
