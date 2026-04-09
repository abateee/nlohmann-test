# Fonctions avancees

## 1. Types personnalises

Le mecanisme standard repose sur deux fonctions libres dans le namespace du type :

```cpp
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace app
{
struct Person
{
    std::string name;
    int age;
};

inline void to_json(json& j, const Person& p)
{
    j = json{{"name", p.name}, {"age", p.age}};
}

inline void from_json(const json& j, Person& p)
{
    j.at("name").get_to(p.name);
    j.at("age").get_to(p.age);
}
}
```

Ensuite :

```cpp
app::Person p{"Alice", 30};
json j = p;
app::Person copy = j.get<app::Person>();
```

Regles importantes :

- `to_json` et `from_json` doivent etre dans le namespace du type ;
- ces fonctions doivent etre visibles partout ou tu fais `json j = obj` ou `j.get<T>()`.

## 2. Macros pour reduire le boilerplate

### Hors classe

```cpp
namespace app
{
struct Person
{
    std::string name;
    int age;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, age)
}
```

### Dans la classe

```cpp
struct Person
{
    std::string name;
    int age;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Person, name, age)
};
```

### Variantes utiles

- `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT`
- `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE`
- `NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT`
- `NLOHMANN_DEFINE_TYPE_INTRUSIVE_ONLY_SERIALIZE`

Usage mental :

- `NON_INTRUSIVE` : tu ne veux pas modifier la classe ou ses membres sont publics ;
- `INTRUSIVE` : tu peux declarer la macro dans la classe et elle peut acceder aux membres prives ;
- `WITH_DEFAULT` : pratique quand des champs peuvent manquer a la deserialisation ;
- `ONLY_SERIALIZE` : utile si tu ne veux pas de `from_json`.

## 3. Enums

Par defaut, les enums sont serialises comme des entiers.

Pour une representation stable et lisible, utilise :

```cpp
namespace app
{
enum class Status
{
    unknown,
    pending,
    running,
    done
};

NLOHMANN_JSON_SERIALIZE_ENUM(Status, {
    {Status::unknown, "unknown"},
    {Status::pending, "pending"},
    {Status::running, "running"},
    {Status::done, "done"}
})
}
```

Bonnes pratiques :

- mets le couple par defaut en premier ;
- garde cette macro dans le namespace de l'enum ;
- prefere une representation string pour ne pas casser les fichiers si l'ordre interne de l'enum change.

Si tu veux interdire la serialisation implicite des enums en entier :

```cpp
#define JSON_DISABLE_ENUM_SERIALIZATION 1
```

## 4. JSON Pointer

La bibliotheque implemente RFC 6901.

Exemple :

```cpp
using namespace nlohmann::literals;

json j = {
    {"nested", {
        {"one", 1},
        {"three", {true, false}}
    }}
};

int one = j.at("/nested/one"_json_pointer).get<int>();
bool second = j.at("/nested/three/1"_json_pointer).get<bool>();
```

Utilite :

- chemins dynamiques ;
- acces a des structures profondes ;
- base des mecanismes `patch`, `diff`, `flatten`.

## 5. JSON Patch et `diff`

La bibliotheque implemente JSON Patch RFC 6902.

### Appliquer un patch

```cpp
json doc = {
    {"baz", "qux"},
    {"foo", "bar"}
};

json patch = json::parse(R"([
    {"op":"replace","path":"/baz","value":"boo"},
    {"op":"add","path":"/hello","value":["world"]},
    {"op":"remove","path":"/foo"}
])");

json out = doc.patch(patch);
```

### Construire un patch avec `diff`

```cpp
json source = {
    {"baz", "qux"},
    {"foo", "bar"}
};

json target = {
    {"baz", "boo"},
    {"hello", {"world"}}
};

json patch = json::diff(source, target);
json rebuilt = source.patch(patch);
```

Relation importante :

```cpp
source.patch(json::diff(source, target)) == target
```

## 6. JSON Merge Patch

La bibliotheque implemente JSON Merge Patch RFC 7386.

```cpp
json doc = {
    {"title", "Goodbye!"},
    {"author", {
        {"givenName", "John"},
        {"familyName", "Doe"}
    }},
    {"tags", {"example", "sample"}}
};

json patch = {
    {"title", "Hello!"},
    {"phoneNumber", "+01-123-456-7890"},
    {"author", {
        {"familyName", nullptr}
    }},
    {"tags", {"example"}}
};

doc.merge_patch(patch);
```

Quand le preferer a JSON Patch :

- si tu veux une syntaxe qui ressemble au document final ;
- si tu fais surtout des mises a jour de type configuration ou API HTTP PATCH simple.

## 7. `flatten()` et `unflatten()`

`flatten()` transforme un document imbrique en objet plat dont les cles sont des JSON Pointers.

```cpp
json j = {
    {"answer", {{"everything", 42}}},
    {"happy", true}
};

json flat = j.flatten();
json back = flat.unflatten();
```

Cas d'usage :

- comparaisons de structures ;
- export plat ;
- certaines transformations de schema.

Limite officielle :

- les tableaux ou objets vides deviennent `null` dans la representation plate et ne sont pas restaures a l'identique.

## 8. Ordre des objets : `ordered_json`

Par defaut, `json` ne preserve pas l'ordre d'insertion des cles.

Pour le conserver :

```cpp
using ordered_json = nlohmann::ordered_json;

ordered_json j;
j["one"] = 1;
j["two"] = 2;
j["three"] = 3;
```

Important a ne pas rater :

```cpp
std::ifstream in("input.json");
auto j = nlohmann::ordered_json::parse(in);
```

Si tu appelles `json::parse(...)` puis convertis ensuite, tu as deja perdu l'ordre.

## 9. Formats binaires

La bibliotheque sait encoder/decoder :

- BJData
- BSON
- CBOR
- MessagePack
- UBJSON

### Fonctions

```cpp
std::vector<std::uint8_t> cbor = json::to_cbor(j);
json from_cbor = json::from_cbor(cbor);
```

Meme principe pour :

- `to_bson` / `from_bson`
- `to_msgpack` / `from_msgpack`
- `to_ubjson` / `from_ubjson`
- `to_bjdata` / `from_bjdata`

### Points officiels utiles

- MessagePack : serialisation et deserialisation completes
- UBJSON : serialisation et deserialisation completes
- BSON : serialisation incomplete car la racine doit etre un objet
- CBOR : serialisation complete, deserialisation partielle pour certains cas non JSON natifs
- support des valeurs binaires selon le format

## 10. Parser callbacks

`parse()` accepte un callback de type `parser_callback_t` pour filtrer des parties du document pendant la lecture.

Exemple d'idee :

```cpp
json::parser_callback_t cb =
    [](int depth, json::parse_event_t event, json& parsed)
    {
        if (event == json::parse_event_t::key && parsed == json("debug"))
        {
            return false;
        }
        return true;
    };

json j = json::parse(text, cb);
```

Pratique pour :

- ignorer certaines cles ;
- reduire le document pendant le parsing ;
- controler un flux entrant.

## 11. Interface SAX

La bibliotheque expose aussi une interface SAX pour un parsing evenementiel plus bas niveau.

A utiliser si :

- tu dois traiter de gros flux sans construire tout l'objet en memoire ;
- tu as besoin d'un parseur evenementiel ;
- tu veux du controle fin.

Pour un usage applicatif classique, `parse()` reste le meilleur point d'entree.

## 12. Macros utiles de configuration

### `JSON_DIAGNOSTICS`

Active des messages d'erreur plus riches. Tres utile en debug.

```cpp
#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>
```

### `JSON_NOEXCEPTION`

Desactive le systeme d'exceptions de la bibliotheque.

```cpp
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>
```

Cette option est reservee a des projets qui gerent deja un modele strict sans exceptions. Pour un usage normal, garde les exceptions activees.

### `JSON_USE_IMPLICIT_CONVERSIONS`

Controle certaines conversions implicites. Si tu veux un code plus strict, il peut etre pertinent de les limiter.

## 13. Quand utiliser les fonctions avancees

- types metier : `to_json` / `from_json` ou macros `NLOHMANN_DEFINE_*`
- API evolutives : `NLOHMANN_JSON_SERIALIZE_ENUM`
- chemins dynamiques : `json_pointer`
- synchronisation de documents : `diff`, `patch`, `merge_patch`
- serialisation reseau/stockage compact : formats binaires
- volumetrie ou streaming : callback de parsing, SAX
- besoin d'ordre stable : `ordered_json`
