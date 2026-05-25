#include "visiondarts/vision/live_camera_source.hpp"

#include <string>
#include <stdexcept>

namespace visiondarts
{
void LiveCameraSource::open(const std::vector<LiveCameraConfig>& cameras)
{
    close();

    for (const auto& config : cameras)
    {
        if (!config.enabled)
        {
            continue;
        }

        CameraHandle handle;
        handle.config = config;
        if (!handle.capture.open(config.device_index))
        {
            throw std::runtime_error("Impossible d'ouvrir la camera device_index=" + std::to_string(config.device_index));
        }

        if (config.width > 0)
        {
            handle.capture.set(cv::CAP_PROP_FRAME_WIDTH, config.width);
        }
        if (config.height > 0)
        {
            handle.capture.set(cv::CAP_PROP_FRAME_HEIGHT, config.height);
        }
        if (config.fps > 0)
        {
            handle.capture.set(cv::CAP_PROP_FPS, config.fps);
        }

        handle.opened = true;
        cameras_.push_back(std::move(handle));
    }

    if (cameras_.empty())
    {
        throw std::runtime_error("Aucune camera live active n'est configuree.");
    }
}

void LiveCameraSource::close()
{
    for (auto& camera : cameras_)
    {
        if (camera.capture.isOpened())
        {
            camera.capture.release();
        }
        camera.opened = false;
    }
    cameras_.clear();
}

bool LiveCameraSource::is_open() const
{
    return opened_count() > 0;
}

int LiveCameraSource::configured_count() const
{
    return static_cast<int>(cameras_.size());
}

int LiveCameraSource::opened_count() const
{
    int count = 0;
    for (const auto& camera : cameras_)
    {
        if (camera.opened && camera.capture.isOpened())
        {
            ++count;
        }
    }
    return count;
}

bool LiveCameraSource::grab_all()
{
    bool grabbed = false;
    for (auto& camera : cameras_)
    {
        if (camera.opened && camera.capture.isOpened())
        {
            grabbed = camera.capture.grab() || grabbed;
        }
    }
    return grabbed;
}

std::vector<LiveCameraFrame> LiveCameraSource::retrieve_all()
{
    std::vector<LiveCameraFrame> frames;
    for (auto& camera : cameras_)
    {
        if (!camera.opened || !camera.capture.isOpened())
        {
            continue;
        }

        cv::Mat image;
        if (camera.capture.retrieve(image) && !image.empty())
        {
            frames.push_back(LiveCameraFrame{camera.config, image});
        }
    }
    return frames;
}

std::vector<LiveCameraFrame> LiveCameraSource::capture_frames()
{
    if (!grab_all())
    {
        return {};
    }
    return retrieve_all();
}
} // namespace visiondarts
