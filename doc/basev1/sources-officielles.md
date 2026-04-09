# Sources officielles

Ce fichier liste les sources officielles consultees pour verifier les choix de cette base V1.

Verification effectuee le `2026-04-03`.

## OpenCV

### `cv::VideoCapture`

Source :

- `https://docs.opencv.org/master/d8/dfe/classcv_1_1VideoCapture.html`

Point retenu :

- la documentation recommande `grab()` puis `retrieve()` quand on veut rapprocher temporellement des images provenant de plusieurs cameras et retarder le cout de decodage

### `cv::FileStorage`

Source :

- `https://docs.opencv.org/4.x/da/d56/classcv_1_1FileStorage.html`

Point retenu :

- `FileStorage` convient bien pour lire et ecrire des structures OpenCV, y compris en format `JSON`

### homographie et projection

Sources :

- `https://docs.opencv.org/4.x/d7/dff/tutorial_feature_homography.html`
- `https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html`

Points retenus :

- l'homographie est bien le bon outil pour relier deux plans
- `findHomography` est la voie robuste si on veut un calcul d'homographie a partir de points et eventuellement de `RANSAC`

## nlohmann::json

### documentation generale

Source :

- `https://json.nlohmann.me/`

Points retenus :

- `nlohmann::json` convient pour les payloads applicatifs, la config et la serialisation de resultats

### acces et serialisation

Sources :

- `https://json.nlohmann.me/api/basic_json/at/`
- `https://json.nlohmann.me/api/basic_json/value/`
- `https://json.nlohmann.me/api/basic_json/contains/`
- `https://json.nlohmann.me/api/basic_json/dump/`
- `https://json.nlohmann.me/features/arbitrary_types/`

Points retenus :

- `at()` pour lecture stricte
- `value()` pour lecture avec defaut
- `contains()` pour verifier des cles
- `dump()` pour produire des payloads JSON
- conversions personnalisees possibles pour des types applicatifs

## Usage de ces sources dans cette base

Elles ont servi a verifier surtout :

- la strategie de capture multi-camera
- la separation `OpenCV` vs `JSON`
- le choix de l'homographie comme base spatiale
- la forme generale du contrat JSON et de la serialisation
