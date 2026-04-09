# Reference pratique

Cette page sert de pense-bete. Elle vise l'usage quotidien, pas l'exhaustivite de toute l'API.

## 1. Types et classes a connaitre

| Type | Role |
| --- | --- |
| `cv::Mat` | matrice dense, image standard OpenCV |
| `cv::UMat` | variante orientee acceleration / memoire geree |
| `cv::Point`, `cv::Point2f` | points 2D |
| `cv::Size` | largeur / hauteur |
| `cv::Rect` | rectangle ROI |
| `cv::Scalar` | 4 composantes, souvent couleur |
| `cv::Vec3b` | pixel 3 canaux 8 bits |
| `cv::VideoCapture` | lecture webcam ou video |
| `cv::VideoWriter` | ecriture video |
| `cv::FileStorage` | serialisation OpenCV XML/YAML/JSON |

## 2. Modules utiles

| Module | Usage |
| --- | --- |
| `core` | matrices, types, algorithmes generiques |
| `imgcodecs` | lecture/ecriture image |
| `imgproc` | traitement image |
| `highgui` | fenetres, interactions de base |
| `videoio` | capture camera, video, ecriture |
| `features2d` | detecteurs / descripteurs |
| `calib3d` | calibration, geometrie multi-vues |
| `dnn` | inference reseaux neurones |

## 3. Lecture / ecriture image

| Fonction | Role |
| --- | --- |
| `cv::imread(path, flags)` | charge une image depuis un fichier |
| `cv::imwrite(path, image)` | sauvegarde une image |
| `cv::imdecode(buf, flags)` | decode depuis un buffer memoire |
| `cv::imencode(ext, image, buf)` | encode en buffer memoire |

Rappels :

- `imread()` retourne une matrice vide si la lecture echoue ;
- les images couleur decodees sont en `BGR`.

## 4. Affichage et GUI

| Fonction | Role |
| --- | --- |
| `cv::namedWindow()` | cree une fenetre |
| `cv::imshow()` | affiche une image |
| `cv::waitKey()` | traite les evenements GUI et lit une touche |
| `cv::destroyAllWindows()` | ferme toutes les fenetres |

## 5. Traitement d'image courant

| Fonction | Role |
| --- | --- |
| `cv::cvtColor()` | conversion de couleur |
| `cv::resize()` | redimensionnement |
| `cv::GaussianBlur()` | flou gaussien |
| `cv::medianBlur()` | flou median |
| `cv::blur()` | flou simple |
| `cv::threshold()` | seuillage |
| `cv::adaptiveThreshold()` | seuillage adaptatif |
| `cv::Canny()` | detection de bords |
| `cv::Sobel()` | derivees / gradients |
| `cv::Laplacian()` | derivee seconde |
| `cv::equalizeHist()` | egalisation histogramme |
| `cv::normalize()` | normalisation |

## 6. Geometrie et manipulation

| Fonction | Role |
| --- | --- |
| `cv::warpAffine()` | transformation affine |
| `cv::warpPerspective()` | homographie / perspective |
| `cv::rotate()` | rotation simple |
| `cv::flip()` | retournement |
| `cv::getRotationMatrix2D()` | matrice de rotation 2D |

## 7. Formes, contours, masques

| Fonction | Role |
| --- | --- |
| `cv::findContours()` | extraire des contours |
| `cv::drawContours()` | dessiner des contours |
| `cv::boundingRect()` | rectangle englobant |
| `cv::contourArea()` | aire d'un contour |
| `cv::arcLength()` | perimetre d'un contour |
| `cv::bitwise_and()` | masque logique ET |
| `cv::inRange()` | produire un masque de seuil |

## 8. Dessin

| Fonction | Role |
| --- | --- |
| `cv::line()` | ligne |
| `cv::rectangle()` | rectangle |
| `cv::circle()` | cercle |
| `cv::ellipse()` | ellipse |
| `cv::polylines()` | polylignes |
| `cv::putText()` | texte |

## 9. Video

| Fonction / classe | Role |
| --- | --- |
| `cv::VideoCapture` | lecture flux camera/video |
| `open()` | ouvrir source |
| `isOpened()` | verifier l'ouverture |
| `read(frame)` | lire une frame |
| `grab()` / `retrieve()` | controle plus fin de lecture |
| `cv::VideoWriter` | ecrire une video |

## 10. `cv::Mat` : operations mentales utiles

| Operation | Sens |
| --- | --- |
| `mat.empty()` | matrice vide ? |
| `mat.clone()` | copie profonde |
| `mat.copyTo(dst)` | copie profonde vers `dst` |
| `mat.row(i)` | vue ligne |
| `mat.col(j)` | vue colonne |
| `mat(rect)` | ROI |
| `mat.at<T>(y, x)` | acces typique a un element |
| `mat.channels()` | nombre de canaux |
| `mat.depth()` | profondeur scalaire |
| `mat.type()` | type OpenCV complet |

## 11. Types de donnees OpenCV frequents

| Type | Sens |
| --- | --- |
| `CV_8U` | entier non signe 8 bits |
| `CV_8UC1` | image 1 canal 8 bits |
| `CV_8UC3` | image couleur 3 canaux 8 bits |
| `CV_32F` | flottant 32 bits |
| `CV_32FC1` | matrice flottante 1 canal |
| `CV_64F` | flottant 64 bits |

## 12. Serialisation

| Outil | Usage |
| --- | --- |
| `cv::FileStorage` | lire/ecrire des structures OpenCV |
| `cv::FileNode` | noeud lu depuis `FileStorage` |
| `releaseAndGetString()` | recuperer la sortie texte si ecriture en memoire |

Formats supportes par `FileStorage` :

- XML
- YAML
- JSON

## 13. Regles simples pour ne pas te tromper

- image manquante : teste `empty()`
- affichage fenetre : `imshow()` puis `waitKey()`
- couleur pour affichage externe : pense `BGR` vs `RGB`
- copie independante : `clone()`
- stockage natif OpenCV : `FileStorage`
- pipeline standard : `imread -> imgproc -> imwrite/imshow`
