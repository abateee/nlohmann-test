#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "visiondarts/api/event_publisher.hpp"
#include "visiondarts/app/offline_engine.hpp"
#include "visiondarts/core/config.hpp"
#include "visiondarts/core/types.hpp"
#include "visiondarts/vision/replay.hpp"

namespace visiondarts
{
class ServiceController
{
  public:
    explicit ServiceController(AppConfig config);
    ~ServiceController();

    nlohmann::json start();
    nlohmann::json stop();
    nlohmann::json reset_reference();
    nlohmann::json healthcheck() const;
    nlohmann::json apply_calibration_payload(const nlohmann::json& payload);

  private:
    void worker_loop();
    void set_last_error(const std::string& message);
    std::optional<CalibrationData> find_override(int camera_id) const;

    AppConfig config_;
    ReplayFrameSource frame_source_{};
    OfflineVisionEngine engine_;
    HttpEventPublisher publisher_;
    mutable std::mutex mutex_;
    std::unordered_map<int, CalibrationData> calibration_overrides_;
    std::thread worker_;
    std::atomic<bool> stop_requested_{false};
    ServiceState state_ = ServiceState::Idle;
    std::string current_scenario_;
    std::optional<std::string> last_error_;
    bool calibration_loaded_ = false;
    std::uint64_t shot_sequence_ = 1;
};
} // namespace visiondarts
