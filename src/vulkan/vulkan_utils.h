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

#include "vulkan_renderer.h"
#include <string>

bool vulkan_createBuffer(vulkan_renderer* renderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
bool vulkan_createImage(vulkan_renderer* renderer, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);

VkCommandBuffer vulkan_beginSingleTimeCommandBuffer(vulkan_renderer* renderer);
void vulkan_endSingleTimeCommandBuffer(vulkan_renderer* renderer, VkCommandBuffer commandBuffer);

void vulkan_copyBuffers(vulkan_renderer* renderer, VkBuffer src, VkBuffer dst, VkDeviceSize size);

VkShaderModule vulkan_createShaderModule(vulkan_renderer* renderer, std::string shaderFile);