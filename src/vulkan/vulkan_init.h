#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct vulkan_objects {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
};

struct vulkan_init_parameters {
	bool useValidationLayers;
};

bool init_vulkan(vulkan_init_parameters init_params, vulkan_objects* objects);
void deinit_vulkan(vulkan_objects* objects);
