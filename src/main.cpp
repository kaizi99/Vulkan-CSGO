#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_init.h"
#include "vulkan/vulkan_renderer.h"
#include "dearimgui/imgui_vulkan.h"
#include "bsp/bsp_loader.h"
#include "camera.h"

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

	std::cout << "Loading: de_train.bsp" << std::endl;
	bsp_parsed* parsed = load_bsp(csgo_folder + "maps\\de_train.bsp");
	bsp_geometry_vulkan bsp_geometry = create_geometry_from_bsp(renderer, parsed);

	camera c;
	c.position = glm::vec3(-1000, -160, 1300);
	c.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	c.rotation = glm::rotate(c.rotation, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	c.rotation = glm::rotate(c.rotation, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	c.fov = 90;
	c.aspectRatio = (float)init_params.width / (float)init_params.height;

	while (!glfwWindowShouldClose(window)) {
	    glfwPollEvents();

		renderer_begin_frame(renderer);

		imguivk_beginFrame(renderer, &imgui);

		bool metrics = true;
		ImGui::ShowMetricsWindow(&metrics);

		// Render BSP
		vkCmdBindPipeline(renderer->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bsp_geometry.pipeline);

		VkBuffer vertexBuffers[] = { bsp_geometry.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(renderer->command_buffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(renderer->command_buffer, bsp_geometry.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		updateCamera(&c, window);
		glm::mat4 mvp = calculateViewProjection(c);
		vkCmdPushConstants(renderer->command_buffer, bsp_geometry.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

		vkCmdDrawIndexed(renderer->command_buffer, bsp_geometry.indicesCount, 1, 0, 0, 0);

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
