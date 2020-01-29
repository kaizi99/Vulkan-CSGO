//
// Created by kaizi99 on 1/25/20.
//

#ifndef VULKAN_TEST_BSP_LOADER_H
#define VULKAN_TEST_BSP_LOADER_H

#include <string>
#include <cstdint>
#include "../vulkan/vulkan_renderer.h"

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

struct bsp_parsed {
    vertex* vertices;
    size_t verticesCount;
    edge* edges;
    size_t edgeCount;
    int* surfedges;
    size_t surfedgeCount;
    face* faces;
    size_t faceCount;
};

bsp_parsed* load_bsp(const std::string& file);

struct bsp_geometry_vulkan {
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
};

bsp_geometry_vulkan create_geometry_from_bsp(vulkan_renderer* renderer, bsp_parsed* bsp);

#endif //VULKAN_TEST_BSP_LOADER_H
