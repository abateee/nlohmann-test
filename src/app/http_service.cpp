#include "visiondarts/app/http_service.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "visiondarts/app/service_controller.hpp"

namespace visiondarts
{
void configure_service_socket_options(httplib::Server& server)
{
    server.set_socket_options([](auto sock) {
#ifdef _WIN32
        const BOOL reuse_address = FALSE;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse_address), sizeof(reuse_address));

#ifdef SO_EXCLUSIVEADDRUSE
        const BOOL exclusive_address = TRUE;
        ::setsockopt(
            sock,
            SOL_SOCKET,
            SO_EXCLUSIVEADDRUSE,
            reinterpret_cast<const char*>(&exclusive_address),
            sizeof(exclusive_address));
#endif
#else
        const int reuse_address = 0;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));
#endif
    });
}

void register_service_routes(httplib::Server& server, ServiceController& controller)
{
    server.Post("/commands/start", [&](const httplib::Request&, httplib::Response& response) {
        response.set_content(controller.start().dump(2), "application/json");
    });

    server.Post("/commands/stop", [&](const httplib::Request&, httplib::Response& response) {
        response.set_content(controller.stop().dump(2), "application/json");
    });

    server.Post("/commands/reset-reference", [&](const httplib::Request&, httplib::Response& response) {
        response.status = 501;
        response.set_content(controller.reset_reference().dump(2), "application/json");
    });

    server.Post("/commands/calibrate", [&](const httplib::Request& request, httplib::Response& response) {
        try
        {
            const auto payload = nlohmann::json::parse(request.body);
            response.set_content(controller.apply_calibration_payload(payload).dump(2), "application/json");
        }
        catch (const nlohmann::json::parse_error& exception)
        {
            response.status = 400;
            response.set_content(
                nlohmann::json{
                    {"accepted", false},
                    {"error", "invalid_json"},
                    {"message", exception.what()},
                }
                    .dump(2),
                "application/json");
        }
    });

    server.Get("/healthcheck", [&](const httplib::Request&, httplib::Response& response) {
        response.set_content(controller.healthcheck().dump(2), "application/json");
    });
}
} // namespace visiondarts
