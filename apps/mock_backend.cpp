#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>

#include <httplib.h>

int main(int argc, char** argv)
{
    const int port = argc >= 2 ? std::stoi(argv[1]) : 8080;
    const std::string output_path = argc >= 3 ? argv[2] : "build/mock_backend_events.jsonl";

    const std::filesystem::path output_parent = std::filesystem::path(output_path).parent_path();
    if (!output_parent.empty())
    {
        std::filesystem::create_directories(output_parent);
    }
    std::mutex file_mutex;
    httplib::Server server;

    server.Post("/vision/events", [&](const httplib::Request& request, httplib::Response& response) {
        {
            std::lock_guard<std::mutex> lock(file_mutex);
            std::ofstream output(output_path, std::ios::app);
            output << request.body << '\n';
        }

        std::cout << request.body << '\n';
        response.set_content("{\"accepted\":true}", "application/json");
    });

    server.Get("/health", [&](const httplib::Request&, httplib::Response& response) {
        response.set_content("{\"status\":\"ok\"}", "application/json");
    });

    std::cout << "Mock backend sur http://127.0.0.1:" << port << '\n';
    server.listen("127.0.0.1", port);
    return 0;
}
