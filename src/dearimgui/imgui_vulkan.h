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
    GLFWwindow* window;
};

bool imguivk_init(vulkan_renderer* renderer, imguivk* imgui, GLFWwindow* window);

void imguivk_beginFrame(vulkan_renderer* renderer, imguivk* imgui);
void imguivk_endFrame(vulkan_renderer* renderer, imguivk* imgui);

void imguivk_deinit(vulkan_renderer* renderer, imguivk* imgui);