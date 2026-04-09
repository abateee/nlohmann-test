# Installation et integration

## 1. Include minimal

Pour aller vite :

```cpp
#include <opencv2/opencv.hpp>
```

Dans un vrai projet, tu peux aussi inclure seulement les modules utiles :

```cpp
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
```

Avantage :

- moins de dependances explicites cachees ;
- temps de compilation souvent plus propres dans les gros projets.

## 2. CMake recommande

La forme classique est :

```cmake
cmake_minimum_required(VERSION 3.16)
project(OpenCvExample LANGUAGES CXX)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc highgui videoio)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE ${OpenCV_LIBS})
target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})
```

En pratique :

- `find_package(OpenCV REQUIRED)` suffit souvent ;
- ajouter les `COMPONENTS` clarifie les besoins du projet ;
- `${OpenCV_LIBS}` reste la forme courante de liaison.

## 3. Structure de projet simple

Exemple minimal :

```text
project/
  CMakeLists.txt
  src/
    main.cpp
  images/
    input.jpg
```

## 4. Exemple `main.cpp`

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::Mat image = cv::imread("images/input.jpg", cv::IMREAD_COLOR);
    if (image.empty())
    {
        std::cerr << "Lecture image impossible\n";
        return 1;
    }

    cv::imshow("image", image);
    cv::waitKey(0);
    return 0;
}
```

## 5. Build source ou paquets precompiles

Tu peux integrer OpenCV de plusieurs manieres :

- paquets systeme ;
- package manager C++ ;
- binaires precompiles ;
- build source.

Le choix depend de :

- ton OS ;
- ton compilateur ;
- les modules optionnels necessaires ;
- le besoin ou non de `opencv_contrib`.

## 6. `opencv` et `opencv_contrib`

Points importants :

- `opencv` contient le coeur du projet ;
- `opencv_contrib` ajoute des modules supplementaires ;
- si tu compiles depuis les sources avec `opencv_contrib`, les versions doivent correspondre.

## 7. Ce qu'il faut verifier quand ca ne linke pas

- version du compilateur compatible avec les binaires installes ;
- architecture coherente : `x64` avec `x64`, pas `x86` ;
- mode Debug/Release coherent avec les libs choisies ;
- modules demandes reels dans `find_package` ;
- chemins d'execution corrects si tu depens de DLL.

## 8. A retenir

- include simple : `#include <opencv2/opencv.hpp>`
- type central : `cv::Mat`
- CMake : `find_package(OpenCV REQUIRED ...)`
- lier avec `${OpenCV_LIBS}`
- version stable de reference ici : `4.13.0`
