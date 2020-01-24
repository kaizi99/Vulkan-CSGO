#pragma once

#include "../vulkan/vulkan_renderer.h"
#include "imgui.h"

struct imguivk {
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView imageView;
    VkSampler sampler;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkBuffer> oldBuffers;
    std::vector<VkDeviceMemory> oldDeviceMemory;
};

bool imguivk_init(vulkan_renderer* renderer, imguivk* imgui);

void imguivk_beginFrame(vulkan_renderer* renderer, imguivk* imgui);
void imguivk_endFrame(vulkan_renderer* renderer, imguivk* imgui);