# Bonnes pratiques et pieges

## 1. OpenCV lit la couleur en `BGR`, pas en `RGB`

C'est une source classique d'erreurs.

Si tu envoies une image vers :

- une lib web ;
- un moteur graphique ;
- un format attendu en `RGB` ;
- du code Python/JS qui suppose `RGB` ;

pense a convertir :

```cpp
cv::Mat rgb;
cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
```

## 2. `cv::Mat` partage souvent les donnees

Exemple :

```cpp
cv::Mat a = image;
```

Ici, `a` et `image` pointent souvent vers les memes donnees.

Si tu veux une copie independante :

```cpp
cv::Mat copy = image.clone();
```

ou

```cpp
image.copyTo(copy);
```

## 3. Toujours verifier `empty()` apres un chargement

Exemple :

```cpp
cv::Mat image = cv::imread("input.jpg");
if (image.empty())
{
    return 1;
}
```

Ne pars jamais du principe qu'un fichier a ete lu correctement.

## 4. `imshow()` sans `waitKey()` est presque toujours une erreur

`waitKey()` ne sert pas seulement a "attendre une touche".

Il permet aussi :

- de traiter les evenements GUI ;
- de rafraichir les fenetres ;
- de laisser l'utilisateur interagir.

## 5. Fais attention aux types de donnees

Un meme code peut donner des resultats differents selon :

- `CV_8U`
- `CV_32F`
- `CV_64F`

Par exemple :

- certains filtres ou calculs de gradient gagnent a etre faits en float ;
- un cast implicite mal place peut saturer ou tronquer ;
- une division entiere peut produire un resultat faux.

## 6. Pour les conversions de couleur non lineaires, pense a l'echelle

La doc OpenCV rappelle qu'avant certaines conversions non lineaires, une image float doit etre normalisee dans la bonne plage.

Exemple typique :

- une image `CV_32F` issue d'une conversion depuis `CV_8U` doit souvent etre remise dans `[0,1]` selon l'algorithme vise.

## 7. Evite les boucles pixel manuelles si OpenCV propose deja une fonction

Les fonctions natives sont souvent :

- plus courtes ;
- plus lisibles ;
- mieux optimisees ;
- moins sujettes aux erreurs d'index ou de type.

Avant d'ecrire une double boucle `for`, verifie si `imgproc` ne fait pas deja le travail.

## 8. Les ROI ne sont pas toujours des copies

Exemple :

```cpp
cv::Mat roi = image(cv::Rect(10, 10, 100, 80));
```

Modifier `roi` peut modifier `image`.

Si tu veux isoler :

```cpp
cv::Mat detached = image(cv::Rect(10, 10, 100, 80)).clone();
```

## 9. Pense a l'environnement d'execution

`highgui` n'est pas toujours adapte en :

- serveur ;
- CI ;
- conteneur headless ;
- service Windows sans session GUI.

Dans ces contextes :

- sauve des images ;
- loggue des statistiques ;
- evite de dependre de `imshow()`.

## 10. Separer I/O et traitement simplifie enormement le debug

Pattern conseille :

1. une fonction charge les donnees ;
2. une fonction traite ;
3. une fonction produit les sorties.

Tu gagnes :

- tests plus simples ;
- traitement reutilisable ;
- meilleure tracabilite des erreurs.

## 11. `FileStorage` est utile, mais pas pour tout

Tres bon choix pour :

- matrices OpenCV ;
- calibration ;
- structures vision internes.

Moins bon choix pour :

- API JSON metier ;
- echanges inter-services generalistes ;
- documents applicatifs riches.

Dans ces cas, une vraie bibliotheque JSON generaliste comme `nlohmann::json` est souvent meilleure.

## 12. Checklist rapide

- verifier `empty()`
- ne pas oublier `waitKey()`
- connaitre `BGR`
- utiliser `clone()` quand il faut isoler
- choisir le bon type de matrice
- preferer les fonctions OpenCV aux boucles pixel artisanales
