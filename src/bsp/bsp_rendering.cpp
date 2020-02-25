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

#include "bsp_rendering.h"
#include "../vulkan/vulkan_utils.h"
#include <stdexcept>

bsp_rendering_data bsp_rendering_prepare(bsp_parsed* bsp, vulkan_renderer* renderer) {
    bsp_rendering_data renderingData;

    // Create Vertex Buffer
    vulkan_createBuffer(renderer, bsp->verticesCount * sizeof(vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, renderingData.vertexBuffer, renderingData.vertexBufferMemory);

    void* bufferAddress;
    vkMapMemory(renderer->init_objects.device, renderingData.vertexBufferMemory, 0, bsp->verticesCount * sizeof(vertex), 0, &bufferAddress);
    memcpy(bufferAddress, bsp->vertices, bsp->verticesCount * sizeof(vertex));
    vkUnmapMemory(renderer->init_objects.device, renderingData.vertexBufferMemory);

    renderingData.bspTrees.resize(bsp->bspTreeCount);
    for (int i = 0; i < bsp->bspTreeCount; i++) {
        renderingData.bspTrees[i] = bsp->bspTrees[i];
    }

    int indicesCount = 0;

    /*
    for (auto& clusterPair : bsp->clusters) {
        cluster* c = clusterPair.second;

        for (face* f : c->faces) {
            indicesCount += f->edgeCount;
            if (f->edgeCount > 3) {
                indicesCount += indicesCount - 3;
            }
        }
    }

    short* indexBuffer = (short*) malloc(sizeof(short) * indicesCount);
    int currentIndex = 0;

    for (auto& clusterPair : bsp->clusters) {
        cluster* c = clusterPair.second;

        bsp_cluster_rendering_data clusterRenderingData;
        
        for (face* f : c->faces) {
            bsp_face_rendering_data face = {};

            short fanVertex = bsp->edges[abs(bsp->surfedges[f->firstSurfedgeIndex])].v[0];

            for (int j = 1; j < f->edgeCount - 1; j++) {
                int surfedge = bsp->surfedges[f->firstSurfedgeIndex + j];

                int abssurfedge = abs(surfedge);
                edge e = bsp->edges[abssurfedge];
                
                face.indexBufferOffset = currentIndex;
                indexBuffer[currentIndex++] = fanVertex;

                if (surfedge == abssurfedge) {
                    indexBuffer[currentIndex++] = e.v[0];
                    indexBuffer[currentIndex++] = e.v[1];
                }
                else {
                    indexBuffer[currentIndex++] = e.v[1];
                    indexBuffer[currentIndex++] = e.v[0];
                }
            }

            clusterRenderingData.faces.push_back(face);
        }
        
        renderingData.clusterRenderingData[clusterPair.first] = clusterRenderingData;
    }

    vulkan_createBuffer(renderer, indicesCount * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, renderingData.indexBuffer, renderingData.indexBufferMemory);

    vkMapMemory(renderer->init_objects.device, renderingData.indexBufferMemory, 0, indicesCount * sizeof(short), 0, &bufferAddress);
    memcpy(bufferAddress, indexBuffer, indicesCount * sizeof(short));
    vkUnmapMemory(renderer->init_objects.device, renderingData.indexBufferMemory);
    */

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

    if (vkCreateDescriptorSetLayout(renderer->init_objects.device, &layoutInfo, nullptr, &renderingData.descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &renderingData.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderer->init_objects.device, &pipelineLayoutInfo, nullptr, &renderingData.pipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = renderingData.pipelineLayout;
    pipelineInfo.renderPass = renderer->render_pass;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(renderer->init_objects.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &renderingData.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create bsp pipeline!");
    }

    vkDestroyShaderModule(renderer->init_objects.device, vertShaderStageInfo.module, nullptr);
    vkDestroyShaderModule(renderer->init_objects.device, fragShaderStageInfo.module, nullptr);

    return renderingData;
}

void bsp_render(bsp_rendering_data* renderingData, vulkan_renderer* renderer, camera* c) {
    vkCmdBindPipeline(renderer->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderingData->pipeline);

    VkBuffer vertexBuffers[] = { renderingData->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(renderer->command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(renderer->command_buffer, renderingData->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    glm::mat4 mvp = calculateViewProjection(*c);
    vkCmdPushConstants(renderer->command_buffer, renderingData->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

    //bsp_cluster_rendering_data* cluster = renderingData->clusterRenderingData.data();
    for (auto& clusterPair : renderingData->clusterRenderingData) {
        bsp_cluster_rendering_data* cluster = &clusterPair.second;
        for (const bsp_face_rendering_data& face : cluster->faces) {
            vkCmdDrawIndexed(renderer->command_buffer, face.indicesCount, 1, face.indexBufferOffset, 0, 0);
        }
    }

}