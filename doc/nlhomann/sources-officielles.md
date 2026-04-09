# Sources officielles

Documentation redigee a partir des sources officielles consultees le `2026-03-27`.

## Release stable verifiee

- GitHub Releases : <https://github.com/nlohmann/json/releases>
  - release marquee `Latest` au moment de la consultation : `v3.12.0`
  - date de release : `2025-04-11`

## Documentation officielle

- Accueil : <https://json.nlohmann.me/>
- CMake : <https://json.nlohmann.me/integration/cmake/>
- Package managers : <https://json.nlohmann.me/integration/package_managers/>
- Conversions de types personnalises : <https://json.nlohmann.me/features/arbitrary_types/>
- JSON Pointer : <https://json.nlohmann.me/features/json_pointer/>
- JSON Patch et Diff : <https://json.nlohmann.me/features/json_patch/>
- JSON Merge Patch : <https://json.nlohmann.me/features/merge_patch/>
- Formats binaires : <https://json.nlohmann.me/features/binary_formats/>
- Ordre des objets : <https://json.nlohmann.me/features/object_order/>
- Macros : <https://json.nlohmann.me/features/macros/>
- `parse()` : <https://json.nlohmann.me/api/basic_json/parse/>
- `dump()` : <https://json.nlohmann.me/api/basic_json/dump/>
- `value()` : <https://json.nlohmann.me/api/basic_json/value/>
- `contains()` : <https://json.nlohmann.me/api/basic_json/contains/>
- `items()` : <https://json.nlohmann.me/api/basic_json/items/>
- `update()` : <https://json.nlohmann.me/api/basic_json/update/>
- `accept()` : <https://json.nlohmann.me/api/basic_json/accept/>
- `flatten()` : <https://json.nlohmann.me/api/basic_json/flatten/>
- `unflatten()` : <https://json.nlohmann.me/api/basic_json/unflatten/>
- `operator[]` : <https://json.nlohmann.me/features/element_access/unchecked_access/>
- `at()` : <https://json.nlohmann.me/features/element_access/checked_access/>
- `_json` : <https://json.nlohmann.me/api/operator_literal_json/>
- `NLOHMANN_JSON_SERIALIZE_ENUM` : <https://json.nlohmann.me/api/macros/nlohmann_json_serialize_enum/>

## Note sur les versions

Certaines pages du site officiel mentionnent des ajouts notes comme introduits en `3.12.1`, notamment :

- `ignore_trailing_commas` dans `parse()`
- surcharge `char8_t` pour le litteral `_json`

Comme la release stable verifiee ici est `3.12.0`, cette documentation signale ces points quand ils apparaissent.
