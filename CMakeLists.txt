cmake_minimum_required(VERSION 3.8)

project("Vulkan Test")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
add_subdirectory(deps/glfw)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(deps/glfw/include)

add_executable(test src/main.cpp src/vulkan/vulkan_init.cpp)
target_link_libraries(test ${Vulkan_LIBRARY} glfw ${GLFW_LIBRARIES})

