//
// Created by wdoppenberg on 11-12-21.
//

#include "LveRenderer.hpp"

namespace lve {

	LveRenderer::LveRenderer(LveWindow &window, LveDevice &mLveDevice) : m_lveWindow_(window),
	                                                                     m_lveDevice_(mLveDevice) {
		recreateSwapChain();
		createCommandBuffers();
	}

	LveRenderer::~LveRenderer() {
		freeCommandBuffers();
	}

	void LveRenderer::createCommandBuffers() {
		m_commandBuffers_.resize(LveSwapChain::m_maxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_lveDevice_.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers_.size());

		if (vkAllocateCommandBuffers(m_lveDevice_.device(), &allocInfo, m_commandBuffers_.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers");
		}
	}


	void LveRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
				m_lveDevice_.device(),
				m_lveDevice_.getCommandPool(),
				static_cast<uint32_t>(m_commandBuffers_.size()),
				m_commandBuffers_.data());
		m_commandBuffers_.clear();
	}

	VkCommandBuffer LveRenderer::beginFrame() {
#ifndef NDEBUG
		assert(!m_isFrameStarted_ && "Can't call beginFrame while already in progress");
#endif
		auto result = m_lveSwapChain_->acquireNextImage(&m_currentImageIndex_);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image");
		}

		m_isFrameStarted_ = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		return commandBuffer;
	}

	void LveRenderer::endFrame() {
#ifndef NDEBUG
		assert(m_isFrameStarted_ && "Can't call endFrame while frame is not in progress");
#endif
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}

		auto result = m_lveSwapChain_->submitCommandBuffers(&commandBuffer, &m_currentImageIndex_);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_lveWindow_.wasWindowResized()) {
			m_lveWindow_.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image");
		}

		m_isFrameStarted_ = false;
		m_currentFrameIndex_ = (m_currentFrameIndex_ + 1) % LveSwapChain::m_maxFramesInFlight;
	}

	void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
#ifndef NDEBUG
		assert(m_isFrameStarted_ && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
				commandBuffer == getCurrentCommandBuffer() &&
				"Can't begin render pass on command buffer from a different frame"
		);
#endif
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_lveSwapChain_->getRenderPass();
		renderPassInfo.framebuffer = m_lveSwapChain_->getFrameBuffer(m_currentImageIndex_);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = m_lveSwapChain_->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_lveSwapChain_->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_lveSwapChain_->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, m_lveSwapChain_->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}

	void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
#ifndef NDEBUG
		assert(m_isFrameStarted_ && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
				commandBuffer == getCurrentCommandBuffer() &&
				"Can't end render pass on command buffer from a different frame"
		);
#endif
		vkCmdEndRenderPass(commandBuffer);
	}

	void LveRenderer::recreateSwapChain() {
		auto extent = m_lveWindow_.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = m_lveWindow_.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_lveDevice_.device());

		if (m_lveSwapChain_ == nullptr) {
			m_lveSwapChain_ = std::make_unique<LveSwapChain>(m_lveDevice_, extent);
		} else {
			std::shared_ptr<LveSwapChain> oldSwapChain = std::move(m_lveSwapChain_);
			m_lveSwapChain_ = std::make_unique<LveSwapChain>(m_lveDevice_, extent, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*m_lveSwapChain_)) {
				throw std::runtime_error("Swap chain image (or depth) format has changed!");
			}

		}
	}
}