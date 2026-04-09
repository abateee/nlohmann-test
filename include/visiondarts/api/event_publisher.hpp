#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

#include "visiondarts/core/config.hpp"

namespace visiondarts
{
class HttpEventPublisher
{
  public:
    explicit HttpEventPublisher(BackendConfig config);
    ~HttpEventPublisher();

    void start();
    void stop();
    void enqueue_event(const nlohmann::json& event);
    void set_error_callback(std::function<void(const std::string&)> callback);
    std::optional<std::string> last_error() const;

  private:
    void worker_loop();
    bool post_once(const nlohmann::json& event, std::string& error_message) const;

    BackendConfig config_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<nlohmann::json> queue_;
    std::thread worker_;
    std::function<void(const std::string&)> error_callback_;
    bool stop_requested_ = false;
    bool started_ = false;
    std::optional<std::string> last_error_;
};
} // namespace visiondarts

