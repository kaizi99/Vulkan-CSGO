#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_init.h"

bool initGLFWAndVulkan(GLFWwindow* window, VkInstance* instance);

int main() {
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "GLFW cant be initialized" << std::endl;
	}

	if (glfwVulkanSupported() != GLFW_TRUE) {
		std::cout << "Vulkan is not supported on this platform!" << std::endl;
	}

	vulkan_init_parameters init_params;

#ifdef NDEBUG
	init_params.useValidationLayers = true;
#else
	init_params.useValidationLayers = false;
#endif

	vulkan_objects objects;
	if (!init_vulkan(init_params, &objects)) {
		std::cout << "Error initializing Vulkan!" << std::endl;
	}

	deinit_vulkan(&objects);

	return 0;
}
