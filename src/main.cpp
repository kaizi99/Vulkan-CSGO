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

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_init.h"
#include "vulkan/vulkan_renderer.h"
#include "dearimgui/imgui_vulkan.h"
#include "bsp/bsp_loader.h"
#include "camera.h"
#include "bsp/vpk.h"
#include "bsp/bsp_rendering.h"

#include <glm/gtc/matrix_transform.hpp>

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

	std::string csgo_folder = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo\\";
	//std::string csgo_folder = "/Users/kaizi99/Library/Application Support/Steam/steamapps/common/Counter-Strike Global Offensive/csgo/";
	std::cout << "Loading: de_train.bsp" << std::endl;

	bsp_parsed* parsed = load_bsp(csgo_folder + "maps/de_train.bsp");
	//bsp_rendering_data bsp_rendering = bsp_rendering_prepare(parsed, renderer);

	//std::string gmod_folder = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\GarrysMod\\garrysmod\\";
	//bsp_parsed* parsed = load_bsp(gmod_folder + "maps\\gm_construct.bsp");

	assert(parsed != nullptr);

	//bsp_rendering_data bsp_rendering = bsp_rendering_prepare(parsed, renderer);

	/*
    vpk_directory* vpk = load_vpk(csgo_folder, "pak01");

	for (int i = 0; i < parsed->textureCount; i++) {
		std::string textureName = parsed->textures[i].textureName;
		std::string toSearch = "materials/" + textureName;

		auto entry = vpk->entries["vmt"].find(toSearch);

		if (entry == vpk->entries["vmt"].end()) {

		}

		std::cout << textureName << std::endl;
	}
	*/

	camera c;
	c.position = glm::vec3(-50, -1300, -20);
	c.rotation = glm::vec3(0.0f);
	c.fov = 90;
	c.aspectRatio = (float)init_params.width / (float)init_params.height;

	while (!glfwWindowShouldClose(window)) {
	    glfwPollEvents();

		renderer_begin_frame(renderer);

		imguivk_beginFrame(renderer, &imgui);

		bool metrics = true;
		ImGui::ShowMetricsWindow(&metrics);

		updateCamera(&c, window);
		//bsp_render(&bsp_rendering, renderer, &c);

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
