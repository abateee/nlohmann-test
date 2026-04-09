#include "visiondarts/api/event_publisher.hpp"

#include <chrono>
#include <regex>
#include <stdexcept>

#include <httplib.h>

namespace visiondarts
{
namespace
{
struct ParsedUrl
{
    std::string host;
    int port = 80;
    std::string path = "/";
};

ParsedUrl parse_http_url(const std::string& url)
{
    const std::regex pattern(R"(^http://([^/:]+)(?::(\d+))?(\/.*)?$)");
    std::smatch match;
    if (!std::regex_match(url, match, pattern))
    {
        throw std::runtime_error("URL HTTP invalide: " + url);
    }

    ParsedUrl parsed;
    parsed.host = match[1].str();
    parsed.port = match[2].matched ? std::stoi(match[2].str()) : 80;
    parsed.path = match[3].matched ? match[3].str() : "/";
    return parsed;
}
} // namespace

HttpEventPublisher::HttpEventPublisher(BackendConfig config)
    : config_(std::move(config))
{
}

HttpEventPublisher::~HttpEventPublisher()
{
    stop();
}

void HttpEventPublisher::start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (started_)
    {
        return;
    }

    stop_requested_ = false;
    started_ = true;
    worker_ = std::thread(&HttpEventPublisher::worker_loop, this);
}

void HttpEventPublisher::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!started_)
        {
            return;
        }
        stop_requested_ = true;
    }

    condition_.notify_all();
    if (worker_.joinable())
    {
        worker_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    started_ = false;
}

void HttpEventPublisher::enqueue_event(const nlohmann::json& event)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(event);
    }
    condition_.notify_one();
}

void HttpEventPublisher::set_error_callback(std::function<void(const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    error_callback_ = std::move(callback);
}

std::optional<std::string> HttpEventPublisher::last_error() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return last_error_;
}

void HttpEventPublisher::worker_loop()
{
    for (;;)
    {
        nlohmann::json event;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                return stop_requested_ || !queue_.empty();
            });

            if (stop_requested_ && queue_.empty())
            {
                break;
            }

            event = queue_.front();
            queue_.pop_front();
        }

        std::string error_message;
        bool delivered = false;
        for (int attempt = 0; attempt < config_.post_retry_count; ++attempt)
        {
            if (post_once(event, error_message))
            {
                delivered = true;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (!delivered)
        {
            std::function<void(const std::string&)> callback;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                last_error_ = error_message;
                callback = error_callback_;
            }

            if (callback)
            {
                callback(error_message);
            }
        }
    }
}

bool HttpEventPublisher::post_once(const nlohmann::json& event, std::string& error_message) const
{
    const ParsedUrl url = parse_http_url(config_.post_url);
    httplib::Client client(url.host, url.port);
    client.set_connection_timeout(config_.post_timeout_ms / 1000, (config_.post_timeout_ms % 1000) * 1000);
    client.set_read_timeout(config_.post_timeout_ms / 1000, (config_.post_timeout_ms % 1000) * 1000);
    client.set_write_timeout(config_.post_timeout_ms / 1000, (config_.post_timeout_ms % 1000) * 1000);

    const auto response = client.Post(url.path.c_str(), event.dump(), "application/json");
    if (!response)
    {
        error_message = "Echec HTTP pendant l'envoi d'un evenement vers le backend.";
        return false;
    }

    if (response->status < 200 || response->status >= 300)
    {
        error_message = "Le backend a repondu avec un code HTTP " + std::to_string(response->status) + ".";
        return false;
    }

    return true;
}
} // namespace visiondarts
