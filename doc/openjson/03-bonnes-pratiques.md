# Bonnes pratiques

## 1. Garde les types OpenCV a la frontiere technique

Les types comme :

- `cv::Mat`
- `cv::Rect`
- `cv::Point`
- `cv::Size`

sont excellents dans le pipeline de vision.

Mais pour les echanges applicatifs :

- convertis vite en structures JSON lisibles ;
- evite de propager `cv::Mat` partout dans le code metier.

## 2. Utilise `at()` pour le JSON critique

Exemple :

```cpp
int width = cfg.at("resize").at("width").get<int>();
```

Pourquoi :

- echec immediat si la config obligatoire manque ;
- erreur plus explicite ;
- moins de comportements implicites.

Reserve `value()` aux vrais defauts acceptables.

## 3. Versionne ton schema JSON

Ajoute par exemple :

```json
{
  "schema_version": 1
}
```

Tres utile si :

- les pipelines evoluent ;
- les noms de champs changent ;
- plusieurs outils lisent les memes fichiers.

## 4. Sauvegarde explicitement l'espace couleur

Ne suppose jamais que le consommateur sait si une image est :

- `BGR`
- `RGB`
- `GRAY`
- `HSV`

Exemple :

```json
{
  "color_space": "BGR"
}
```

## 5. N'utilise pas JSON pour les gros blobs image par defaut

JSON convient tres bien pour :

- scores ;
- labels ;
- boites englobantes ;
- points ;
- parametres.

JSON convient mal pour :

- video brute ;
- images nombreuses ;
- gros masques binaires ;
- grandes cartes de chaleur.

## 6. Si tu serialises une matrice, documente le format

Ajoute au minimum :

- `rows`
- `cols`
- `type`
- `layout`
- `data_encoding`

Exemple :

```json
{
  "rows": 3,
  "cols": 3,
  "type": "CV_64F",
  "layout": "row-major",
  "data_encoding": "plain-array"
}
```

## 7. `cv::FileStorage` et `nlohmann::json` ne servent pas le meme but

Ne cherche pas a imposer un seul outil pour tout.

Pattern propre :

- `nlohmann::json` pour l'orchestration applicative ;
- `FileStorage` pour les artefacts purement OpenCV.

## 8. Pour des sorties diffables, pense a la stabilite

Si tu veux :

- comparer les rapports entre deux runs ;
- versionner les sorties ;
- relire facilement en revue de code ;

alors :

- garde des cles stables ;
- garde des tableaux ordonnes ;
- evite les champs inutiles ou aleatoires ;
- considere `ordered_json` si l'ordre humain compte.

## 9. Centralise les conversions `to_json` / `from_json`

Mets les adaptations OpenCV -> JSON dans un seul header ou module.

Tu y gagnes :

- moins de duplication ;
- moins d'incoherences de schema ;
- maintenance plus simple.

## 10. Checklist rapide

- OpenCV pour les pixels
- JSON pour la config et les resultats
- `at()` pour les champs obligatoires
- `value()` pour les vrais defauts
- ne pas mettre de grosses images brutes dans JSON
- documenter `color_space` et `type`
- centraliser les conversions
