# Harmonie des responsabilites

## 1. Regle simple : qui fait quoi

### `OpenCV`

Utilise OpenCV pour :

- lire/ecrire les images et videos ;
- manipuler `cv::Mat` ;
- transformer, filtrer, detecter, mesurer ;
- dessiner, annoter, encoder ;
- gerer les structures vision natives.

### `nlohmann::json`

Utilise `nlohmann::json` pour :

- config applicative ;
- parametres de pipeline ;
- resultats metier ;
- metadonnees ;
- format d'echange avec des API, UI ou outils externes.

## 2. Ce qu'il faut eviter

Ne pas representer une image complete comme :

```json
{"pixels":[[...],[...],[...]]}
```

## 3. Format conseille en pratique

Pattern robuste :

- image ou masque sur disque via OpenCV ;
- JSON pour les chemins, dimensions, seuils, labels, scores, ROIs, temps d'execution, etc.

Exemple :

```json
{
  "input_image": "input.jpg",
  "output_image": "output.png",
  "preprocess": {
    "grayscale": true,
    "resize": {"width": 640, "height": 480}
  },
  "detections": [
    {"label": "part", "score": 0.98, "bbox": {"x": 40, "y": 25, "width": 120, "height": 80}}
  ]
}
```

## 4. Convertir des types OpenCV en JSON

Le plus propre est d'ecrire `to_json` / `from_json` pour les petits types de geometrie.

### `cv::Point`

```cpp
inline void to_json(nlohmann::json& j, const cv::Point& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}};
}

inline void from_json(const nlohmann::json& j, cv::Point& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
}
```

### `cv::Size`

```cpp
inline void to_json(nlohmann::json& j, const cv::Size& s)
{
    j = nlohmann::json{{"width", s.width}, {"height", s.height}};
}

inline void from_json(const nlohmann::json& j, cv::Size& s)
{
    j.at("width").get_to(s.width);
    j.at("height").get_to(s.height);
}
```

### `cv::Rect`

```cpp
inline void to_json(nlohmann::json& j, const cv::Rect& r)
{
    j = nlohmann::json{
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height}
    };
}

inline void from_json(const nlohmann::json& j, cv::Rect& r)
{
    j.at("x").get_to(r.x);
    j.at("y").get_to(r.y);
    j.at("width").get_to(r.width);
    j.at("height").get_to(r.height);
}
```

## 5. Et pour `cv::Mat` ?

Ne serialize pas un `cv::Mat` complet en JSON brut par defaut.

Choix raisonnables :

- petite matrice numerique de calibration : JSON manuel acceptable ;
- image reelle : fichier image ou buffer encode ;
- structure OpenCV pure : `cv::FileStorage`.

## 6. `cv::FileStorage` vs `nlohmann::json`

### `cv::FileStorage`

Tres bien pour :

- matrices OpenCV ;
- calibration camera ;
- structures internes de vision.

### `nlohmann::json`

Tres bien pour :

- format applicatif lisible ;
- API REST ;
- configuration humaine ;
- resultats metier ;
- interop generale.

## 7. Choix recommande

Dans un projet combine :

- `nlohmann::json` pour le document principal ;
- `cv::FileStorage` uniquement pour les morceaux explicitement OpenCV si besoin.

Exemple :

- `config.json` : chemins, seuils, mode debug, liste d'operations ;
- `camera_params.json` via `FileStorage` : matrices de calibration ;
- `report.json` via `nlohmann::json` : detections, temps, qualite, chemins de sortie.

## 8. Si tu dois transporter une image dans un payload JSON

Le moins mauvais compromis est souvent :

1. encoder l'image avec `cv::imencode()` ;
2. convertir le buffer en Base64 ;
3. stocker aussi les metadonnees utiles.

Mais en pratique, si tu controles les 2 bouts :

- prefere un fichier ;
- ou un binaire separable du JSON.

## 9. Pense aux metadonnees explicites

Quand tu enregistres des resultats lies a une image, ajoute clairement :

- `width`
- `height`
- `channels`
- `color_space`
- `opencv_type`
- `source_path`

Surtout si le JSON quitte le perimetre C++.
