#pragma once

#include <atomic>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

#include <opencv2/core.hpp>
#include <nlohmann/json.hpp>

#include "visiondarts/api/event_publisher.hpp"
#include "visiondarts/app/live_engine.hpp"
#include "visiondarts/app/offline_engine.hpp"
#include "visiondarts/core/config.hpp"
#include "visiondarts/core/types.hpp"
#include "visiondarts/vision/live_camera_source.hpp"
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
    void replay_worker_loop();
    void live_worker_loop();
    void ensure_live_camera_source_open();
    void load_live_calibrations();
    void capture_live_reference();
    void set_last_error(const std::string& message);
    std::optional<CalibrationData> find_override(int camera_id) const;
    std::filesystem::path calibration_path_for_camera(int camera_id) const;

    AppConfig config_;
    ReplayFrameSource frame_source_{};
    OfflineVisionEngine engine_;
    LiveVisionEngine live_engine_;
    LiveCameraSource live_camera_source_;
    HttpEventPublisher publisher_;
    mutable std::mutex mutex_;
    std::unordered_map<int, CalibrationData> calibration_overrides_;
    std::unordered_map<int, CalibrationData> live_calibrations_;
    std::unordered_map<int, cv::Mat> live_references_;
    std::thread worker_;
    std::atomic<bool> stop_requested_{false};
    ServiceState state_ = ServiceState::Idle;
    std::string current_scenario_;
    std::optional<std::string> last_error_;
    bool calibration_loaded_ = false;
    int cameras_configured_ = 0;
    int cameras_opened_ = 0;
    std::uint64_t shot_sequence_ = 1;
};
} // namespace visiondarts
