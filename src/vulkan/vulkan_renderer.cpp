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

#include "vulkan_renderer.h"

vulkan_renderer* init_renderer(vulkan_objects init_objects) {
    vulkan_renderer* renderer = new vulkan_renderer();
    renderer->init_objects = init_objects;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = init_objects.swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(renderer->init_objects.device, &renderPassInfo, nullptr, &renderer->render_pass) != VK_SUCCESS) {
        delete renderer;
        return NULL;
    }

    renderer->framebuffers.resize(renderer->init_objects.swapchainImageViews.size());
    for (int i = 0; i < renderer->init_objects.swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            renderer->init_objects.swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer->render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = renderer->init_objects.swapchainExtent.width;
        framebufferInfo.height = renderer->init_objects.swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(renderer->init_objects.device, &framebufferInfo, nullptr, &renderer->framebuffers[i]) != VK_SUCCESS) {
            //TODO: Delete the already existing framebuffers
            vkDestroyRenderPass(renderer->init_objects.device, renderer->render_pass, nullptr);
            delete renderer;
            return NULL;
        }
    }

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = renderer->init_objects.indices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(renderer->init_objects.device, &poolInfo, nullptr, &renderer->command_pool) != VK_SUCCESS) {
        for (int i = 0; i < renderer->framebuffers.size(); i++) {
            vkDestroyFramebuffer(renderer->init_objects.device, renderer->framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(renderer->init_objects.device, renderer->render_pass, nullptr);

        delete renderer;
        return NULL;
    }

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer->command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(renderer->init_objects.device, &allocInfo, &renderer->command_buffer) != VK_SUCCESS) {
        vkDestroyCommandPool(renderer->init_objects.device, renderer->command_pool, nullptr);

        for (int i = 0; i < renderer->framebuffers.size(); i++) {
            vkDestroyFramebuffer(renderer->init_objects.device, renderer->framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(renderer->init_objects.device, renderer->render_pass, nullptr);

        delete renderer;
        return NULL;
    }

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    if (vkCreateSemaphore(renderer->init_objects.device, &semaphoreInfo, nullptr, &renderer->imageAvailableSemaphore ) != VK_SUCCESS
        || vkCreateSemaphore(renderer->init_objects.device, &semaphoreInfo, nullptr, &renderer->renderFinishedSemaphore) != VK_SUCCESS) {

        vkFreeCommandBuffers(renderer->init_objects.device, renderer->command_pool, 1, &renderer->command_buffer);
        vkDestroyCommandPool(renderer->init_objects.device, renderer->command_pool, nullptr);

        for (int i = 0; i < renderer->framebuffers.size(); i++) {
            vkDestroyFramebuffer(renderer->init_objects.device, renderer->framebuffers[i], nullptr);
        }

        vkDestroyRenderPass(renderer->init_objects.device, renderer->render_pass, nullptr);

        delete renderer;

        return NULL;
    }

    return renderer;
}

void deinit_renderer(vulkan_renderer* renderer) {
    vkQueueWaitIdle(renderer->init_objects.presentQueue);
    vkDestroySemaphore(renderer->init_objects.device, renderer->imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(renderer->init_objects.device, renderer->renderFinishedSemaphore, nullptr);

    vkFreeCommandBuffers(renderer->init_objects.device, renderer->command_pool, 1, &renderer->command_buffer);
    vkDestroyCommandPool(renderer->init_objects.device, renderer->command_pool, nullptr);

    for (int i = 0; i < renderer->framebuffers.size(); i++) {
        vkDestroyFramebuffer(renderer->init_objects.device, renderer->framebuffers[i], nullptr);
    }

    vkDestroyRenderPass(renderer->init_objects.device, renderer->render_pass, nullptr);

    delete renderer;
}

void renderer_begin_frame(vulkan_renderer* renderer) {
    vkQueueWaitIdle(renderer->init_objects.presentQueue);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(renderer->command_buffer, &beginInfo);

    uint32_t framebufferIndex;
    vkAcquireNextImageKHR(renderer->init_objects.device, renderer->init_objects.swapchain, UINT64_MAX, renderer->imageAvailableSemaphore, VK_NULL_HANDLE, &framebufferIndex);
    renderer->currentSwapchainImageIndex = framebufferIndex;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer->render_pass;
    renderPassInfo.framebuffer = renderer->framebuffers[framebufferIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderer->init_objects.swapchainExtent;

    VkClearValue clearColor = {0.2f, 0.2f, 0.2f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(renderer->command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void renderer_end_frame(vulkan_renderer* renderer) {
    vkCmdEndRenderPass(renderer->command_buffer);
    vkEndCommandBuffer(renderer->command_buffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {renderer->imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer->command_buffer;

    VkSemaphore signalSemaphores[] = {renderer->renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(renderer->init_objects.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {renderer->init_objects.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &renderer->currentSwapchainImageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(renderer->init_objects.presentQueue, &presentInfo);
}