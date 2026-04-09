# Documentation locale de `OpenCV`

Ce dossier contient une documentation en francais, orientee pratique, pour `OpenCV` en C++.

## Ce que c'est

OpenCV est une bibliotheque majeure de vision par ordinateur et de traitement d'image.

Elle couvre notamment :

- lecture et ecriture d'images ;
- manipulation de matrices et tableaux multi-canaux ;
- filtrage, seuillage, contours, transformations geometriques ;
- video, webcam, capture ;
- calibration, features, dnn, etc.

## Version de reference utilisee ici

Cette documentation prend comme base la release stable officiellement marquee `Latest` sur GitHub au moment de la redaction :

- `opencv/opencv` `4.13.0`
- date de release : `2025-12-31`

Note importante :

- le site `docs.opencv.org/4.x/` peut afficher une documentation generee depuis une version de developpement plus recente ;
- ce dossier se cale sur la release stable ci-dessus.

## Plan du dossier

- [`01-installation-et-integration.md`](01-installation-et-integration.md) : headers, CMake, structure projet.
- [`02-prise-en-main.md`](02-prise-en-main.md) : `cv::Mat`, lecture image, affichage, video, traitements de base.
- [`03-reference-pratique.md`](03-reference-pratique.md) : pense-bete des classes, fonctions et modules a connaitre.
- [`04-bonnes-pratiques-et-pieges.md`](04-bonnes-pratiques-et-pieges.md) : BGR/RGB, copies, types, GUI, erreurs frequentes.
- [`exemples.cpp`](exemples.cpp) : exemples C++ simples et directement reutilisables.
- [`sources-officielles.md`](sources-officielles.md) : sources officielles consultees.

## Demarrage rapide

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::Mat image = cv::imread("input.jpg", cv::IMREAD_COLOR);
    if (image.empty())
    {
        std::cerr << "Impossible de lire input.jpg\n";
        return 1;
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::imwrite("gray.png", gray);

    cv::imshow("gray", gray);
    cv::waitKey(0);
    return 0;
}
```

## Quand utiliser quoi

- conteneur image/matrice : `cv::Mat`
- lecture image : `cv::imread`
- ecriture image : `cv::imwrite`
- conversion de couleur : `cv::cvtColor`
- resize : `cv::resize`
- flou : `cv::GaussianBlur`
- bords : `cv::Canny`
- affichage fenetre : `cv::imshow`
- attente clavier / boucle GUI : `cv::waitKey`
- webcam / video : `cv::VideoCapture`
- serialisation native OpenCV : `cv::FileStorage`

## Resume mental

OpenCV, en C++, c'est surtout :

- `cv::Mat` pour les donnees ;
- des fonctions libres pour transformer ces donnees ;
- des modules specialises selon le besoin : `core`, `imgproc`, `imgcodecs`, `videoio`, `highgui`, etc.

La plupart des pipelines ressemblent a :

1. charger des pixels ;
2. transformer avec `imgproc` ;
3. afficher ou sauvegarder ;
4. produire des resultats metier.
