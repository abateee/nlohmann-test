// Includes pour le traitement d'images OpenCV, sortie console et chaînes.
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

// Affiche des informations utiles sur une image OpenCV.
static void print_info(const cv::Mat& image, const std::string& name)
{
    std::cout
        << name
        << ": "
        << image.cols
        << "x"
        << image.rows
        << ", channels="
        << image.channels()
        << ", type="
        << image.type()
        << '\n';
}

// Montre un petit pipeline d'image sur un fichier disque.
static int image_pipeline_example()
{
    // Charge une image couleur depuis le fichier
    cv::Mat image = cv::imread("input.jpg", cv::IMREAD_COLOR);

    if (image.empty())
    {
        std::cerr << "Impossible de lire input.jpg\n";
        return 1;
    }

    // Affiche les métadonnées de l'image source
    print_info(image, "source");

    // Convertit en niveaux de gris ( Homographie en gros )
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Applique un flou gaussien pour lisser le bruit avant filtre de Canny
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // Détecte les contours avec Canny
    cv::Mat edges;
    cv::Canny(blurred, edges, 80, 160);

    // Crée une image de prévisualisation plus petite
    cv::Mat preview;
    cv::resize(edges, preview, cv::Size(), 0.75, 0.75);

    // Sauvegarde l'image en niveaux de gris, contours et prévisualisation
    cv::imwrite("gray.png", gray);
    cv::imwrite("edges.png", edges);
    cv::imwrite("preview.png", preview);

    // Affiche les images dans des fenêtres
    cv::imshow("source", image);
    cv::imshow("edges", edges);

    cv::waitKey(0);

    return 0;
}

// Montre une boucle de capture caméra simple.
static int camera_example()
{
    // Ouvre la caméra par défaut du système
    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        std::cerr << "Impossible d'ouvrir la camera\n";
        return 1;
    }

    cv::Mat frame;

    // Lit et traite les images tant que la caméra fournit des frames
    while (cap.read(frame))
    {
        // Convertit la frame courante en niveaux de gris
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Affiche la frame en niveaux de gris
        cv::imshow("camera-gray", gray);

        // Quitte la boucle si l'utilisateur appuie sur Échap
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }

    return 0;
}

// Montre l'utilisation de FileStorage pour écrire des données OpenCV au format JSON.
static void filestorage_example()
{
    // Crée une matrice 3x3 ressemblant à des intrinsèques de caméra
    cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) <<
        500.0, 0.0, 320.0,
        0.0, 500.0, 240.0,
        0.0, 0.0, 1.0);

    // Ouvre un fichier JSON en écriture via l'API FileStorage d'OpenCV
    cv::FileStorage fs("camera_params.json", cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON);

    // Écrit la matrice de caméra sous la clé "camera_matrix"
    fs << "camera_matrix" << camera_matrix;

    // Écrit un vecteur de distorsion nul sous la clé "distortion"
    fs << "distortion" << cv::Mat::zeros(1, 5, CV_64F);

    // Ferme le fichier explicitement
    fs.release();
}

// Point d'entrée principal du programme de démonstration OpenCV.
int main()
{
    // Génère d'abord un petit fichier JSON avec des paramètres de caméra
    filestorage_example();

    // Exécute ensuite l'exemple de pipeline sur image fichier
    return image_pipeline_example();

    // Pour tester la caméra à la place :
    // return camera_example();
}
