#pragma once

#include "vulkan_init.h"

struct vulkan_renderer {
    vulkan_objects init_objects;
    VkRenderPass render_pass;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    uint32_t currentSwapchainImageIndex;
};

vulkan_renderer* init_renderer(vulkan_objects init_objects);
void deinit_renderer(vulkan_renderer* renderer);

void renderer_begin_frame(vulkan_renderer* renderer);
void renderer_end_frame(vulkan_renderer* renderer);