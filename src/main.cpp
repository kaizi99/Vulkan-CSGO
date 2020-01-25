#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_init.h"
#include "vulkan/vulkan_renderer.h"
#include "dearimgui/imgui_vulkan.h"

int main() {
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "GLFW cant be initialized" << std::endl;
	}

	if (glfwVulkanSupported() != GLFW_TRUE) {
		std::cout << "Vulkan is not supported on this platform!" << std::endl;
	}

    vulkan_init_parameters init_params = {};

	init_params.width = 1280;
	init_params.height = 720;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(init_params.width, init_params.height, "Vulkan", nullptr, nullptr);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions;
    for (int i = 0; i < glfwExtensionCount; i++) {
        init_params.instanceExtensions.push_back(glfwExtensions[i]);
    }

    init_params.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

//#ifndef NDEBUG
	init_params.useValidationLayers = true;
    init_params.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    init_params.instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
//#else
//	init_params.useValidationLayers = false;
//#endif

	init_params.window = window;
    init_params.swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    init_params.swapchainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	vulkan_objects objects = {};
	if (!init_vulkan(init_params, &objects)) {
		std::cout << "Error initializing Vulkan!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
	}

	vulkan_renderer* renderer = init_renderer(objects);
	if (renderer == nullptr) {
		std::cout << "Error initializing Vulkan!" << std::endl;
		deinit_vulkan(&objects);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
	}
	std::cout << "Vulkan has been initialized" << std::endl;

	imguivk imgui;
	imguivk_init(renderer, &imgui, window);

	std::cout << "DearImGui has been intialized" << std::endl;

	while (!glfwWindowShouldClose(window)) {
	    glfwPollEvents();

		renderer_begin_frame(renderer);

		imguivk_beginFrame(renderer, &imgui);

		bool metrics = true;
		ImGui::ShowMetricsWindow(&metrics);

		imguivk_endFrame(renderer, &imgui);

		renderer_end_frame(renderer);
	}

	vkQueueWaitIdle(renderer->init_objects.graphicsQueue);
	imguivk_deinit(renderer, &imgui);
	deinit_renderer(renderer);
	deinit_vulkan(&objects);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
