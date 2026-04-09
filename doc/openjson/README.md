# Documentation combinee `OpenCV` + `nlohmann::json`

Ce dossier explique comment utiliser `OpenCV` et `nlohmann::json` ensemble, proprement, en C++.

## Pourquoi combiner ces 2 bibliotheques

Le partage de responsabilites le plus sain est :

- `OpenCV` gere les pixels, matrices, videos, algorithmes de vision ;
- `nlohmann::json` gere la configuration, les metadonnees, les resultats, les payloads d'API et les fichiers applicatifs.

Autrement dit :

- OpenCV fait le traitement ;
- JSON decrit, configure et transporte ce traitement.

## Versions de reference utilisees ici

- OpenCV : `4.13.0`, release stable verifiee du `2025-12-31`
- `nlohmann::json` : `3.12.0`, release stable verifiee du `2025-04-11`

## Plan du dossier

- [`01-harmonie-des-responsabilites.md`](01-harmonie-des-responsabilites.md) : quoi stocker ou non dans JSON, conversions utiles, role de `cv::FileStorage`.
- [`02-pipelines-et-serialisation.md`](02-pipelines-et-serialisation.md) : pipelines OpenCV pilotes par JSON, rapports de sortie, serialisation de metadonnees.
- [`03-bonnes-pratiques.md`](03-bonnes-pratiques.md) : recommandations d'architecture et pieges frequents.
- [`exemples.cpp`](exemples.cpp) : exemples C++ combinant les 2 bibliotheques.
- [`sources-officielles.md`](sources-officielles.md) : sources officielles consultees.

## Regle d'or

Dans la majorite des projets :

- Je pense on va : stocker les images comme fichiers ou buffers encodes ;
- stocker les parametres et resultats en JSON ;

## Pipeline mental recommande

1. lire un fichier JSON de configuration ;
2. le parser avec `nlohmann::json` ;
3. lancer le pipeline OpenCV selon cette config ;
4. sauver l'image de sortie avec OpenCV ;
5. sauver le rapport, les mesures et les detections en JSON.

## Exemple mental simple

```json
{
  "input": "input.jpg",
  "pipeline": [
    {"op": "cvtColor", "code": "BGR2GRAY"},
    {"op": "resize", "width": 640, "height": 480},
    {"op": "canny", "threshold1": 80, "threshold2": 160}
  ],
  "output_image": "edges.png",
  "output_report": "report.json",
  "output_timestamp": " {either inside report json or } : timestamp.txt"
}
```

Le JSON decrit le pipeline ; OpenCV execute. 
Ca reste a voir, tout simplement un exemple des possibilitées.
