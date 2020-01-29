//
// Created by kaizi99 on 1/25/20.
//

#include "bsp_loader.h"

#include <fstream>
#include <iostream>
#include <glm/mat4x4.hpp>
#include "../vulkan/vulkan_utils.h"

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
#define HEADER_LUMPS 64

#pragma pack(push, 1)
struct lump_t
{
    int fileoffset;
    int filelength;
    int version;
    char fourCC[4];
};

struct dheader_t
{
    int ident;
    int version;
    lump_t lumps[HEADER_LUMPS];
    int mapRevision;
};

struct dface_t
{
    unsigned short	planenum;		// the plane number
    unsigned char		side;			// faces opposite to the node's plane direction
    unsigned char		onNode;			// 1 of on node, 0 if in leaf
    int		firstedge;		// index into surfedges
    short		numedges;		// number of surfedges
    short		texinfo;		// texture info
    short		dispinfo;		// displacement info
    short		surfaceFogVolumeID;	// ?
    unsigned char		styles[4];		// switchable lighting info
    int		lightofs;		// offset into lightmap lump
    float		area;			// face area in units^2
    int		LightmapTextureMinsInLuxels[2];	// texture lighting info
    int		LightmapTextureSizeInLuxels[2];	// texture lighting info
    int		origFace;		// original face this was split from
    unsigned short	numPrims;		// primitives
    unsigned short	firstPrimID;
    unsigned int	smoothingGroups;	// lightmap smoothing group
};
#pragma pack(pop)

bsp_parsed* load_bsp(const std::string& file) {
    std::ifstream fs(file, std::ios::binary);

    if (!fs.is_open()) {
        std::cout << "Could not open " << file << std::endl;
        return nullptr;
    }

    dheader_t bspheader;
    fs.read((char*)&bspheader, sizeof(bspheader));

    if (bspheader.ident != IDBSPHEADER) {
        std::cout << file << " is not a valid CS:GO map!" << std::endl;
        return nullptr;
    }

    // Read vertices
    lump_t vertexLump = bspheader.lumps[3];

    vertex* vertices = new vertex[vertexLump.filelength / 12];

    fs.seekg(0, std::ios::beg);
    fs.seekg(vertexLump.fileoffset);
    fs.read((char*)vertices, vertexLump.filelength);

    // Read edges
    lump_t edgesLump = bspheader.lumps[12];

    edge* edges = new edge[edgesLump.filelength / sizeof(edge)];

    fs.seekg(edgesLump.fileoffset, std::ios::beg);
    fs.read((char*)edges, edgesLump.filelength);

    // Read surfedges
    lump_t surfedgeLump = bspheader.lumps[13];

    int* surfedges = new int[surfedgeLump.filelength / sizeof(int)];

    fs.seekg(surfedgeLump.fileoffset, std::ios::beg);
    fs.read((char*)surfedges, surfedgeLump.filelength);

    // Read Faces
    lump_t facesLump = bspheader.lumps[7];

    dface_t* lfaces = new dface_t[facesLump.filelength / sizeof(dface_t)];
    face* faces = new face[facesLump.filelength / sizeof(dface_t)];

    fs.seekg(facesLump.fileoffset, std::ios::beg);
    fs.read((char*)lfaces, facesLump.filelength);

    for (int i = 0; i < facesLump.filelength / sizeof(dface_t); i++) {
        faces[i].edgeCount = lfaces[i].numedges;
        faces[i].firstSurfedgeIndex = lfaces[i].firstedge;
    }

    delete[] lfaces;

    bsp_parsed* returnStruct = new bsp_parsed();
    returnStruct->vertices = vertices;
    returnStruct->verticesCount = vertexLump.filelength / sizeof(vertex);
    returnStruct->edges = edges;
    returnStruct->edgeCount = edgesLump.filelength / sizeof(edge);
    returnStruct->surfedges = surfedges;
    returnStruct->surfedgeCount = surfedgeLump.filelength / sizeof(int);
    returnStruct->faces = faces;
    returnStruct->faceCount = facesLump.filelength / sizeof(dface_t);

    return returnStruct;
}

bsp_geometry_vulkan create_geometry_from_bsp(vulkan_renderer* renderer, bsp_parsed* bsp) {
    bsp_geometry_vulkan geometry = {};

    vulkan_createBuffer(renderer, bsp->verticesCount * sizeof(vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, geometry.vertexBuffer, geometry.vertexBufferMemory);
    
    void* bufferAddress;
    vkMapMemory(renderer->init_objects.device, geometry.vertexBufferMemory, 0, bsp->verticesCount * sizeof(vertex), 0, &bufferAddress);
    memcpy(bufferAddress, bsp->vertices, bsp->verticesCount * sizeof(vertex));
    vkUnmapMemory(renderer->init_objects.device, geometry.vertexBufferMemory);

    std::vector<short> indices;
    for (int i = 0; i < bsp->faceCount; i++) {
        short fanVertex = bsp->edges[abs(bsp->surfedges[bsp->faces[i].firstSurfedgeIndex])].v[0];

        for (int j = 1; j < bsp->faces[i].edgeCount - 1; j++) {
            int surfedge = bsp->surfedges[bsp->faces[i].firstSurfedgeIndex + j];

            int abssurfedge = abs(surfedge);
            edge e = bsp->edges[abssurfedge];
            indices.push_back(fanVertex);
            if (surfedge == abssurfedge) {
                indices.push_back(e.v[0]);
                indices.push_back(e.v[1]);
            }
            else {
                indices.push_back(e.v[1]);
                indices.push_back(e.v[0]);
            }
        }
    }

    geometry.indicesCount = indices.size();
    vulkan_createBuffer(renderer, geometry.indicesCount * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, geometry.indexBuffer, geometry.indexBufferMemory);

    vkMapMemory(renderer->init_objects.device, geometry.indexBufferMemory, 0, geometry.indicesCount * sizeof(short), 0, &bufferAddress);
    memcpy(bufferAddress, indices.data(), geometry.indicesCount * sizeof(short));
    vkUnmapMemory(renderer->init_objects.device, geometry.indexBufferMemory);

    // Create Pipeline for Rendering Operations
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vulkan_createShaderModule(renderer, "bsp_vert.spv");
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = vulkan_createShaderModule(renderer, "bsp_frag.spv");
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = 0;

    VkVertexInputAttributeDescription inputAttributes[] = {
        positionAttribute
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = inputAttributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->init_objects.swapchainExtent.width;
    viewport.height = (float)renderer->init_objects.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = renderer->init_objects.swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 0;
    dynamicState.pDynamicStates = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 0;
    layoutInfo.pBindings = nullptr;

    if (vkCreateDescriptorSetLayout(renderer->init_objects.device, &layoutInfo, nullptr, &geometry.descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &geometry.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderer->init_objects.device, &pipelineLayoutInfo, nullptr, &geometry.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = geometry.pipelineLayout;
    pipelineInfo.renderPass = renderer->render_pass;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(renderer->init_objects.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometry.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bsp pipeline!");
    }

    vkDestroyShaderModule(renderer->init_objects.device, vertShaderStageInfo.module, nullptr);
    vkDestroyShaderModule(renderer->init_objects.device, fragShaderStageInfo.module, nullptr);

    return geometry;
}