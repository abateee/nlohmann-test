#ifdef _WIN32

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/imgproc.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/vision/calibration.hpp"
#include "visiondarts/vision/live_camera_source.hpp"

namespace
{
constexpr int kRequiredPoints = 4;

const std::vector<cv::Point2d> kBoardPoints = {
    {0.0, 1.0},
    {1.0, 0.0},
    {0.0, -1.0},
    {-1.0, 0.0},
};

const std::vector<std::string> kPointLabels = {
    "haut double",
    "droite double",
    "bas double",
    "gauche double",
};

struct UiState
{
    visiondarts::LiveCameraConfig camera{};
    cv::Mat frame_bgr;
    cv::Mat display_bgra;
    std::vector<cv::Point2d> points;
    bool save_requested = false;
    bool quit_requested = false;
};

std::vector<visiondarts::LiveCameraConfig> enabled_cameras(const visiondarts::AppConfig& config)
{
    std::vector<visiondarts::LiveCameraConfig> cameras;
    if (config.cameras.empty())
    {
        cameras.push_back(visiondarts::LiveCameraConfig{});
        return cameras;
    }

    for (const auto& camera : config.cameras)
    {
        if (camera.enabled)
        {
            cameras.push_back(camera);
        }
    }
    return cameras;
}

visiondarts::LiveCameraConfig camera_by_id(
    const std::vector<visiondarts::LiveCameraConfig>& cameras,
    int camera_id)
{
    const auto it = std::find_if(cameras.begin(), cameras.end(), [camera_id](const auto& camera) {
        return camera.camera_id == camera_id;
    });
    if (it == cameras.end())
    {
        throw std::runtime_error("camera_id introuvable dans la config: " + std::to_string(camera_id));
    }
    return *it;
}

std::vector<visiondarts::LiveCameraConfig> select_cameras(
    const visiondarts::AppConfig& config,
    int argc,
    char** argv)
{
    const auto cameras = enabled_cameras(config);
    if (cameras.empty())
    {
        throw std::runtime_error("Aucune camera active dans la config.");
    }

    if (argc >= 3 && std::string(argv[2]) == "--all")
    {
        return cameras;
    }

    if (argc >= 3)
    {
        return {camera_by_id(cameras, std::stoi(argv[2]))};
    }

    if (cameras.size() == 1)
    {
        return cameras;
    }

    throw std::runtime_error("Plusieurs cameras sont configurees. Fournis un camera_id ou --all.");
}

UiState* state_from_window(HWND hwnd)
{
    return reinterpret_cast<UiState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

RECT image_rect_in_client(const RECT& client, const cv::Size& image_size)
{
    RECT result = client;
    if (image_size.width <= 0 || image_size.height <= 0)
    {
        return result;
    }

    const int client_width = client.right - client.left;
    const int client_height = client.bottom - client.top;
    const double scale = std::min(
        static_cast<double>(client_width) / static_cast<double>(image_size.width),
        static_cast<double>(client_height) / static_cast<double>(image_size.height));
    const int width = static_cast<int>(image_size.width * scale);
    const int height = static_cast<int>(image_size.height * scale);
    result.left = client.left + ((client_width - width) / 2);
    result.top = client.top + ((client_height - height) / 2);
    result.right = result.left + width;
    result.bottom = result.top + height;
    return result;
}

void add_click_point(HWND hwnd, int mouse_x, int mouse_y)
{
    auto* state = state_from_window(hwnd);
    if (state == nullptr || state->frame_bgr.empty() || state->points.size() >= kRequiredPoints)
    {
        return;
    }

    RECT client;
    GetClientRect(hwnd, &client);
    const RECT image_rect = image_rect_in_client(client, state->frame_bgr.size());
    if (mouse_x < image_rect.left || mouse_x >= image_rect.right || mouse_y < image_rect.top || mouse_y >= image_rect.bottom)
    {
        return;
    }

    const double scale_x = static_cast<double>(state->frame_bgr.cols) / static_cast<double>(image_rect.right - image_rect.left);
    const double scale_y = static_cast<double>(state->frame_bgr.rows) / static_cast<double>(image_rect.bottom - image_rect.top);
    state->points.emplace_back(
        (mouse_x - image_rect.left) * scale_x,
        (mouse_y - image_rect.top) * scale_y);
}

void draw_overlay(UiState& state)
{
    cv::Mat display = state.frame_bgr.clone();
    const std::string next_label = state.points.size() < kPointLabels.size()
        ? kPointLabels.at(state.points.size())
        : "pret a sauvegarder";

    cv::putText(
        display,
        "Camera " + std::to_string(state.camera.camera_id) + " - clic: " + next_label,
        {20, 32},
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        {0, 255, 255},
        2);
    cv::putText(
        display,
        "U=annuler  R=recommencer  S=sauver  Q/ESC=quitter",
        {20, 64},
        cv::FONT_HERSHEY_SIMPLEX,
        0.7,
        {255, 255, 255},
        2);

    for (std::size_t index = 0; index < state.points.size(); ++index)
    {
        const auto point = state.points.at(index);
        cv::circle(display, point, 7, {0, 255, 0}, cv::FILLED);
        cv::putText(
            display,
            std::to_string(index + 1),
            point + cv::Point2d{10.0, -10.0},
            cv::FONT_HERSHEY_SIMPLEX,
            0.7,
            {0, 255, 0},
            2);
    }

    for (std::size_t index = 1; index < state.points.size(); ++index)
    {
        cv::line(display, state.points.at(index - 1), state.points.at(index), {0, 255, 0}, 2);
    }
    if (state.points.size() == kRequiredPoints)
    {
        cv::line(display, state.points.back(), state.points.front(), {0, 255, 0}, 2);
        cv::putText(
            display,
            "4 points OK - appuie sur S pour sauvegarder",
            {20, 96},
            cv::FONT_HERSHEY_SIMPLEX,
            0.75,
            {0, 255, 0},
            2);
    }

    cv::cvtColor(display, state.display_bgra, cv::COLOR_BGR2BGRA);
}

void paint_window(HWND hwnd)
{
    auto* state = state_from_window(hwnd);
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(hwnd, &paint);

    if (state != nullptr && !state->display_bgra.empty())
    {
        RECT client;
        GetClientRect(hwnd, &client);
        const RECT image_rect = image_rect_in_client(client, state->frame_bgr.size());

        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = state->display_bgra.cols;
        info.bmiHeader.biHeight = -state->display_bgra.rows;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        StretchDIBits(
            dc,
            image_rect.left,
            image_rect.top,
            image_rect.right - image_rect.left,
            image_rect.bottom - image_rect.top,
            0,
            0,
            state->display_bgra.cols,
            state->display_bgra.rows,
            state->display_bgra.data,
            &info,
            DIB_RGB_COLORS,
            SRCCOPY);
    }

    EndPaint(hwnd, &paint);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        add_click_point(hwnd, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        return 0;
    case WM_KEYDOWN:
    {
        auto* state = state_from_window(hwnd);
        if (state == nullptr)
        {
            return 0;
        }
        if (wparam == VK_ESCAPE || wparam == 'Q')
        {
            state->quit_requested = true;
            return 0;
        }
        if (wparam == 'R')
        {
            state->points.clear();
            return 0;
        }
        if (wparam == 'U' && !state->points.empty())
        {
            state->points.pop_back();
            return 0;
        }
        if (wparam == 'S')
        {
            state->save_requested = true;
            return 0;
        }
        return 0;
    }
    case WM_PAINT:
        paint_window(hwnd);
        return 0;
    case WM_CLOSE:
    {
        auto* state = state_from_window(hwnd);
        if (state != nullptr)
        {
            state->quit_requested = true;
        }
        return 0;
    }
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

ATOM register_window_class(HINSTANCE instance)
{
    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = L"VisionDartsCalibrationWindow";
    window_class.hCursor = LoadCursor(nullptr, IDC_CROSS);
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    return RegisterClassW(&window_class);
}

bool calibrate_camera_ui(HINSTANCE instance, const visiondarts::LiveCameraConfig& camera)
{
    visiondarts::LiveCameraSource source;
    source.open({camera});

    UiState state;
    state.camera = camera;
    const std::wstring title = L"Calibration camera " + std::to_wstring(camera.camera_id);

    RECT window_rect{0, 0, std::max(800, camera.width), std::max(600, camera.height)};
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        L"VisionDartsCalibrationWindow",
        title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);
    if (hwnd == nullptr)
    {
        throw std::runtime_error("Impossible de creer la fenetre de calibration.");
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&state));
    std::cout << "Calibration camera " << camera.camera_id << " (device_index=" << camera.device_index << ")\n";
    std::cout << "Clique dans l'ordre: haut double, droite double, bas double, gauche double.\n";

    bool saved = false;
    while (!state.quit_requested)
    {
        MSG message;
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        const auto frames = source.capture_frames();
        if (!frames.empty() && !frames.front().image.empty())
        {
            state.frame_bgr = frames.front().image.clone();
            draw_overlay(state);
            InvalidateRect(hwnd, nullptr, FALSE);
            UpdateWindow(hwnd);
        }

        if (state.save_requested)
        {
            state.save_requested = false;
            if (state.points.size() != kRequiredPoints)
            {
                std::cerr << "Sauvegarde impossible: " << kRequiredPoints << " points sont requis.\n";
            }
            else
            {
                const auto calibration = visiondarts::compute_calibration(
                    camera.camera_id,
                    state.points,
                    kBoardPoints,
                    0.0);
                visiondarts::CalibrationStore::save_json(camera.calibration_path, calibration);
                std::cout << "Calibration sauvegardee: " << camera.calibration_path.string() << '\n';
                saved = true;
                break;
            }
        }

        Sleep(30);
    }

    DestroyWindow(hwnd);
    source.close();
    return saved;
}

void print_usage()
{
    std::cout << "Usage:\n"
              << "  vision_live_calibrate_ui.exe config/live_windows.json\n"
              << "  vision_live_calibrate_ui.exe config/live_windows.json <camera_id>\n"
              << "  vision_live_calibrate_ui.exe config/live_windows.json --all\n";
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2)
        {
            print_usage();
            return 1;
        }

        HINSTANCE instance = GetModuleHandle(nullptr);
        register_window_class(instance);

        const auto config = visiondarts::load_app_config(argv[1]);
        const auto cameras = select_cameras(config, argc, argv);

        bool all_saved = true;
        for (const auto& camera : cameras)
        {
            all_saved = calibrate_camera_ui(instance, camera) && all_saved;
        }

        return all_saved ? 0 : 2;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }
}

#else

#include <iostream>

int main()
{
    std::cerr << "vision_live_calibrate_ui est disponible uniquement sur Windows.\n";
    return 1;
}

#endif
