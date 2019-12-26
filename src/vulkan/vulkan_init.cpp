#include "vulkan_init.h"

#include <vector>
#include <iostream>
#include <string>
#include <optional>

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

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> requiredExtensions;	
	for (int i = 0; i < glfwExtensionCount; i++) {
		requiredExtensions.push_back(glfwExtensions[i]);
	}

	if (init_params.useValidationLayers)
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// Check if all the extensions GLFW needs are actually supported
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);	
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());	

	uint32_t matchingExtensions = 0;
	for (const char*  extensionName : requiredExtensions) {
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

	if (matchingExtensions != requiredExtensions.size()) {
		std::cerr << "Missing vulkan extensions!" << std::endl;
		return false;
	}

	createInfo.enabledExtensionCount = matchingExtensions;
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// Check if we need valiation layers and if we can use them		
	std::vector<const char*> requestedLayers;

	if (init_params.useValidationLayers)
		requestedLayers.push_back("VK_LAYER_KHRONOS_validation");

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	
	uint32_t matchingLayers = 0;
	for (const char* layerName : requestedLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				matchingLayers++;
				layerFound = true;
				break;
			}
		}

		if (layerFound == false)
			break;
	}

	if (matchingLayers != requestedLayers.size()) {
		std::cerr << "Missing vulkan layers" << std::endl;
		return false;
	}

	createInfo.enabledLayerCount = requestedLayers.size();
	createInfo.ppEnabledLayerNames = requestedLayers.data(); 

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


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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

		i++;
	}

	return indices;
}

static bool isDeviceSuitable(const VkPhysicalDevice& device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	return indices.isComplete();
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

		uint32_t score = isDeviceSuitable(device);

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
	QueueFamilyIndices indices = findQueueFamilies(objects->physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(objects->physicalDevice, &createInfo, nullptr, &(objects->device)) != VK_SUCCESS) {
		std::cerr << "Could not create device" << std::endl;
		return false;
	}

	vkGetDeviceQueue(objects->device, indices.graphicsFamily.value(), 0, &(objects->graphicsQueue));
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

	if (!init_physical_device(init_params, objects)) {
		return false;
	}

	if (!init_device_and_queue(init_params, objects)) {
	    return false;
	}
	
	return true;
}

void deinit_vulkan(vulkan_objects* objects) {
	vkDestroyDevice(objects->device, nullptr);
	if (objects->debugMessenger != 0) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(objects->instance, "vkDestroyDebugUtilsMessengerEXT");
		func(objects->instance, objects->debugMessenger, nullptr);
	}
	vkDestroyInstance(objects->instance, nullptr);
}
