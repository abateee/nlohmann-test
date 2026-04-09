# Prise en main

## 1. `cv::Mat` : le conteneur central

`cv::Mat` represente une matrice dense. En pratique, c'est aussi le type image standard d'OpenCV.

Exemple :

```cpp
cv::Mat image = cv::imread("input.jpg", cv::IMREAD_COLOR);
```

Quelques proprietes utiles :

- `rows`
- `cols`
- `channels()`
- `type()`
- `empty()`

## 2. Lire une image

```cpp
cv::Mat image = cv::imread("input.jpg", cv::IMREAD_COLOR);
if (image.empty())
{
    // fichier absent, permissions, format non supporte, etc.
}
```

Point officiel important :

- si l'image ne peut pas etre lue, `imread` retourne une matrice vide.

Par defaut, les images couleur lues par OpenCV sont en ordre `BGR`.

## 3. Ecrire une image

```cpp
cv::imwrite("output.png", image);
```

Tu peux sauver :

- image source ;
- image convertie ;
- image annotee ;
- image intermediaire de debug.

## 4. Afficher une image

```cpp
cv::imshow("preview", image);
cv::waitKey(0);
```

Points pratiques :

- `imshow()` cree ou met a jour une fenetre ;
- `waitKey()` laisse la boucle GUI traiter les evenements ;
- sans `waitKey()`, l'affichage peut ne pas apparaitre ou se fermer immediatement.

## 5. Conversion de couleur

```cpp
cv::Mat gray;
cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
```

Conversions classiques :

- `cv::COLOR_BGR2GRAY`
- `cv::COLOR_GRAY2BGR`
- `cv::COLOR_BGR2RGB`
- `cv::COLOR_BGR2HSV`

## 6. Resize

```cpp
cv::Mat resized;
cv::resize(image, resized, cv::Size(640, 480));
```

Tu peux aussi utiliser un facteur d'echelle :

```cpp
cv::resize(image, resized, cv::Size(), 0.5, 0.5);
```

## 7. Flou, bords, seuillage

### Flou gaussien

```cpp
cv::Mat blurred;
cv::GaussianBlur(image, blurred, cv::Size(5, 5), 1.5);
```

### Canny

```cpp
cv::Mat gray;
cv::Mat edges;

cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
cv::Canny(gray, edges, 80, 160);
```

### Threshold

```cpp
cv::Mat binary;
cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY);
```

## 8. ROI : travailler sur une region

```cpp
cv::Rect roi(50, 40, 200, 100);
cv::Mat patch = image(roi);
```

Tres utile pour :

- crop ;
- post-traitement local ;
- detection sur sous-zones ;
- visualisation partielle.

Attention :

- un ROI sur `cv::Mat` partage souvent la meme memoire que l'image source ;
- si tu veux une copie independante, utilise `clone()`.

## 9. Copies : partage ou duplication

### Affectation simple

```cpp
cv::Mat a = image;
```

Cette affectation partage generalement les donnees.

### Copie profonde

```cpp
cv::Mat b = image.clone();
```

ou

```cpp
cv::Mat b;
image.copyTo(b);
```

## 10. Acces pixels

Pour des operations simples :

```cpp
cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
uchar b = pixel[0];
uchar g = pixel[1];
uchar r = pixel[2];
```

Mais en pratique :

- prefere les fonctions OpenCV deja optimisees si elles existent ;
- les boucles pixel manuelles sont plus fragiles et plus lentes si mal ecrites.

## 11. Dessiner

### Rectangle

```cpp
cv::rectangle(image, cv::Rect(50, 40, 200, 100), cv::Scalar(0, 255, 0), 2);
```

### Cercle

```cpp
cv::circle(image, cv::Point(100, 100), 25, cv::Scalar(255, 0, 0), 2);
```

### Ligne

```cpp
cv::line(image, cv::Point(0, 0), cv::Point(200, 200), cv::Scalar(0, 0, 255), 2);
```

### Texte

```cpp
cv::putText(
    image,
    "OpenCV",
    cv::Point(20, 40),
    cv::FONT_HERSHEY_SIMPLEX,
    1.0,
    cv::Scalar(255, 255, 255),
    2
);
```

## 12. Lire une video ou une webcam

### Ouvrir une webcam

```cpp
cv::VideoCapture cap(0);
if (!cap.isOpened())
{
    return 1;
}
```

### Lire les frames

```cpp
cv::Mat frame;
while (cap.read(frame))
{
    cv::imshow("camera", frame);
    if (cv::waitKey(1) == 27)
    {
        break;
    }
}
```

`27` correspond a la touche `Esc`.

## 13. Sauvegarder des structures OpenCV

OpenCV fournit `cv::FileStorage` pour lire/ecrire des structures en XML, YAML et JSON.

Exemple :

```cpp
cv::FileStorage fs("params.json", cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON);
fs << "threshold1" << 80;
fs << "threshold2" << 160;
fs.release();
```

Tres utile pour :

- parametres vision ;
- matrices de calibration ;
- structures OpenCV natives.

## 14. Ce qu'il faut retenir

- toujours tester `empty()` apres `imread()`
- OpenCV lit la couleur en `BGR`
- `cv::Mat` est le type pivot
- `clone()` fait une vraie copie
- `waitKey()` fait partie de l'usage normal de `imshow()`
- `VideoCapture` est l'entree standard pour webcam/video
