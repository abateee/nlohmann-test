# Prise en main

## 1. Creer des valeurs JSON

### Objet

```cpp
json j = {
    {"name", "Alice"},
    {"age", 30},
    {"admin", true}
};
```

### Tableau

```cpp
json j = {1, 2, 3, 4};
```

### Valeurs primitives

```cpp
json j_null = nullptr;
json j_bool = true;
json j_int = 42;
json j_uint = 42u;
json j_double = 3.14;
json j_string = "hello";
```

### Construction explicite

```cpp
json obj = json::object();
json arr = json::array();
json bin = json::binary({0x01, 0x02, 0x03});
```

## 2. Identifier le type d'une valeur

```cpp
if (j.is_object()) { /* ... */ }
if (j.is_array()) { /* ... */ }
if (j.is_string()) { /* ... */ }
if (j.is_number()) { /* ... */ }
if (j.is_null()) { /* ... */ }
```

Fonctions utiles :

- `type()`
- `type_name()`
- `is_null()`
- `is_boolean()`
- `is_number()`
- `is_number_integer()`
- `is_number_unsigned()`
- `is_number_float()`
- `is_string()`
- `is_object()`
- `is_array()`
- `is_binary()`

## 3. Parser du JSON

### Depuis une chaine

```cpp
json j = json::parse(R"({
    "host": "localhost",
    "port": 8080
})");
```

### Depuis un flux

```cpp
#include <fstream>

std::ifstream in("config.json");
json j = json::parse(in);
```

### Depuis des iterateurs

```cpp
std::string s = R"([1,2,3])";
json j = json::parse(s.begin(), s.end());
```

### Validation sans exception

```cpp
if (json::accept(R"({"ok":true})"))
{
    // JSON valide
}
```

### Sans lever d'exception

```cpp
json j = json::parse(text, nullptr, false);
if (j.is_discarded())
{
    // parsing echoue
}
```

## 4. Parser avec commentaires et virgules terminales

### Commentaires

Le parseur peut ignorer les commentaires si `ignore_comments = true` :

```cpp
json j = json::parse(text, nullptr, true, true);
```

Signification des parametres :

- callback : `nullptr`
- allow_exceptions : `true`
- ignore_comments : `true`

### Virgules terminales

La documentation officielle montre aussi `ignore_trailing_commas = true` :

```cpp
json j = json::parse(text, nullptr, true, false, true);
```

Attention :

- la page API officielle precise que ce parametre a ete ajoute en `3.12.1` ;
- la release stable verifiee ici est `3.12.0` ;
- si tu es strictement en `3.12.0`, verifie d'abord sa disponibilite dans ton environnement.

## 5. Serialiser avec `dump()`

### Compact

```cpp
std::string s = j.dump();
```

### Pretty-print

```cpp
std::string s = j.dump(2);
```

### Sortie stream

```cpp
#include <iomanip>
#include <iostream>

std::cout << std::setw(2) << j << '\n';
```

### ASCII seulement

```cpp
std::string s = j.dump(-1, ' ', true);
```

### Gestion UTF-8 invalide a la serialisation

```cpp
std::string s = j.dump(-1, ' ', false, json::error_handler_t::replace);
```

Valeurs possibles pour `error_handler_t` :

- `strict`
- `replace`
- `ignore`

## 6. Lire des valeurs

### `operator[]` : pratique mais permissif

```cpp
std::string name = j["name"];
```

Bon pour :

- construire un document ;
- ecrire rapidement ;
- lire quand tu es certain de la structure.

Attention :

- sur objet non const, une cle absente cree une valeur `null` ;
- sur tableau non const, un index hors borne peut etendre le tableau avec des `null` ;
- sur valeur const, lire une cle absente via `operator[]` est un comportement indefini.

### `at()` : lecture securisee

```cpp
std::string name = j.at("name").get<std::string>();
```

Si la cle n'existe pas, `at()` lance une exception `out_of_range`.

### `value()` : lecture avec valeur par defaut

```cpp
int port = j.value("port", 80);
std::string host = j.value("host", std::string("127.0.0.1"));
```

Tres utile pour les fichiers de configuration.

### `contains()` : tester avant lecture

```cpp
if (j.contains("timeout"))
{
    int timeout = j["timeout"];
}
```

## 7. Convertir vers des types C++

### `get<T>()`

```cpp
int age = j.at("age").get<int>();
std::vector<int> v = j.at("numbers").get<std::vector<int>>();
```

### `get_to()`

```cpp
std::string name;
j.at("name").get_to(name);
```

`get_to()` est pratique pour remplir une variable existante.

### Conversions STL

La bibliotheque convertit naturellement beaucoup de types STL compatibles :

- `std::vector`
- `std::map`
- `std::unordered_map`
- `std::string`
- `std::array`
- `std::pair`
- `std::tuple`
- etc.

## 8. Modifier des valeurs

### Affectation

```cpp
j["name"] = "Bob";
j["enabled"] = true;
```

### Ajouter a un tableau

```cpp
j["tags"].push_back("cpp");
j["tags"] += "json";
```

### Inserer dans un objet

```cpp
j["config"]["theme"] = "light";
j["version"] = 1;
```

### Fusionner des objets

```cpp
json defaults = {
    {"host", "localhost"},
    {"port", 8080},
    {"features", {{"logs", true}}}
};

json user = {
    {"port", 9090},
    {"features", {{"cache", true}}}
};

defaults.update(user, true);
```

Avec `merge_objects = true`, les sous-objets communs sont fusionnes recursivement.

## 9. Parcourir un JSON

### Parcours direct

```cpp
for (const auto& value : j["items"])
{
    // ...
}
```

### Parcours avec `items()`

```cpp
for (auto& el : j.items())
{
    std::cout << el.key() << " => " << el.value() << '\n';
}
```

### Structured bindings en C++17

```cpp
for (auto& [key, value] : j.items())
{
    std::cout << key << " => " << value << '\n';
}
```

Attention :

- n'utilise pas `items()` sur un objet temporaire ;
- l'objet JSON doit vivre pendant toute l'iteration.

## 10. JSON Pointer

`JSON Pointer` permet d'adresser un chemin dans un document JSON.

```cpp
using namespace nlohmann::literals;

json j = {
    {"user", {
        {"name", "Alice"},
        {"roles", {"admin", "editor"}}
    }}
};

std::string name = j.at("/user/name"_json_pointer).get<std::string>();
std::string role0 = j["/user/roles/0"_json_pointer];
```

## 11. `flatten()` et `unflatten()`

```cpp
json original = {
    {"user", {{"name", "Alice"}}},
    {"enabled", true}
};

json flat = original.flatten();
json restored = flat.unflatten();
```

Utiles pour :

- comparer des structures ;
- manipuler des chemins ;
- produire des representations plates.

Limite officielle a connaitre :

- les objets et tableaux vides sont aplatis en `null` et ne sont pas reconstruits parfaitement par `unflatten()`.

## 12. Patchs

### JSON Patch

```cpp
json doc = {{"name", "Alice"}};
json patch = json::parse(R"([
    {"op":"replace","path":"/name","value":"Bob"},
    {"op":"add","path":"/active","value":true}
])");

json result = doc.patch(patch);
```

### Diff

```cpp
json before = {{"name", "Alice"}};
json after = {{"name", "Bob"}, {"active", true}};

json d = json::diff(before, after);
json rebuilt = before.patch(d);
```

Invariant important :

```cpp
before.patch(json::diff(before, after)) == after
```

### Merge Patch

```cpp
json doc = {
    {"title", "Old"},
    {"meta", {{"draft", true}, {"views", 10}}}
};

json patch = {
    {"title", "New"},
    {"meta", {{"draft", false}}}
};

doc.merge_patch(patch);
```

## 13. Ordre des cles

Par defaut, `nlohmann::json` utilise une representation orientee objet qui ne preserve pas l'ordre d'insertion.

Si tu veux conserver l'ordre :

```cpp
using ordered_json = nlohmann::ordered_json;

ordered_json j;
j["first"] = 1;
j["second"] = 2;
```

Important :

- si tu veux parser tout en preservant l'ordre, il faut appeler `ordered_json::parse(...)` et pas `json::parse(...)`.

## 14. Literaaux utiles

### `_json`

```cpp
using namespace nlohmann::literals;
json j = R"({"answer":42})"_json;
```

### `_json_pointer`

```cpp
using namespace nlohmann::literals;
auto p = "/settings/theme"_json_pointer;
```

## 15. Ce qu'il faut retenir

- pour construire : `operator[]`, `push_back()`, `+=`
- pour lire proprement : `at()`, `value()`, `contains()`
- pour convertir : `get<T>()`, `get_to()`
- pour serialiser : `dump()`
- pour les chemins : `json_pointer`
- pour conserver l'ordre : `ordered_json`
