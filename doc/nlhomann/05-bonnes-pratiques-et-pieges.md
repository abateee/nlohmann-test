# Bonnes pratiques et pieges

## 1. Ne lis pas tout avec `operator[]`

`operator[]` est excellent pour construire un JSON, mais ce n'est pas toujours le meilleur outil pour lire.

### Pourquoi

- sur objet non const, une cle absente insere `null` ;
- sur tableau non const, un index hors borne peut agrandir le tableau ;
- sur objet const avec cle absente, le comportement est indefini.

### Regle simple

- ecriture et construction : `operator[]`
- lecture stricte : `at()`
- lecture avec defaut : `value()`
- test d'existence : `contains()`

## 2. `value()` est pratique, mais son type de retour depend du defaut

Exemple :

```cpp
json j = {{"big", 18446744073709551615u}};
auto x = j.value("big", 0);
```

Ici, `0` est un `int`, donc la conversion peut etre inattendue.

Bonne pratique :

```cpp
auto x = j.value<std::uint64_t>("big", 0);
```

ou

```cpp
std::uint64_t x = j.value("big", std::uint64_t{0});
```

## 3. Par defaut, l'ordre des cles n'est pas preserve

Si tu veux reproduire fidelement l'ordre d'un fichier JSON :

```cpp
using ordered_json = nlohmann::ordered_json;
auto j = ordered_json::parse(input);
```

Ne fais pas :

```cpp
json j = json::parse(input);
ordered_json k = j;
```

L'ordre est deja perdu au moment du `json::parse`.

## 4. Utilise `contains()` avant un acces permissif

Quand la structure n'est pas garantie :

```cpp
if (j.contains("timeout"))
{
    int timeout = j["timeout"];
}
```

Encore mieux si tu veux une valeur de secours :

```cpp
int timeout = j.value("timeout", 30);
```

## 5. Prefere les conversions explicites

Evite de t'appuyer trop lourdement sur les conversions implicites.

Mieux :

```cpp
std::string name = j.at("name").get<std::string>();
int age = j.at("age").get<int>();
```

Plutot que :

```cpp
std::string name = j["name"];
int age = j["age"];
```

Raison :

- code plus lisible ;
- erreurs de type plus nettes ;
- moins d'ambiguite.

## 6. Active `JSON_DIAGNOSTICS` en debug si tu debogues beaucoup

```cpp
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
```

Cela aide a obtenir des erreurs plus riches. Tres utile sur des documents imbriques.

## 7. Ne desactive `JSON_NOEXCEPTION` que si ton projet l'exige vraiment

Les exceptions font partie de l'ergonomie normale de la bibliotheque.

Desactiver les exceptions complique souvent :

- le code appelant ;
- le debogage ;
- la lecture des erreurs.

Dans un projet standard, garde le comportement par defaut.

## 8. Fais attention a UTF-8

La bibliotheque attend des chaines JSON valides. Lors de `dump()`, une chaine UTF-8 invalide peut lever une `type_error.316` avec le comportement `strict`.

Si tu serialises des donnees d'origine incertaine :

```cpp
std::string safe = j.dump(-1, ' ', false, json::error_handler_t::replace);
```

## 9. N'utilise pas `items()` sur un temporaire

Evite :

```cpp
for (auto& [key, value] : json::parse(text).items())
{
    // dangereux
}
```

Prefere :

```cpp
json j = json::parse(text);
for (auto& [key, value] : j.items())
{
    // ok
}
```

## 10. Les commentaires et les virgules terminales ne sont pas du JSON standard

Oui, la bibliotheque peut etre configuree pour les tolerer.

Mais :

- les commentaires necessitent `ignore_comments = true` ;
- les trailing commas sont documentees comme supportees a partir de `3.12.1` ;
- ce n'est pas du JSON strict.

Bonne pratique :

- si tu controles le format, reste en JSON standard ;
- si tu lis des fichiers "humains", documente clairement cette tolerance.

## 11. `flatten()` / `unflatten()` ne restaurent pas parfaitement les structures vides

La doc officielle precise :

- objets vides et tableaux vides deviennent `null` dans la version plate ;
- `unflatten()` ne peut donc pas toujours reconstruire exactement l'original.

Conclusion :

- tres utile pour des structures "pleines" ;
- a manier avec prudence si les vides ont un sens metier.

## 12. Pour les enums, ne t'appuie pas sur les valeurs entieres par defaut

Par defaut :

- les enums sont serialises comme des entiers ;
- si tu reordonnes l'enum plus tard, tes fichiers ou payloads deviennent fragiles.

Bonne pratique :

```cpp
NLOHMANN_JSON_SERIALIZE_ENUM(Status, {
    {Status::unknown, "unknown"},
    {Status::ready, "ready"},
    {Status::running, "running"}
})
```

## 13. Pour les configurations, combine `value()` et `update()`

Pattern robuste :

```cpp
json defaults = {
    {"host", "localhost"},
    {"port", 8080},
    {"features", {{"logs", true}, {"cache", false}}}
};

json user = json::parse(file_stream);
defaults.update(user, true);
```

Ensuite :

```cpp
int port = defaults.value("port", 8080);
bool logs = defaults["features"].value("logs", true);
```

## 14. Pour du code metier, centralise la conversion JSON

Evite de parser "a la main" partout dans le code.

Mieux :

- une structure C++ claire ;
- un `to_json` / `from_json` associe ;
- des appels `get<T>()` et `json j = t`.

Tu gagnes :

- lisibilite ;
- reutilisation ;
- tests plus simples ;
- schema plus coherent.

## 15. Checklist rapide

- lecture stricte : `at()`
- lecture avec defaut : `value()`
- test d'existence : `contains()`
- construction : `operator[]`
- type explicite : `get<T>()`
- ordre preserve : `ordered_json`
- messages d'erreur plus utiles : `JSON_DIAGNOSTICS`
- configuration robuste : `update(..., true)`
