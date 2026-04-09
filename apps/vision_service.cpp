#include <iostream>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "visiondarts/app/http_service.hpp"
#include "visiondarts/app/service_controller.hpp"
#include "visiondarts/core/config.hpp"

int main(int argc, char** argv)
{
    try
    {
        visiondarts::AppConfig config;
        if (argc >= 2)
        {
            config = visiondarts::load_app_config(argv[1]);
        }

        visiondarts::ServiceController controller(config);
        httplib::Server server;
        visiondarts::configure_service_socket_options(server);
        visiondarts::register_service_routes(server, controller);

        std::cout << "Service offline sur http://" << config.backend.service_host << ':' << config.backend.service_port << '\n';
        if (!server.bind_to_port(config.backend.service_host, config.backend.service_port))
        {
            std::cerr << "Erreur: impossible de reserver " << config.backend.service_host << ':'
                      << config.backend.service_port << ".\n";
            return 1;
        }

        if (!server.listen_after_bind())
        {
            std::cerr << "Erreur: impossible de demarrer l'ecoute HTTP sur " << config.backend.service_host << ':'
                      << config.backend.service_port << ".\n";
            return 1;
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
