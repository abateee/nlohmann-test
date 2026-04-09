# Reference pratique

Cette page sert de pense-bete. Elle ne remplace pas l'API complete, mais couvre l'essentiel de l'usage quotidien.

## 1. Types principaux

### Alias courants

```cpp
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
```

### Type generique

- `nlohmann::basic_json`
- `nlohmann::json` : specialisation par defaut
- `nlohmann::ordered_json` : variante qui preserve l'ordre d'insertion

## 2. Construction

### Valeurs

| But | Exemple |
| --- | --- |
| null | `json j = nullptr;` |
| booleen | `json j = true;` |
| entier | `json j = 42;` |
| flottant | `json j = 3.14;` |
| chaine | `json j = "hello";` |
| objet | `json j = {{"k", "v"}};` |
| tableau | `json j = {1, 2, 3};` |
| tableau explicite | `json j = json::array();` |
| objet explicite | `json j = json::object();` |
| binaire | `json j = json::binary({0x01, 0x02});` |

## 3. Inspection de type

| Fonction | Role |
| --- | --- |
| `type()` | retourne l'enum interne |
| `type_name()` | retourne un nom lisible du type |
| `is_null()` | teste `null` |
| `is_boolean()` | teste `bool` |
| `is_number()` | teste tout nombre |
| `is_number_integer()` | teste entier signe |
| `is_number_unsigned()` | teste entier non signe |
| `is_number_float()` | teste flottant |
| `is_string()` | teste chaine |
| `is_object()` | teste objet |
| `is_array()` | teste tableau |
| `is_binary()` | teste valeur binaire |
| `is_structured()` | teste objet ou tableau |
| `is_primitive()` | teste type primitif |
| `is_discarded()` | teste l'etat d'un parse en mode non exception |

## 4. Parsing

| Fonction | Usage |
| --- | --- |
| `json::parse(input)` | parse depuis une chaine, un flux, un buffer, etc. |
| `json::parse(first, last)` | parse depuis une plage d'iterateurs |
| `json::accept(input)` | valide sans construire d'objet JSON |

### Signatures mentales utiles

```cpp
json::parse(input, callback, allow_exceptions, ignore_comments, ignore_trailing_commas);
json::accept(input, ignore_comments, ignore_trailing_commas);
```

Notes :

- `allow_exceptions = false` retourne une valeur `discarded` en cas d'erreur ;
- `ignore_comments` est documente et etabli ;
- `ignore_trailing_commas` est documente comme ajoute en `3.12.1`.

## 5. Serialisation

| Fonction | Usage |
| --- | --- |
| `dump()` | JSON compact |
| `dump(2)` | JSON joli avec indentation |
| `std::cout << j` | sortie stream compacte |
| `std::cout << std::setw(2) << j` | sortie stream pretty |

### Signature utile

```cpp
j.dump(indent, indent_char, ensure_ascii, error_handler);
```

`error_handler` :

- `json::error_handler_t::strict`
- `json::error_handler_t::replace`
- `json::error_handler_t::ignore`

## 6. Acces aux elements

| Outil | Quand l'utiliser |
| --- | --- |
| `operator[]` | construction, ecriture rapide, lecture si structure certaine |
| `at()` | lecture stricte, cle/index obligatoire |
| `value()` | lecture avec valeur par defaut |
| `contains()` | test d'existence |
| `find()` | recherche style STL |
| `count()` | compter la presence d'une cle |
| `front()` / `back()` | premier/dernier element |

### Rappels importants

- `operator[]` sur objet non const cree une cle absente avec `null`
- `operator[]` sur tableau non const peut agrandir le tableau
- `operator[]` sur objet const avec cle absente : comportement indefini
- `value()` ne modifie jamais le JSON
- `contains()` sur une valeur non objet renvoie `false`

## 7. Iteration

| Fonction | Usage |
| --- | --- |
| `begin()` / `end()` | iteration STL |
| `cbegin()` / `cend()` | iteration const |
| `rbegin()` / `rend()` | iteration inverse |
| `items()` | obtenir `key()` et `value()` facilement |

Exemple :

```cpp
for (auto& [key, value] : j.items())
{
    // ...
}
```

## 8. Modification

| Fonction | Role |
| --- | --- |
| `operator=` | affecter une nouvelle valeur |
| `push_back()` | ajouter a un tableau |
| `emplace_back()` | construire a la fin d'un tableau |
| `operator+=` | ajouter a un tableau ou inserer dans un objet selon le contexte |
| `insert()` | inserer dans tableau ou objet |
| `emplace()` | inserer de maniere style STL |
| `erase()` | supprimer par cle, index ou iterateur |
| `clear()` | vider |
| `swap()` | echanger |
| `update()` | fusion d'objets type dictionnaire |
| `merge_patch()` | appliquer un merge patch RFC 7386 |

### `update()` en pratique

```cpp
target.update(source);       // ecrase les cles existantes
target.update(source, true); // fusion recursive des objets communs
```

## 9. Conversion vers des types C++

| Fonction | Role |
| --- | --- |
| `get<T>()` | convertit en `T` et retourne le resultat |
| `get_to(x)` | remplit une variable existante |
| `get_ptr()` | acces pointeur bas niveau si type exact |
| `get_ref()` | recuperer une reference du type stocke |

Exemples :

```cpp
int n = j.at("n").get<int>();

std::string s;
j.at("name").get_to(s);
```

## 10. JSON Pointer

| Element | Usage |
| --- | --- |
| `json::json_pointer` | type chemin RFC 6901 |
| `"_json_pointer"` | litteral de chemin |
| `at(ptr)` | acces securise par chemin |
| `operator[](ptr)` | acces permissif par chemin |
| `value(ptr, defaut)` | lecture avec defaut par chemin |
| `contains(ptr)` | test d'existence par chemin |

Exemple :

```cpp
using namespace nlohmann::literals;
auto enabled = j.value("/features/logs"_json_pointer, false);
```

## 11. Transformations de documents

| Fonction | Role |
| --- | --- |
| `patch(patch_doc)` | applique un JSON Patch RFC 6902 |
| `patch_inplace(patch_doc)` | applique directement sur l'objet |
| `json::diff(a, b)` | produit un patch transformant `a` en `b` |
| `merge_patch(patch_doc)` | applique un JSON Merge Patch RFC 7386 |
| `flatten()` | transforme en objet de pointeurs JSON -> valeurs primitives |
| `unflatten()` | reconstruit la structure |

Invariant officiel :

```cpp
source.patch(json::diff(source, target)) == target
```

## 12. Taille et capacite

| Fonction | Role |
| --- | --- |
| `size()` | nombre d'elements |
| `empty()` | teste si vide |
| `max_size()` | capacite maximale theorique |

## 13. Comparaisons

| Operateur | Role |
| --- | --- |
| `==`, `!=` | egalite / difference |
| `<`, `<=`, `>`, `>=` | comparaison ordonnee |
| `<=>` | comparaison 3 voies si disponible |

Utile pour :

- tests unitaires ;
- verification de configuration ;
- diff logique.

## 14. Formats binaires

| Fonction | Format |
| --- | --- |
| `to_bson()` / `from_bson()` | BSON |
| `to_cbor()` / `from_cbor()` | CBOR |
| `to_msgpack()` / `from_msgpack()` | MessagePack |
| `to_ubjson()` / `from_ubjson()` | UBJSON |
| `to_bjdata()` / `from_bjdata()` | BJData |

Raccourci mental :

- MessagePack et UBJSON : usage binaire courant
- CBOR : format binaire compact courant
- BSON : attention, la serialisation BSON attend un objet en racine

## 15. Exceptions a connaitre

| Type | Situation |
| --- | --- |
| `parse_error` | JSON invalide |
| `type_error` | operation incoherente avec le type |
| `out_of_range` | cle/index absent avec acces verifie |
| `invalid_iterator` | mauvais usage des iterateurs |
| `other_error` | autres cas de la bibliotheque |

## 16. Macros frequentes

| Macro | Role |
| --- | --- |
| `JSON_DIAGNOSTICS` | messages d'erreur plus riches |
| `JSON_NOEXCEPTION` | desactive les exceptions |
| `JSON_USE_IMPLICIT_CONVERSIONS` | controle certaines conversions implicites |
| `JSON_DISABLE_ENUM_SERIALIZATION` | force la serialisation custom des enums |
| `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` | genere `to_json` / `from_json` hors classe |
| `NLOHMANN_DEFINE_TYPE_INTRUSIVE` | genere `to_json` / `from_json` dans la classe |
| `NLOHMANN_JSON_SERIALIZE_ENUM` | mappe un enum vers des valeurs JSON |

## 17. Regles simples pour ne pas te tromper

- lecture stricte : `at()`
- lecture souple : `value()`
- ecriture rapide : `operator[]`
- existence : `contains()`
- conversion explicite : `get<T>()`
- fusion de config : `update(..., true)`
- chemins dynamiques : `json_pointer`
- ordre preserve : `ordered_json`
