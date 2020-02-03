//
// Created by kaizi99 on 1/25/20.
//

#include "bsp_loader.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
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
    unsigned char	side;			// faces opposite to the node's plane direction
    unsigned char	onNode;			// 1 of on node, 0 if in leaf
    int		        firstedge;		// index into surfedges
    short		    numedges;		// number of surfedges
    short		    texinfo;		// texture info
    short		    dispinfo;		// displacement info
    short		    surfaceFogVolumeID;	// ?
    unsigned char   styles[4];		// switchable lighting info
    int		        lightofs;		// offset into lightmap lump
    float		    area;			// face area in units^2
    int		        LightmapTextureMinsInLuxels[2];	// texture lighting info
    int		        LightmapTextureSizeInLuxels[2];	// texture lighting info
    int		        origFace;		// original face this was split from
    unsigned short	numPrims;		// primitives
    unsigned short	firstPrimID;
    unsigned int	smoothingGroups;	// lightmap smoothing group
};

struct dtexdata_t
{
    glm::vec3	reflectivity;		// RGB reflectivity
    int	        nameStringTableID;	// index into TexdataStringTable
    int	        width, height;		// source image
    int	        view_width, view_height;
};

struct dnode_t
{
    int		planenum;	// index into plane array
    int		children[2];	// negative numbers are -(leafs + 1), not nodes
    short		mins[3];	// for frustum culling
    short		maxs[3];
    unsigned short	firstface;	// index into face array
    unsigned short	numfaces;	// counting both sides
    short		area;		// If all leaves below this node are in the same area, then
    // this is the area index. If not, this is -1.
    short		paddding;	// pad to 32 bytes length
};

struct dleaf_t
{
    int			contents;		// OR of all brushes (not needed?)
    short			cluster;		// cluster this leaf is in
    short			area:9;			// area this leaf is in
    short			flags:7;		// flags
    short			mins[3];		// for frustum culling
    short			maxs[3];
    unsigned short		firstleafface;		// index into leaffaces
    unsigned short		numleaffaces;
    unsigned short		firstleafbrush;		// index into leafbrushes
    unsigned short		numleafbrushes;
    short			leafWaterDataID;	// -1 for not in water

    //!!! NOTE: for maps of version 19 or lower uncomment this block
    /*
    CompressedLightCube	ambientLighting;	// Precaculated light info for entities.
    short			padding;		// padding to 4-byte boundary
    */
};

struct dmodel_t
{
    glm::vec3	mins, maxs;		// bounding box
    glm::vec3	origin;			// for sounds or lights
    int	        headnode;		// index into node array
    int	        firstface, numfaces;	// index into face array
};

#pragma pack(pop)

void convertTree(bspTree* node, plane* splittingPlanes, dnode_t* nodes, dleaf_t* leafs, int leftChild, int rightChild) {
    if (leftChild == abs(leftChild)) {
        // Left child is node
        node->childs[0] = new bspTree();
        node->childs[0]->type = NODE;
        node->childs[0]->nSplittingPlane = splittingPlanes[nodes[leftChild].planenum];

        convertTree(node->childs[0], splittingPlanes, nodes, leafs, nodes[leftChild].children[0], nodes[leftChild].children[1]);
    } else {
        // Left child is leaf
        node->childs[0] = new bspTree();
        node->childs[0]->type = LEAF;
        node->childs[0]->lClusterNumber = leafs[abs(leftChild) - 1].cluster;
    }

    if (rightChild == abs(rightChild)) {
        // Right child is node
        node->childs[1] = new bspTree();
        node->childs[1]->type = NODE;
        node->childs[1]->nSplittingPlane = splittingPlanes[nodes[rightChild].planenum];

        convertTree(node->childs[1], splittingPlanes, nodes, leafs, nodes[rightChild].children[0], nodes[rightChild].children[1]);
    } else {
        // Right child is leaf
        node->childs[1] = new bspTree();
        node->childs[1]->type = LEAF;
        node->childs[1]->lClusterNumber = leafs[abs(rightChild) - 1].cluster;
    }
}

void* read_lump(dheader_t* bspHeader, std::ifstream& stream, int lumpNumber, size_t objectSize, size_t* outObjectCount) {
    lump_t lump = bspHeader->lumps[lumpNumber];

    void* alloc = malloc(lump.filelength);

    stream.seekg(lump.fileoffset, std::ios::beg);
    stream.read((char*)alloc, lump.filelength);

    if (outObjectCount != nullptr)
        *outObjectCount = lump.filelength / objectSize;

    return alloc;
}

bsp_parsed* load_bsp(const std::string& file) {
    std::ifstream fs(file, std::ios::binary);

    if (!fs.is_open()) {
        std::cout << "Could not open " << file << std::endl;
        return nullptr;
    }

    dheader_t bspheader = {};
    fs.read((char*)&bspheader, sizeof(bspheader));

    if (bspheader.ident != IDBSPHEADER) {
        std::cout << file << " is not a valid CS:GO map!" << std::endl;
        return nullptr;
    }

    // Read vertices
    size_t vertexCount;
    vertex* vertices = (vertex*)read_lump(&bspheader, fs, 3, sizeof(vertex), &vertexCount);

    // Read edges
    size_t edgeCount;
    edge* edges = (edge*)read_lump(&bspheader, fs, 12, sizeof(edge), &edgeCount);

    // Read surfedges
    size_t surfedgeCount;
    int* surfedges = (int*)read_lump(&bspheader, fs, 13, sizeof(int), &surfedgeCount);

    // Read Faces
    size_t facesCount;
    dface_t* lfaces = (dface_t*)read_lump(&bspheader, fs, 7, sizeof(dface_t), &facesCount);

    face* faces = new face[facesCount];

    for (int i = 0; i < facesCount; i++) {
        faces[i].edgeCount = lfaces[i].numedges;
        faces[i].firstSurfedgeIndex = lfaces[i].firstedge;
    }

    free(lfaces);

    // Read texinfo
    size_t texdataCount;
    dtexdata_t* ltexdata = (dtexdata_t*)read_lump(&bspheader, fs, 2, sizeof(dtexdata_t), &texdataCount);
    int* texdataStringTable = (int*)read_lump(&bspheader, fs, 44, 0, nullptr);
    char* texdataStringData = (char*)read_lump(&bspheader, fs, 43, 0, nullptr);

    textureInfo* texInfo = new textureInfo[texdataCount];

    for (int i = 0; i < texdataCount; i++) {
        texInfo[i].width = ltexdata[i].width;
        texInfo[i].height = ltexdata[i].height;
        texInfo[i].viewWidth = ltexdata[i].view_width;
        texInfo[i].viewHeight = ltexdata[i].view_height;
        texInfo[i].reflectivity = ltexdata[i].reflectivity;
        
        texInfo[i].textureName = std::string((char*)(texdataStringData + texdataStringTable[ltexdata[i].nameStringTableID]));
    }

    delete[] ltexdata;
    delete[] texdataStringTable;
    delete[] texdataStringData;

    // Read BSP Trees

    // Maps cluster number from the leaves into cluster objects
    // Normally, only one leaf is assigned to one cluster but it seems that with csgo maps, especially with skyboxes,
    // multiple leafs can be assigned to one cluster (from valve bsp documentation)
    std::unordered_map<int, cluster> clusterMapping;

    size_t modelCount;
    dnode_t* nodes = (dnode_t*)read_lump(&bspheader, fs, 5, 0, nullptr);
    dleaf_t* leafs = (dleaf_t*)read_lump(&bspheader, fs, 10, 0, nullptr);
    dmodel_t* models = (dmodel_t*)read_lump(&bspheader, fs, 14, sizeof(dmodel_t), &modelCount);
    plane* splittingPlanes = (plane*)read_lump(&bspheader, fs, 1, 0, nullptr);

    bspTree* trees = new bspTree[modelCount];

    for (int i = 0; i < modelCount; i++) {
        dnode_t* headNode = nodes + models[i].headnode;

        convertTree(trees + i, splittingPlanes, nodes, leafs, headNode->children[0], headNode->children[1]);
    }

    free(models);
    free(leafs);
    free(nodes);
    free(splittingPlanes);

    bsp_parsed* returnStruct = new bsp_parsed();
    returnStruct->vertices = vertices;
    returnStruct->verticesCount = vertexCount;
    returnStruct->edges = edges;
    returnStruct->edgeCount = edgeCount;
    returnStruct->surfedges = surfedges;
    returnStruct->surfedgeCount = surfedgeCount;
    returnStruct->faces = faces;
    returnStruct->faceCount = facesCount;
    returnStruct->textures = texInfo;
    returnStruct->textureCount = texdataCount;
    returnStruct->bspTrees = trees;
    returnStruct->bspTreeCount = modelCount;

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

    if (vkCreateGraphicsPipelines(renderer->init_objects.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &geometry.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bsp pipeline!");
    }

    vkDestroyShaderModule(renderer->init_objects.device, vertShaderStageInfo.module, nullptr);
    vkDestroyShaderModule(renderer->init_objects.device, fragShaderStageInfo.module, nullptr);

    return geometry;
}