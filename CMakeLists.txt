cmake_minimum_required(VERSION 3.24)

project(vision_darts_offline LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs calib3d)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_library(visiondarts
    src/core/types.cpp
    src/core/json_utils.cpp
    src/core/config.cpp
    src/core/board_model.cpp
    src/vision/calibration.cpp
    src/vision/replay.cpp
    src/vision/impact_detector.cpp
    src/vision/fusion_engine.cpp
    src/api/event_publisher.cpp
    src/app/offline_engine.cpp
    src/app/service_controller.cpp
)

target_include_directories(visiondarts
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(visiondarts
    PUBLIC
        nlohmann_json::nlohmann_json
        httplib::httplib
        ${OpenCV_LIBS}
)

if(MSVC)
    target_compile_options(visiondarts PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(visiondarts PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(vision_replay apps/vision_replay.cpp)
target_link_libraries(vision_replay PRIVATE visiondarts)

add_executable(vision_service apps/vision_service.cpp)
target_link_libraries(vision_service PRIVATE visiondarts)

add_executable(mock_backend apps/mock_backend.cpp)
target_link_libraries(mock_backend PRIVATE visiondarts)

add_executable(vision_calibration_check apps/vision_calibration_check.cpp)
target_link_libraries(vision_calibration_check PRIVATE visiondarts)

add_executable(visiondarts_tests tests/test_main.cpp)
target_link_libraries(visiondarts_tests PRIVATE visiondarts)

if(WIN32)
    get_filename_component(VISIONDARTS_PACKAGE_SHARE_DIR "${OpenCV_DIR}" DIRECTORY)
    get_filename_component(VISIONDARTS_PACKAGE_ROOT "${VISIONDARTS_PACKAGE_SHARE_DIR}" DIRECTORY)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(GLOB VISIONDARTS_RUNTIME_DLLS "${VISIONDARTS_PACKAGE_ROOT}/debug/bin/*.dll")
    else()
        file(GLOB VISIONDARTS_RUNTIME_DLLS "${VISIONDARTS_PACKAGE_ROOT}/bin/*.dll")
    endif()

    add_custom_target(visiondarts_runtime_dlls ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${VISIONDARTS_RUNTIME_DLLS}
            $<TARGET_FILE_DIR:vision_replay>
        COMMAND_EXPAND_LISTS
        DEPENDS vision_replay
    )
endif()

add_test(NAME visiondarts_tests COMMAND visiondarts_tests)
