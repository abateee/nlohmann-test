#include <iostream>

#include <httplib.h>
#include <nlohmann/json.hpp>

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

        server.Post("/commands/start", [&](const httplib::Request&, httplib::Response& response) {
            response.set_content(controller.start().dump(2), "application/json");
        });

        server.Post("/commands/stop", [&](const httplib::Request&, httplib::Response& response) {
            response.set_content(controller.stop().dump(2), "application/json");
        });

        server.Post("/commands/reset-reference", [&](const httplib::Request&, httplib::Response& response) {
            response.set_content(controller.reset_reference().dump(2), "application/json");
        });

        server.Post("/commands/calibrate", [&](const httplib::Request& request, httplib::Response& response) {
            const auto payload = nlohmann::json::parse(request.body);
            response.set_content(controller.apply_calibration_payload(payload).dump(2), "application/json");
        });

        server.Get("/healthcheck", [&](const httplib::Request&, httplib::Response& response) {
            response.set_content(controller.healthcheck().dump(2), "application/json");
        });

        std::cout << "Service offline sur http://" << config.backend.service_host << ':' << config.backend.service_port << '\n';
        server.listen(config.backend.service_host, config.backend.service_port);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
