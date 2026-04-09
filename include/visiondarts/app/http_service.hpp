#pragma once

namespace httplib
{
class Server;
}

namespace visiondarts
{
class ServiceController;

void configure_service_socket_options(httplib::Server& server);
void register_service_routes(httplib::Server& server, ServiceController& controller);
} // namespace visiondarts
