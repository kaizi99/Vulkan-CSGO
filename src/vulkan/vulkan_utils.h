#pragma once

#include "vulkan_renderer.h"
#include <string>

bool vulkan_createBuffer(vulkan_renderer* renderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
bool vulkan_createImage(vulkan_renderer* renderer, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);

VkCommandBuffer vulkan_beginSingleTimeCommandBuffer(vulkan_renderer* renderer);
void vulkan_endSingleTimeCommandBuffer(vulkan_renderer* renderer, VkCommandBuffer commandBuffer);

void vulkan_copyBuffers(vulkan_renderer* renderer, VkBuffer src, VkBuffer dst, VkDeviceSize size);

VkShaderModule vulkan_createShaderModule(vulkan_renderer* renderer, std::string shaderFile);