# Pipelines et serialisation

## 1. Piloter OpenCV avec un fichier JSON

Le pattern le plus efficace consiste a piloter le pipeline avec un document JSON.

Exemple :

```json
{
  "input": "input.jpg",
  "pipeline": [
    {"op": "cvtColor", "code": "BGR2GRAY"},
    {"op": "gaussian_blur", "ksize": 5, "sigma": 1.4},
    {"op": "canny", "threshold1": 80, "threshold2": 160}
  ],
  "output_image": "edges.png",
  "output_report": "report.json"
}
```

Avantages :

- parametres editables sans recompiler ;
- pipelines versionnables ;
- execution reproductible ;
- debug plus simple.

## 2. Exemple de lecture de config

```cpp
std::ifstream in("config.json");
nlohmann::json cfg = nlohmann::json::parse(in);

std::string input_path = cfg.at("input").get<std::string>();
std::string output_path = cfg.value("output_image", std::string("output.png"));
```

## 3. Dispatcher les operations

Exemple de logique :

```cpp
cv::Mat image = cv::imread(input_path, cv::IMREAD_COLOR);
cv::Mat current = image;

for (const auto& step : cfg.at("pipeline"))
{
    const std::string op = step.at("op").get<std::string>();

    if (op == "cvtColor")
    {
        const std::string code = step.at("code").get<std::string>();
        if (code == "BGR2GRAY")
        {
            cv::Mat tmp;
            cv::cvtColor(current, tmp, cv::COLOR_BGR2GRAY);
            current = tmp;
        }
    }
    else if (op == "gaussian_blur")
    {
        int ksize = step.value("ksize", 5);
        double sigma = step.value("sigma", 1.0);
        cv::Mat tmp;
        cv::GaussianBlur(current, tmp, cv::Size(ksize, ksize), sigma);
        current = tmp;
    }
    else if (op == "canny")
    {
        double t1 = step.value("threshold1", 80.0);
        double t2 = step.value("threshold2", 160.0);
        cv::Mat tmp;
        cv::Canny(current, tmp, t1, t2);
        current = tmp;
    }
}
```

## 4. Sauver un rapport JSON en sortie

Une fois le pipeline fini, sauve :

- les chemins ;
- les dimensions ;
- les temps d'execution ;
- les detections / regions / scores ;
- les parametres reels utilises.

Exemple :

```cpp
nlohmann::json report = {
    {"input", input_path},
    {"output_image", output_path},
    {"width", current.cols},
    {"height", current.rows},
    {"channels", current.channels()},
    {"opencv_type", current.type()},
    {"pipeline", cfg.at("pipeline")}
};

std::ofstream out(cfg.at("output_report").get<std::string>());
out << report.dump(2) << '\n';
```

## 5. Resultats de detection : JSON naturel

Les detections se pretent tres bien a JSON :

```json
{
  "detections": [
    {
      "label": "bolt",
      "score": 0.97,
      "bbox": {"x": 112, "y": 58, "width": 44, "height": 30}
    }
  ]
}
```

Ce format est :

- lisible ;
- facile a logger ;
- compatible API et frontend ;
- simple a comparer entre runs.

## 6. Calibration ou petites matrices numeriques

Pour une petite matrice, tu peux faire du JSON manuel :

```json
{
  "camera_matrix": {
    "rows": 3,
    "cols": 3,
    "data": [500.0, 0.0, 320.0, 0.0, 500.0, 240.0, 0.0, 0.0, 1.0]
  }
}
```

Bon cas :

- matrices petites ;
- besoin d'interoperabilite hors OpenCV ;
- lecture humaine possible.

## 7. Si la structure est tres OpenCV, `FileStorage` reste utile

OpenCV peut ecrire du JSON via `cv::FileStorage`.

Exemple :

```cpp
cv::FileStorage fs(
    "camera_params.json",
    cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON
);

fs << "camera_matrix" << camera_matrix;
fs << "distortion" << distortion;
fs.release();
```

Bonne regle :

- `FileStorage` pour les structures OpenCV natives ;
- `nlohmann::json` pour le document applicatif global.

## 8. Encoder une image en memoire si besoin

```cpp
std::vector<uchar> buf;
cv::imencode(".png", image, buf);
```

Ensuite :

- soit tu sauvegardes le buffer tel quel dans un canal binaire ;
- soit tu le convertis en Base64 pour JSON.

Mais attention :

- cela grossit fortement le payload ;
- ce n'est pas le meilleur choix pour des gros volumes d'images.

## 9. Metadonnees utiles a sauvegarder

Pense a enregistrer :

- nom ou chemin source ;
- taille ;
- nombre de canaux ;
- type OpenCV ;
- espace couleur ;
- version du schema JSON ;
- version de l'algorithme ou du modele.

## 10. Resultat final recommande

Dans la plupart des cas :

- `input.jpg` ou `input.mp4` : donnees brutes ;
- `config.json` : parametres d'execution ;
- `output.png` : resultat visuel ;
- `report.json` : mesures, detections, stats, chemins.
