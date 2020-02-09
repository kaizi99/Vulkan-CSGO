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

#include "bsp_loader.h"

struct bsp_face_rendering_data {
    int indexBufferOffset;
    int indicesCount;
};

struct bsp_cluster_rendering_data {
    std::vector<bsp_face_rendering_data> faces;
};

struct bsp_rendering_data {
    std::vector<bspTree> bspTrees;
    std::vector<bsp_cluster_rendering_data> chunkRenderingData;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};

bsp_rendering_data bsp_rendering_prepare(bsp_parsed* bsp);