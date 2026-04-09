# Changelog Executif

- Redige le `2026-04-09 11:07:53 +02:00`
- Validation replay : `expected.json` absent bloque maintenant le mode replay avec `missing_expected_fixture`, mais reste optionnel hors comparaison.
- Calibration : distinction nette entre calibration manquante (`calibration_required`) et calibration corrompue (`vision_error` + `calibration_load_failure`).
- Projection : `offset_angle_deg` est maintenant applique en projection directe et inverse.
- API HTTP : `/commands/calibrate` retourne `400` + `invalid_json` sur JSON invalide.
- API HTTP : `/commands/reset-reference` retourne `501` + `unsupported_in_offline_mode` en mode offline.
- Service : le bind HTTP est explicite et refuse un second demarrage sur le meme port.
- Config : les defaults backend sont unifies sur `http://127.0.0.1:8080/vision/events` et `127.0.0.1:8090`.
- Tests : ajout de regressions C++ pour defaults backend, offset, erreurs calibration, contrats HTTP et exclusivite de port.
- Validation finale : build OK, `ctest` OK, `vision_replay.exe fixtures` OK, smoke test backend/service OK avec `9` evenements recus.
