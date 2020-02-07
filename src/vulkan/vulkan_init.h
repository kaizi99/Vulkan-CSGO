// Copyright (C) 2020 Kai-Uwe Zimdars
/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct vulkan_objects {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueFamilyIndices indices;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
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
