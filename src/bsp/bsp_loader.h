//
// Created by kaizi99 on 1/25/20.
//

#ifndef VULKAN_TEST_BSP_LOADER_H
#define VULKAN_TEST_BSP_LOADER_H

#include <string>
#include <cstdint>
#include <glm/glm.hpp>
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
    cluster* lCluster;
    short lClusterNumber;
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
    cluster* clusters;
    size_t clusterCount;
    bspTree* bspTrees;
    size_t bspTreeCount;
};

bsp_parsed* load_bsp(const std::string& file);

struct bsp_geometry_vulkan {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    int indicesCount;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};

bsp_geometry_vulkan create_geometry_from_bsp(vulkan_renderer* renderer, bsp_parsed* bsp);

#endif //VULKAN_TEST_BSP_LOADER_H
