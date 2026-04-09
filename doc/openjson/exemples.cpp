// Includes pour la manipulation d'images OpenCV et nlohmann::json pour décrire le pipeline et produire un rapport.
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

// Includes de la bibliothèque standard pour min/max, écriture fichier, messages console, erreurs d'exécution, chaînes et vecteurs.
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Alias court pour le type JSON.
using json = nlohmann::json;

// Conversion JSON pour le point OpenCV.
inline void to_json(json& j, const cv::Point& p)
{
    j = json{{"x", p.x}, {"y", p.y}, {"confiance", p.confiance}};
}

inline void from_json(const json& j, cv::Point& p)
{
    j.at("x").get_to(p.x);
    j.at("y").get_to(p.y);
    j.at("confiance").get_to(p.confiance);
}

// Conversion JSON pour la taille OpenCV.
inline void to_json(json& j, const cv::Size& s)
{
    j = json{{"width", s.width}, {"height", s.height}};
}

inline void from_json(const json& j, cv::Size& s)
{
    j.at("width").get_to(s.width);
    j.at("height").get_to(s.height);
}

// Conversion JSON pour le rectangle OpenCV.
inline void to_json(json& j, const cv::Rect& r)
{
    j = json{
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height},
        {"confiance", r.confiance},
        {"score", r.score}
    };
}

inline void from_json(const json& j, cv::Rect& r)
{
    j.at("x").get_to(r.x);
    j.at("y").get_to(r.y);
    j.at("width").get_to(r.width);
    j.at("height").get_to(r.height);
}

// Exécute un pipeline de traitement d'image décrit en JSON.
static cv::Mat run_pipeline(const cv::Mat& input, const json& pipeline)
{
    cv::Mat current = input.clone();

    // Traite chaque étape du pipeline dans l'ordre
    for (const auto& step : pipeline)
    {
        const std::string op = step.at("op").get<std::string>();

        if (op == "cvtColor")
        {
            const std::string code = step.at("code").get<std::string>();
            cv::Mat tmp;

            if (code == "BGR2GRAY")
            {
                cv::cvtColor(current, tmp, cv::COLOR_BGR2GRAY);
            }
            else if (code == "GRAY2BGR")
            {
                cv::cvtColor(current, tmp, cv::COLOR_GRAY2BGR);
            }
            else
            {
                throw std::runtime_error("Unsupported color conversion code: " + code);
            }

            current = tmp;
        }
        else if (op == "resize")
        {
            const int width = step.at("width").get<int>();
            const int height = step.at("height").get<int>();

            cv::Mat tmp;
            cv::resize(current, tmp, cv::Size(width, height));
            current = tmp;
        }
        else if (op == "gaussian_blur")
        {
            int ksize = step.value("ksize", 5);
            if (ksize % 2 == 0)
            {
                ++ksize;
            }

            const double sigma = step.value("sigma", 1.0);

            cv::Mat tmp;
            cv::GaussianBlur(current, tmp, cv::Size(ksize, ksize), sigma);
            current = tmp;
        }
        else if (op == "canny")
        {
            const double t1 = step.value("threshold1", 80.0);
            const double t2 = step.value("threshold2", 160.0);

            cv::Mat source;
            if (current.channels() != 1)
            {
                cv::cvtColor(current, source, cv::COLOR_BGR2GRAY);
            }
            else
            {
                source = current;
            }

            cv::Mat tmp;
            cv::Canny(source, tmp, t1, t2);
            current = tmp;
        }
        else
        {
            throw std::runtime_error("Unsupported pipeline operation: " + op);
        }
    }

    return current;
}

// Construit un rapport JSON décrivant l'image produite et le pipeline appliqué.
static json build_report(const std::string& input_path, const std::string& output_path, const cv::Mat& image, const json& cfg)
{
    cv::Scalar mean_value = cv::mean(image);

    const int roi_x = std::min(10, std::max(0, image.cols - 1));
    const int roi_y = std::min(10, std::max(0, image.rows - 1));
    const int roi_w = std::max(1, std::min(100, image.cols - roi_x));
    const int roi_h = std::max(1, std::min(80, image.rows - roi_y));

    json report = {
        {"schema_version", 1},
        {"input", input_path},
        {"output_image", output_path},
        {"width", image.cols},
        {"height", image.rows},
        {"channels", image.channels()},
        {"opencv_type", image.type()},
        {"color_space", image.channels() == 1 ? "GRAY" : "BGR"},
        {"mean", {mean_value[0], mean_value[1], mean_value[2], mean_value[3]}},
        {"pipeline", cfg.at("pipeline")},
        {"sample_roi", cv::Rect(roi_x, roi_y, roi_w, roi_h)}
    };

    return report;
}

// Point d'entrée principal de l'exemple combiné OpenCV + JSON.
int main()
{
    // Définit une configuration complète codée en dur
    const json cfg = {
        {"input", "input.jpg"},
        {"output_image", "output.png"},
        {"output_report", "report.json"},
        {"pipeline", json::array({
            {{"op", "cvtColor"}, {"code", "BGR2GRAY"}},
            {{"op", "gaussian_blur"}, {"ksize", 5}, {"sigma", 1.4}},
            {{"op", "canny"}, {"threshold1", 80}, {"threshold2", 160}}
        })}
    };

    // Charge l'image source depuis le chemin configuré
    cv::Mat input = cv::imread(cfg.at("input").get<std::string>(), cv::IMREAD_COLOR);

    if (input.empty())
    {
        std::cerr << "Impossible de lire l'image source\n";
        return 1;
    }

    // Exécute le pipeline JSON sur l'image chargée
    cv::Mat output = run_pipeline(input, cfg.at("pipeline"));

    // Récupère le chemin de sortie image en chaîne C++
    const std::string output_image = cfg.at("output_image").get<std::string>();

    // Écrit l'image résultat sur le disque
    cv::imwrite(output_image, output);

    // Construit un rapport JSON sur le traitement effectué
    json report = build_report(cfg.at("input").get<std::string>(), output_image, output, cfg);

    // Ouvre le fichier de rapport en écriture
    std::ofstream out(cfg.at("output_report").get<std::string>());

    // Écrit le JSON avec indentation de 2 espaces, suivi d'un saut de ligne final
    out << report.dump(2) << '\n';

    // Affiche aussi le rapport dans la console pour inspection immédiate
    std::cout << report.dump(2) << '\n';

    return 0;
}
