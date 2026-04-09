# Installation et integration

## 1. Cas le plus simple : bibliotheque header-only

La bibliotheque est principalement utilisee comme une dependance header-only. Dans le cas le plus simple :

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;
```

Il faut simplement :

- que le repertoire d'include contenant `nlohmann/json.hpp` soit visible par le compilateur ;
- compiler avec au minimum C++11.

## 2. Integration manuelle

### Copier le header unique

Tu peux integrer directement le header unique publie par le projet :

- fichier principal : `json.hpp`
- chemin classique une fois installe : `nlohmann/json.hpp`

Ensuite :

```cpp
#include <nlohmann/json.hpp>
```

### Declarations anticipes

Le projet fournit aussi `json_fwd.hpp` pour les forward declarations.

Utilise-le uniquement si tu veux reduire le cout d'inclusion dans certains headers. Dans le doute, commence avec `json.hpp`.

## 3. CMake : `find_package`

Si la bibliotheque est deja installee dans ton systeme ou rendue disponible par un package manager, la voie standard est :

```cmake
cmake_minimum_required(VERSION 3.5)
project(ExampleProject LANGUAGES CXX)

find_package(nlohmann_json 3.12.0 REQUIRED)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json)
```

Points utiles :

- la target recommandee est `nlohmann_json::nlohmann_json` ;
- meme si la bibliotheque est header-only, cette target configure correctement les include paths et les features C++.

## 4. CMake : `FetchContent`

Approche pratique si tu veux que CMake telecharge lui-meme la dependance :

```cmake
cmake_minimum_required(VERSION 3.11)
project(ExampleProject LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json)
```

Quand choisir cette methode :

- tu veux un projet reproductible ;
- tu ne veux pas imposer une installation systeme ;
- tu veux verrouiller la version dans CMake.

## 5. CMake : `add_subdirectory`

Si tu embarques le depot source comme sous-module ou copie vendoree :

```cmake
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json)
```

Pratique si :

- tu as deja le code source de la bibliotheque dans le depot ;
- tu veux garder le controle local sans telechargement a la configuration.

## 6. Package managers courants

La documentation officielle liste entre autres :

- vcpkg : `nlohmann-json`
- Conan : `nlohmann_json`
- Meson : `nlohmann_json`
- Bazel : `nlohmann_json`
- Spack : `nlohmann-json`
- cget : `nlohmann/json`
- NuGet : `nlohmann.json`
- Conda : `nlohmann_json`
- CPM.cmake : `gh:nlohmann/json`
- xmake : `nlohmann_json`

## 7. Exemple vcpkg

Selon l'environnement, le nom de paquet est :

```text
nlohmann-json
```

Puis dans CMake :

```cmake
find_package(nlohmann_json 3.12.0 REQUIRED)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json)
```

## 8. Compilation minimale

### GCC / Clang

```bash
g++ -std=c++11 main.cpp -o app
```

ou avec include local :

```bash
g++ -std=c++11 main.cpp -I/path/to/includes -o app
```

### MSVC

```bat
cl /std:c++17 /EHsc main.cpp
```

La bibliotheque cible C++11, mais compiler en C++17 ou C++20 est souvent plus confortable.

## 9. Recommandation pratique

Choix simple selon le contexte :

- projet local tres simple : copier le header ou utiliser le package manager du systeme ;
- projet CMake moderne : `FetchContent` ;
- monorepo avec dependances vendorees : `add_subdirectory` ;
- environnement equipe/CI deja standardise : `find_package`.

## 10. A retenir

- include principal : `#include <nlohmann/json.hpp>`
- alias recommande : `using json = nlohmann::json;`
- version stable verifiee ici : `3.12.0`
- target CMake recommandee : `nlohmann_json::nlohmann_json`
