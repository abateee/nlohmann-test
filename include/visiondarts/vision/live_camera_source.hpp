#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "visiondarts/core/config.hpp"

namespace visiondarts
{
struct LiveCameraFrame
{
    LiveCameraConfig config{};
    cv::Mat image;
};

int opencv_capture_backend(const std::string& capture_backend);

class LiveCameraSource
{
  public:
    void open(const std::vector<LiveCameraConfig>& cameras);
    void close();
    bool is_open() const;
    int configured_count() const;
    int opened_count() const;

    bool grab_all();
    std::vector<LiveCameraFrame> retrieve_all();
    std::vector<LiveCameraFrame> capture_frames();

  private:
    struct CameraHandle
    {
        LiveCameraConfig config{};
        cv::VideoCapture capture;
        bool opened = false;
    };

    std::vector<CameraHandle> cameras_;
};
} // namespace visiondarts
