// Includes de la bibliothèque standard pour E/S fichiers, formatage, sortie, chaînes et vecteurs.
// Include principal de la bibliothèque nlohmann::json.
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json>

// Alias de types pour les types JSON et littéraux.
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
using namespace nlohmann::literals;

// Place les types de démonstration dans un espace de noms dédié pour éviter de polluer l'espace global.
namespace demo
{
// Type métier simple pour les exemples de conversion JSON.
struct Person
{
    std::string name;
    int age;
    std::vector<std::string> roles;
};

// Conversion automatique de Person vers JSON.
inline void to_json(json& j, const Person& p)
{
    j = json{
        {"name", p.name},
        {"age", p.age},
        {"roles", p.roles}
    };
}

// Conversion automatique de JSON vers Person.
inline void from_json(const json& j, Person& p)
{
    j.at("name").get_to(p.name);
    j.at("age").get_to(p.age);
    j.at("roles").get_to(p.roles);
}

// Énumération pour l'exemple de sérialisation.
enum class Status
{
    unknown,
    pending,
    running,
    done
};

// Mapping de sérialisation JSON pour l'énumération Status.
NLOHMANN_JSON_SERIALIZE_ENUM(Status, {
    {Status::unknown, "unknown"},
    {Status::pending, "pending"},
    {Status::running, "running"},
    {Status::done, "done"}
})
}

// Affiche un titre de section pour les démonstrations.
static void print_title(const std::string& title)
{
    std::cout << "\n=== " << title << " ===\n";
}

// Démontre la création basique JSON, lecture et valeurs par défaut.
static void demo_basics()
{
    print_title("Basics");

    // Crée un objet JSON avec divers types
    json j = {
        {"name", "Alice"},
        {"age", 30},
        {"admin", false},
        {"tags", {"cpp", "json"}}
    };

    std::cout << std::setw(2) << j << '\n';

    // Accède à la clé requise
    std::cout << "name = " << j.at("name").get<std::string>() << '\n';

    // Accède à la clé optionnelle avec valeur par défaut
    std::cout << "port with default = " << j.value("port", 8080) << '\n';
}

// Démontre le parsing depuis un texte brut et différents formats d'affichage.
static void demo_parse_and_dump()
{
    print_title("Parse and dump");

    // Parse la chaîne JSON en objet manipulable
    std::string text = R"({
        "host": "localhost",
        "port": 8080,
        "enabled": true
    })";

    json j = json::parse(text);

    // Sortie compacte sur une seule ligne
    std::cout << j.dump() << '\n';

    // Sortie lisible avec indentation
    std::cout << j.dump(2) << '\n';

    // Vérifie si le texte est du JSON valide sans exceptions
    if (json::accept(text))
    {
        std::cout << "input is valid JSON\n";
    }
}

// Démontre la fusion de valeurs utilisateur dans une configuration par défaut.
static void demo_access_and_update()
{
    print_title("Access and update");

    // Définit la configuration de base
    json defaults = {
        {"host", "localhost"},
        {"port", 8080},
        {"features", {{"logs", true}, {"cache", false}}}
    };

    // Définit les surcharges utilisateur
    json user = {
        {"port", 9090},
        {"features", {{"cache", true}}}
    };

    // Fusionne utilisateur dans les défauts avec fusion récursive des sous-objets
    defaults.update(user, true);

    std::cout << std::setw(2) << defaults << '\n';
}

// Démontre la conversion automatique d'un type C++ personnalisé.
static void demo_custom_type()
{
    print_title("Custom type");

    // Crée un objet métier
    demo::Person person{"Alice", 30, {"admin", "editor"}};

    // Convertit en JSON automatiquement
    json j = person;

    std::cout << std::setw(2) << j << '\n';

    // Reconstruit depuis JSON
    demo::Person copy = j.get<demo::Person>();

    std::cout << copy.name << " / " << copy.age << '\n';
}

// Démontre la conversion JSON d'une énumération en utilisant la macro de bibliothèque.
static void demo_enum()
{
    print_title("Enum");

    // Convertit l'énumération en JSON
    json j = demo::Status::running;

    std::cout << j << '\n';

    // Reconvertit en énumération
    auto status = j.get<demo::Status>();

    std::cout << static_cast<int>(status) << '\n';
}

// Démontre les pointeurs JSON, aplatissement/désaplatissement et patch JSON.
static void demo_pointer_flatten_patch()
{
    print_title("Pointer / flatten / patch");

    // Crée un document JSON imbriqué
    json doc = {
        {"user", {
            {"name", "Alice"},
            {"roles", {"admin", "editor"}}
        }},
        {"enabled", true}
    };

    // Accède à la valeur imbriquée via pointeur JSON
    std::cout << doc.at("/user/name"_json_pointer) << '\n';

    // Aplati en objet avec clés de pointeurs JSON
    json flat = doc.flatten();

    std::cout << std::setw(2) << flat << '\n';

    // Restaure la structure imbriquée depuis l'aplati
    json restored = flat.unflatten();

    std::cout << std::setw(2) << restored << '\n';

    // Applique le patch JSON RFC 6902
    json patch = json::parse(R"([
        {"op":"replace","path":"/user/name","value":"Bob"},
        {"op":"add","path":"/user/active","value":true}
    ])");

    json patched = doc.patch(patch);

    std::cout << std::setw(2) << patched << '\n';
}

// Démontre le calcul de différence JSON et sa relecture.
static void demo_diff()
{
    print_title("Diff");

    // Document source
    json source = {
        {"name", "Alice"},
        {"roles", {"admin"}}
    };

    // Document cible
    json target = {
        {"name", "Bob"},
        {"roles", {"admin", "editor"}},
        {"active", true}
    };

    // Calcule le patch minimal pour aller de source à cible
    json d = json::diff(source, target);

    // Rejoue le patch sur la source pour reconstruire la cible
    json rebuilt = source.patch(d);

    std::cout << std::setw(2) << d << '\n';

    std::cout << std::boolalpha << (rebuilt == target) << '\n';
}

// Démontre ordered_json pour préserver l'ordre d'insertion des clés.
static void demo_ordered_json()
{
    print_title("ordered_json");

    // Crée un objet JSON ordonné
    ordered_json j;

    j["one"] = 1;
    j["two"] = 2;
    j["three"] = 3;

    std::cout << std::setw(2) << j << '\n';
}

// Point d'entrée principal du programme de démonstration.
int main()
{
    demo_basics();
    demo_parse_and_dump();
    demo_access_and_update();
    demo_custom_type();
    demo_enum();
    demo_pointer_flatten_patch();
    demo_diff();
    demo_ordered_json();

    return 0;
}
