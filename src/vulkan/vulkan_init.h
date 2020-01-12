#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

struct vulkan_objects {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapchainFramebuffers;
};

struct vulkan_init_parameters {
	bool useValidationLayers;
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;
	std::vector<const char*> deviceExtensions;
	GLFWwindow* window;
	VkFormat swapchainImageFormat;
	VkColorSpaceKHR swapchainColorSpace;
	uint32_t width;
	uint32_t height;
};

bool init_vulkan(vulkan_init_parameters init_params, vulkan_objects* objects);
void deinit_vulkan(vulkan_objects* objects);
