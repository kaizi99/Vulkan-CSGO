cmake_minimum_required(VERSION 3.8)

project("Vulkan Test")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
add_subdirectory(deps/glfw)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(deps/glfw/include)
include_directories(deps/glm)

add_executable(test src/main.cpp src/vulkan/vulkan_init.cpp src/vulkan/vulkan_renderer.cpp src/dearimgui/imgui.cpp src/dearimgui/imgui_widgets.cpp src/dearimgui/imgui_draw.cpp src/dearimgui/imgui_demo.cpp src/dearimgui/imgui_vulkan.cpp src/vulkan/vulkan_utils.cpp)
target_link_libraries(test ${Vulkan_LIBRARY} glfw ${GLFW_LIBRARIES})