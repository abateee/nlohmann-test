# Documentation locale de `nlohmann::json`

Ce dossier contient une documentation en francais, orientee pratique, pour la bibliotheque C++ `nlohmann::json`.

## Ce que c'est

`nlohmann::json` est une bibliotheque JSON moderne pour C++ :

- integration tres simple ;
- API tres proche des conteneurs STL ;
- un simple `#include <nlohmann/json.hpp>` suffit dans le cas le plus courant ;
- support de C++11 et des standards plus recents ;
- licence MIT.

## Version de reference utilisee ici

Cette documentation prend comme base la release stable officiellement marquee `Latest` sur GitHub :

- `nlohmann/json` `v3.12.0`
- date de release : `2025-04-11`

Note importante :

- le site `json.nlohmann.me` publie parfois des pages qui mentionnent des ajouts plus recents que la derniere release stable ;
- quand c'est le cas, ce dossier le signale explicitement.

## Plan du dossier

- [`01-installation-et-integration.md`](01-installation-et-integration.md) : installation, include, CMake, package managers.
- [`02-prise-en-main.md`](02-prise-en-main.md) : creation, parsing, acces, modification, iteration, conversions.
- [`03-reference-pratique.md`](03-reference-pratique.md) : reference compacte des fonctions et idiomes les plus utiles.
- [`04-fonctions-avancees.md`](04-fonctions-avancees.md) : types personnalises, enums, JSON Pointer, Patch, Merge Patch, formats binaires, callbacks.
- [`05-bonnes-pratiques-et-pieges.md`](05-bonnes-pratiques-et-pieges.md) : erreurs frequentes, comportements subtils, recommandations.
- [`exemples.cpp`](exemples.cpp) : fichier C++ de demonstration.
- [`sources-officielles.md`](sources-officielles.md) : sources officielles consultees.

## Autres documentations dans `doc/`

- [`opencv/`](opencv/README.md) : documentation locale OpenCV en C++.
- [`openjson/`](openjson/README.md) : usage combine OpenCV + `nlohmann::json`.

## Demarrage rapide

### Include minimal

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;
```

### Premier objet JSON

```cpp
#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    json j = {
        {"name", "Alice"},
        {"age", 30},
        {"admin", false},
        {"tags", {"cpp", "json"}}
    };

    std::cout << std::setw(2) << j << '\n';
}
```

### Parsing rapide

```cpp
json j = json::parse(R"({
    "host": "localhost",
    "port": 8080
})");

std::string host = j.at("host").get<std::string>();
int port = j.value("port", 80);
```

## Quand utiliser quoi

- Ecriture simple ou construction progressive : `operator[]`
- Lecture securisee si la cle doit exister : `at()`
- Lecture tolerante avec valeur par defaut : `value()`
- Verification prealable : `contains()`
- Conversion explicite : `get<T>()` ou `get_to(x)`
- Affichage compact : `dump()`
- Affichage lisible : `dump(2)` ou `std::setw(2) << j`
- Conservation de l'ordre d'insertion : `nlohmann::ordered_json`
- Validation sans exception : `json::accept(...)`
- Transformation de schema/document : `patch()`, `merge_patch()`, `diff()`

## Resume mental

Pense a `nlohmann::json` comme a un type dynamique C++ qui peut contenir :

- `null`
- `bool`
- entier signe
- entier non signe
- flottant
- chaine
- objet
- tableau
- valeur binaire

L'API ressemble a un melange de `std::map`, `std::vector` et de conversions explicites.

## Lecture conseillee

Si tu debutes :

1. lis [`02-prise-en-main.md`](02-prise-en-main.md)
2. garde [`03-reference-pratique.md`](03-reference-pratique.md) ouvert a cote
3. lis ensuite [`05-bonnes-pratiques-et-pieges.md`](05-bonnes-pratiques-et-pieges.md)

Si tu veux aller vite :

1. lis la section "Quand utiliser quoi" ci-dessus
2. ouvre [`03-reference-pratique.md`](03-reference-pratique.md)
3. copie adapte [`exemples.cpp`](exemples.cpp)
