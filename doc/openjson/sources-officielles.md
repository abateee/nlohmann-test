# Sources officielles

Documentation redigee a partir des sources officielles consultees le `2026-03-27`.

## Releases stables verifiees

- OpenCV releases : <https://github.com/opencv/opencv/releases>
  - release marquee `Latest` au moment de la consultation : `4.13.0`
  - date de release : `2025-12-31`
- `nlohmann::json` releases : <https://github.com/nlohmann/json/releases>
  - release marquee `Latest` au moment de la consultation : `v3.12.0`
  - date de release : `2025-04-11`

## Documentation officielle OpenCV

- Docs 4.x : <https://docs.opencv.org/4.x/>
- `cv::Mat` : <https://docs.opencv.org/4.x/d6/d6d/tutorial_mat_the_basic_image_container.html>
- `imgcodecs` : <https://docs.opencv.org/4.x/d4/da8/group__imgcodecs.html>
- conversions couleur : <https://docs.opencv.org/4.x/d8/d01/group__imgproc__color__conversions.html>
- `cv::FileStorage` : <https://docs.opencv.org/4.x/da/d56/classcv_1_1FileStorage.html>

## Documentation officielle `nlohmann::json`

- accueil : <https://json.nlohmann.me/>
- types personnalises : <https://json.nlohmann.me/features/arbitrary_types/>
- `parse()` : <https://json.nlohmann.me/api/basic_json/parse/>
- `dump()` : <https://json.nlohmann.me/api/basic_json/dump/>
- `value()` : <https://json.nlohmann.me/api/basic_json/value/>
- `at()` : <https://json.nlohmann.me/features/element_access/checked_access/>

## Note d'usage

La philosophie de ce dossier est volontairement pragmatique :

- OpenCV pour les donnees image et structures vision ;
- `nlohmann::json` pour la couche configuration, orchestration et resultats.
