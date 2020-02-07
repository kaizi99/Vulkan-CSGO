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

#include "vulkan_init.h"

#include <vector>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <optional>
#include <set>
#include <algorithm>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

static bool createInstance(vulkan_init_parameters init_params, vulkan_objects* objects) {
	VkApplicationInfo appInfo = {};

	appInfo.pApplicationName = "Hello Trianlge";
	appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
	appInfo.pEngineName = "kaizi Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Check if all the extensions we need are actually supported
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);	
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());	

	uint32_t matchingExtensions = 0;
	for (const char*  extensionName : init_params.instanceExtensions) {
		bool extensionFound = 0;

		for (const auto& extension : extensions) {
			if (std::strcmp(extensionName, extension.extensionName)) {
				matchingExtensions++;
				extensionFound = true;
				break;
			}
		}

		if (extensionFound == false)
			break;
	}

	if (matchingExtensions != init_params.instanceExtensions.size()) {
		std::cerr << "Missing vulkan extensions!" << std::endl;
		return false;
	}

	createInfo.enabledExtensionCount = matchingExtensions;
	createInfo.ppEnabledExtensionNames = init_params.instanceExtensions.data();

	// Check if we need valiation layers and if we can use them
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	
	uint32_t matchingLayers = 0;
	for (const char* layerName : init_params.instanceLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				matchingLayers++;
				layerFound = true;
				break;
			}
		}

        if (!layerFound)
            break;
	}

	if (matchingLayers != init_params.instanceLayers.size()) {
		std::cerr << "Missing vulkan layers" << std::endl;
		return false;
	}

	createInfo.enabledLayerCount = matchingLayers;
	createInfo.ppEnabledLayerNames = init_params.instanceLayers.data();

	// Create the instance
	if (vkCreateInstance(&createInfo, nullptr, &(objects->instance)) != VK_SUCCESS) {
		std::cerr << "Could not create instance!" << std::endl;
		return false;
	}

	return true;
}

static bool initVulkanDebugUtils(vulkan_init_parameters init_params, vulkan_objects* objects) {	
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = objects;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(objects->instance, "vkCreateDebugUtilsMessengerEXT");
	if (func(objects->instance, &createInfo, nullptr, &(objects->debugMessenger)) != VK_SUCCESS) {
		std::cerr << "Could not initialize debug messenger" << std::endl;
		return false;
	}
	return true;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

		i++;
	}

	return indices;
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions, VkFormat imageFormat, VkColorSpaceKHR imageColorSpace) {
	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	uint32_t matchingExtensions = 0;
	for (const char* requiredExt : deviceExtensions) {
	    bool extensionFound = false;

        for (const auto& extension : availableExtensions) {
            if (std::strcmp(extension.extensionName, requiredExt) == 0) {
                matchingExtensions++;
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            break;
        }
	}

	bool swapChainAdequate = false;
	if (matchingExtensions == deviceExtensions.size()) {
	    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);

	    bool correctFormatFound = false;

	    for (const auto& availableFormat : swapChainSupport.formats) {
	        if (availableFormat.format == imageFormat && availableFormat.colorSpace == imageColorSpace) {
                correctFormatFound = true;
	        }
	    }

	    swapChainAdequate = correctFormatFound && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && swapChainAdequate;
}

static bool init_surface(vulkan_init_parameters init_params, vulkan_objects* objects) {
    if (glfwCreateWindowSurface(objects->instance, init_params.window, nullptr, &(objects->surface)) != VK_SUCCESS) {
        return false;
    }

    return true;
}

static bool init_physical_device(vulkan_init_parameters init_params, vulkan_objects* objects) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(objects->instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		std::cerr << "Failed to find GPU with Vulkan support!" << std::endl;
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(objects->instance, &deviceCount, devices.data());

	// Check for every device if it is suitable and pick the best one
	uint32_t maxScore = 0;
	VkPhysicalDevice highestDevice;
	for (VkPhysicalDevice device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		uint32_t score = isDeviceSuitable(device, objects->surface, init_params.deviceExtensions, init_params.swapchainImageFormat, init_params.swapchainColorSpace);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score = score << 1;

		if (score > maxScore) {
			maxScore = score;
			highestDevice = device;
		}	
	}

	if (maxScore == 0) {
		std::cerr << "Cannot find suitable device" << std::endl;
		return false;
	}		

	objects->physicalDevice = highestDevice;

	return true;
}

static bool init_device_and_queue(vulkan_init_parameters init_params, vulkan_objects* objects) {
	QueueFamilyIndices indices = findQueueFamilies(objects->physicalDevice, objects->surface);
	objects->indices = indices;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
	}
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = init_params.deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = init_params.deviceExtensions.data();
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(objects->physicalDevice, &createInfo, nullptr, &(objects->device)) != VK_SUCCESS) {
		std::cerr << "Could not create device" << std::endl;
		return false;
	}

	vkGetDeviceQueue(objects->device, indices.graphicsFamily.value(), 0, &(objects->graphicsQueue));
	vkGetDeviceQueue(objects->device, indices.presentFamily.value(), 0, &(objects->presentQueue));

	return true;
}

static bool init_swapchain(vulkan_init_parameters init_params, vulkan_objects* objects) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(objects->physicalDevice, objects->surface);

    VkSurfaceFormatKHR surfaceFormat;
    for (const auto& availableFormat : swapChainSupport.formats) {
        if (availableFormat.format == init_params.swapchainImageFormat && availableFormat.colorSpace == init_params.swapchainColorSpace) {
            surfaceFormat = availableFormat;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkExtent2D swapExtent;
    if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX) {
        swapExtent = swapChainSupport.capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {init_params.width, init_params.height};

        actualExtent.width = std::clamp(actualExtent.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);

        swapExtent = actualExtent;
    }

    uint32_t imageCount = std::max(swapChainSupport.capabilities.minImageCount + 1, swapChainSupport.capabilities.maxImageCount);
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = objects->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(objects->physicalDevice, objects->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_FALSE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(objects->device, &createInfo, nullptr, &objects->swapchain)) {
        std::cerr << "Could not create Swapchain!" << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(objects->device, objects->swapchain, &imageCount, nullptr);
    objects->swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(objects->device, objects->swapchain, &imageCount, objects->swapchainImages.data());

    objects->swapchainExtent = swapExtent;
    objects->swapchainImageFormat = surfaceFormat.format;

    return true;
}

static bool init_image_views(vulkan_init_parameters init_params, vulkan_objects* objects) {
    objects->swapchainImageViews.resize(objects->swapchainImages.size());

    for (size_t i = 0; i < objects->swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = objects->swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = objects->swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(objects->device, &createInfo, nullptr, &objects->swapchainImageViews[i]) != VK_SUCCESS) {
            std::cerr << "Could not create image view!" << std::endl;
            return false;
        }
    }

    return true;
}

bool init_vulkan(vulkan_init_parameters init_params, vulkan_objects* objects) {
	init_params.useValidationLayers = true;

	if (!createInstance(init_params, objects)) {
		return false;
	}

	if (init_params.useValidationLayers) {
		if (!initVulkanDebugUtils(init_params, objects))
			return false;
	}

	if (!init_surface(init_params, objects)) {
        return false;
    }

	if (!init_physical_device(init_params, objects)) {
		return false;
	}

	if (!init_device_and_queue(init_params, objects)) {
	    return false;
	}

	if (!init_swapchain(init_params, objects)) {
        return false;
    }

	if (!init_image_views(init_params, objects)) {
		return false;
	}
	
	return true;
}

void deinit_vulkan(vulkan_objects* objects) {
    for (auto imageView : objects->swapchainImageViews) {
        vkDestroyImageView(objects->device, imageView, nullptr);
    }

    if (objects->swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(objects->device, objects->swapchain, nullptr);
    if (objects->device != VK_NULL_HANDLE) vkDestroyDevice(objects->device, nullptr);
    if (objects->surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(objects->instance, objects->surface, nullptr);

    if (objects->debugMessenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(objects->instance, "vkDestroyDebugUtilsMessengerEXT");
        func(objects->instance, objects->debugMessenger, nullptr);
    }

    if (objects->instance != VK_NULL_HANDLE) vkDestroyInstance(objects->instance, nullptr);
}
