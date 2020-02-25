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

#ifndef VULKAN_TEST_BSP_LOADER_H
#define VULKAN_TEST_BSP_LOADER_H

#include <string>
#include <cstdint>
#include <glm/glm.hpp>
#include "../vulkan/vulkan_renderer.h"
#include <unordered_map>

struct vertex {
    float x;
    float y;
    float z;
};

struct edge {
    // Vertex indices of the edge
    unsigned short v[2];
};

struct face {
    int firstSurfedgeIndex;
    int edgeCount;
};

struct textureInfo {
    glm::ivec3 reflectivity;
    int width;
    int height;
    int viewWidth;
    int viewHeight;
    std::string textureName;
};

struct plane {
    glm::vec3 normal;
    float distance;
    int type;
};

struct cluster {
    int clusterID;
    std::vector<face*> faces;
};

enum bspNodeType {
    NODE,
    LEAF
};

struct bspTree {
    bspTree* childs[2];
    bspNodeType type;
    plane nSplittingPlane;
    std::vector<face*> faces;
};

struct bsp_parsed {
    vertex* vertices;
    size_t verticesCount;
    edge* edges;
    size_t edgeCount;
    int* surfedges;
    size_t surfedgeCount;
    face* faces;
    size_t faceCount;
    textureInfo* textures;
    size_t textureCount;
    bspTree* bspTrees;
    size_t bspTreeCount;
};

bsp_parsed* load_bsp(const std::string& file);

/*
struct bsp_geometry_vulkan {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    int maxIndicesCount;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};

bsp_geometry_vulkan create_geometry_from_bsp(vulkan_renderer* renderer, bsp_parsed* bsp);
*/

#endif //VULKAN_TEST_BSP_LOADER_H
