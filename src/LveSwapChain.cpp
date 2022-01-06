#include "LveSwapChain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace lve {

    LveSwapChain::LveSwapChain(LveDevice &deviceRef, VkExtent2D extent)
		    : m_device_{deviceRef}, m_windowExtent_{extent} {
	    init();
    }

	LveSwapChain::LveSwapChain(
			LveDevice &deviceRef, VkExtent2D extent, std::shared_ptr<LveSwapChain> previous)
			: m_device_{deviceRef}, m_windowExtent_{extent}, m_oldSwapChain_{previous} {
		init();
		m_oldSwapChain_ = nullptr;
	}

    void LveSwapChain::init() {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    LveSwapChain::~LveSwapChain() {
	    for (auto imageView: m_swapChainImageViews_) {
		    vkDestroyImageView(m_device_.device(), imageView, nullptr);
	    }
	    m_swapChainImageViews_.clear();

	    if (m_swapChain_ != nullptr) {
		    vkDestroySwapchainKHR(m_device_.device(), m_swapChain_, nullptr);
		    m_swapChain_ = nullptr;
	    }

	    for (int i = 0; i < m_depthImages_.size(); i++) {
		    vkDestroyImageView(m_device_.device(), m_depthImageViews_[i], nullptr);
		    vkDestroyImage(m_device_.device(), m_depthImages_[i], nullptr);
		    vkFreeMemory(m_device_.device(), m_depthImageMemorys_[i], nullptr);
	    }

	    for (auto framebuffer: m_swapChainFramebuffers_) {
		    vkDestroyFramebuffer(m_device_.device(), framebuffer, nullptr);
	    }

	    vkDestroyRenderPass(m_device_.device(), m_renderPass_, nullptr);

	    // cleanup synchronization objects
	    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
		    vkDestroySemaphore(m_device_.device(), m_renderFinishedSemaphores_[i], nullptr);
		    vkDestroySemaphore(m_device_.device(), m_imageAvailableSemaphores_[i], nullptr);
		    vkDestroyFence(m_device_.device(), m_inFlightFences_[i], nullptr);
	    }
    }

    VkResult LveSwapChain::acquireNextImage(uint32_t *imageIndex) {
	    vkWaitForFences(
			    m_device_.device(),
			    1,
			    &m_inFlightFences_[m_currentFrame_],
			    VK_TRUE,
			    std::numeric_limits<uint64_t>::max());

	    VkResult result = vkAcquireNextImageKHR(
			    m_device_.device(),
			    m_swapChain_,
			    std::numeric_limits<uint64_t>::max(),
			    m_imageAvailableSemaphores_[m_currentFrame_],  // must be a not signaled semaphore
			    VK_NULL_HANDLE,
			    imageIndex);

        return result;
    }

    VkResult LveSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
	    if (m_imagesInFlight_[*imageIndex] != VK_NULL_HANDLE) {
		    vkWaitForFences(m_device_.device(), 1, &m_imagesInFlight_[*imageIndex], VK_TRUE, UINT64_MAX);
	    }
	    m_imagesInFlight_[*imageIndex] = m_inFlightFences_[m_currentFrame_];

	    VkSubmitInfo submitInfo = {};
	    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores_[m_currentFrame_]};
	    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	    submitInfo.waitSemaphoreCount = 1;
	    submitInfo.pWaitSemaphores = waitSemaphores;
	    submitInfo.pWaitDstStageMask = waitStages;

	    submitInfo.commandBufferCount = 1;
	    submitInfo.pCommandBuffers = buffers;

	    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores_[m_currentFrame_]};
	    submitInfo.signalSemaphoreCount = 1;
	    submitInfo.pSignalSemaphores = signalSemaphores;

	    vkResetFences(m_device_.device(), 1, &m_inFlightFences_[m_currentFrame_]);
	    if (vkQueueSubmit(m_device_.graphicsQueue(), 1, &submitInfo, m_inFlightFences_[m_currentFrame_]) !=
	        VK_SUCCESS) {
		    throw std::runtime_error("failed to submit draw command buffer!");
	    }

	    VkPresentInfoKHR presentInfo = {};
	    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	    presentInfo.waitSemaphoreCount = 1;
	    presentInfo.pWaitSemaphores = signalSemaphores;

	    VkSwapchainKHR swapChains[] = {m_swapChain_};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

	    auto result = vkQueuePresentKHR(m_device_.presentQueue(), &presentInfo);

	    m_currentFrame_ = (m_currentFrame_ + 1) % m_maxFramesInFlight;

        return result;
    }

    void LveSwapChain::createSwapChain() {
	    SwapChainSupportDetails swapChainSupport = m_device_.getSwapChainSupport();

	    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.m_formats);
	    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.m_presentModes);
	    VkExtent2D extent = chooseSwapExtent(swapChainSupport.m_capabilities);

	    uint32_t imageCount = swapChainSupport.m_capabilities.minImageCount + 1;
	    if (swapChainSupport.m_capabilities.maxImageCount > 0 &&
	        imageCount > swapChainSupport.m_capabilities.maxImageCount) {
		    imageCount = swapChainSupport.m_capabilities.maxImageCount;
	    }

	    VkSwapchainCreateInfoKHR createInfo = {};
	    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	    createInfo.surface = m_device_.surface();

	    createInfo.minImageCount = imageCount;
	    createInfo.imageFormat = surfaceFormat.format;
	    createInfo.imageColorSpace = surfaceFormat.colorSpace;
	    createInfo.imageExtent = extent;
	    createInfo.imageArrayLayers = 1;
	    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	    QueueFamilyIndices indices = m_device_.findPhysicalQueueFamilies();
	    uint32_t queueFamilyIndices[] = {indices.m_graphicsFamily, indices.m_presentFamily};

	    if (indices.m_graphicsFamily != indices.m_presentFamily) {
		    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		    createInfo.queueFamilyIndexCount = 2;
		    createInfo.pQueueFamilyIndices = queueFamilyIndices;
	    } else {
		    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		    createInfo.queueFamilyIndexCount = 0;      // Optional
		    createInfo.pQueueFamilyIndices = nullptr;  // Optional
	    }

	    createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform;
	    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

	    createInfo.oldSwapchain = m_oldSwapChain_ == nullptr ? VK_NULL_HANDLE : m_oldSwapChain_->m_swapChain_;

	    if (vkCreateSwapchainKHR(m_device_.device(), &createInfo, nullptr, &m_swapChain_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create swap chain!");
	    }

	    // we only specified a minimum number of images in the swap chain, so the implementation is
	    // allowed to create a swap chain with more. That's why we'll first query the final number of
	    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	    // retrieve the handles.
	    vkGetSwapchainImagesKHR(m_device_.device(), m_swapChain_, &imageCount, nullptr);
	    m_swapChainImages_.resize(imageCount);
	    vkGetSwapchainImagesKHR(m_device_.device(), m_swapChain_, &imageCount, m_swapChainImages_.data());

	    m_swapChainImageFormat_ = surfaceFormat.format;
	    m_swapChainExtent_ = extent;
    }

    void LveSwapChain::createImageViews() {
	    m_swapChainImageViews_.resize(m_swapChainImages_.size());
	    for (size_t i = 0; i < m_swapChainImages_.size(); i++) {
		    VkImageViewCreateInfo viewInfo{};
		    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		    viewInfo.image = m_swapChainImages_[i];
		    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		    viewInfo.format = m_swapChainImageFormat_;
		    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		    viewInfo.subresourceRange.baseMipLevel = 0;
		    viewInfo.subresourceRange.levelCount = 1;
		    viewInfo.subresourceRange.baseArrayLayer = 0;
		    viewInfo.subresourceRange.layerCount = 1;

		    if (vkCreateImageView(m_device_.device(), &viewInfo, nullptr, &m_swapChainImageViews_[i]) !=
		        VK_SUCCESS) {
			    throw std::runtime_error("failed to create texture image view!");
		    }
        }
    }

    void LveSwapChain::createRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
	    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    VkSubpassDescription subpass = {};
	    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	    subpass.colorAttachmentCount = 1;
	    subpass.pColorAttachments = &colorAttachmentRef;
	    subpass.pDepthStencilAttachment = &depthAttachmentRef;

	    VkSubpassDependency dependency = {};
	    dependency.dstSubpass = 0;
	    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	    dependency.dstStageMask =
			    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	    dependency.srcAccessMask = 0;
	    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	    VkRenderPassCreateInfo renderPassInfo = {};
	    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	    renderPassInfo.pAttachments = attachments.data();
	    renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

	    if (vkCreateRenderPass(m_device_.device(), &renderPassInfo, nullptr, &m_renderPass_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create render pass!");
	    }
    }

    void LveSwapChain::createFramebuffers() {
	    m_swapChainFramebuffers_.resize(imageCount());
        for (size_t i = 0; i < imageCount(); i++) {
	        std::array<VkImageView, 2> attachments = {m_swapChainImageViews_[i], m_depthImageViews_[i]};

            VkExtent2D swapChainExtent = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
	        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	        framebufferInfo.renderPass = m_renderPass_;
	        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

	        if (vkCreateFramebuffer(
			        m_device_.device(),
			        &framebufferInfo,
			        nullptr,
			        &m_swapChainFramebuffers_[i]) != VK_SUCCESS) {
		        throw std::runtime_error("failed to create framebuffer!");
	        }
        }
    }

    void LveSwapChain::createDepthResources() {
	    VkFormat depthFormat = findDepthFormat();
	    m_swapChainDepthFormat_ = depthFormat;
	    VkExtent2D swapChainExtent = getSwapChainExtent();

	    m_depthImages_.resize(imageCount());
	    m_depthImageMemorys_.resize(imageCount());
	    m_depthImageViews_.resize(imageCount());

	    for (int i = 0; i < m_depthImages_.size(); i++) {
		    VkImageCreateInfo imageInfo{};
		    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		    imageInfo.imageType = VK_IMAGE_TYPE_2D;
		    imageInfo.extent.width = swapChainExtent.width;
		    imageInfo.extent.height = swapChainExtent.height;
		    imageInfo.extent.depth = 1;
		    imageInfo.mipLevels = 1;
		    imageInfo.arrayLayers = 1;
		    imageInfo.format = depthFormat;
		    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		    imageInfo.flags = 0;

		    m_device_.createImageWithInfo(
				    imageInfo,
				    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				    m_depthImages_[i],
				    m_depthImageMemorys_[i]);

		    VkImageViewCreateInfo viewInfo{};
		    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		    viewInfo.image = m_depthImages_[i];
		    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		    viewInfo.format = depthFormat;
		    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		    viewInfo.subresourceRange.baseMipLevel = 0;
		    viewInfo.subresourceRange.levelCount = 1;
		    viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

		    if (vkCreateImageView(m_device_.device(), &viewInfo, nullptr, &m_depthImageViews_[i]) != VK_SUCCESS) {
			    throw std::runtime_error("failed to create texture image view!");
		    }
        }
    }

    void LveSwapChain::createSyncObjects() {
	    m_imageAvailableSemaphores_.resize(m_maxFramesInFlight);
	    m_renderFinishedSemaphores_.resize(m_maxFramesInFlight);
	    m_inFlightFences_.resize(m_maxFramesInFlight);
	    m_imagesInFlight_.resize(imageCount(), VK_NULL_HANDLE);

	    VkSemaphoreCreateInfo semaphoreInfo = {};
	    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	    VkFenceCreateInfo fenceInfo = {};
	    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
		    if (vkCreateSemaphore(m_device_.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores_[i]) !=
		        VK_SUCCESS ||
		        vkCreateSemaphore(m_device_.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores_[i]) !=
		        VK_SUCCESS ||
		        vkCreateFence(m_device_.device(), &fenceInfo, nullptr, &m_inFlightFences_[i]) != VK_SUCCESS) {
			    throw std::runtime_error("failed to create synchronization objects for a frame!");
		    }
	    }
    }

    VkSurfaceFormatKHR LveSwapChain::chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR LveSwapChain::chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
        }

        // for (const auto &availablePresentMode : availablePresentModes) {
        //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        //     std::cout << "Present mode: Immediate" << std::endl;
        //     return availablePresentMode;
        //   }
        // }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D LveSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
	        VkExtent2D actualExtent = m_windowExtent_;
            actualExtent.width = std::max(
                    capabilities.minImageExtent.width,
                    std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                    capabilities.minImageExtent.height,
                    std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat LveSwapChain::findDepthFormat() {
	    return m_device_.findSupportedFormat(
			    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			    VK_IMAGE_TILING_OPTIMAL,
			    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

}  // namespace lve