#pragma once

#include "LveDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace lve {

	class LveSwapChain {
	public:
		static constexpr int m_maxFramesInFlight = 2;

		LveSwapChain(LveDevice &deviceRef, VkExtent2D windowExtent);

		LveSwapChain(
				LveDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapChain> previous);

		~LveSwapChain();

		LveSwapChain(const LveSwapChain &) = delete;

		LveSwapChain &operator=(const LveSwapChain &) = delete;

		VkFramebuffer getFrameBuffer(int index) { return m_swapChainFramebuffers_[index]; }

		VkRenderPass getRenderPass() { return m_renderPass_; }

		VkImageView getImageView(int index) { return m_swapChainImageViews_[index]; }

		size_t imageCount() { return m_swapChainImages_.size(); }

		VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat_; }

		VkExtent2D getSwapChainExtent() { return m_swapChainExtent_; }

		uint32_t width() { return m_swapChainExtent_.width; }

		uint32_t height() { return m_swapChainExtent_.height; }

		float extentAspectRatio() {
			return static_cast<float>(m_swapChainExtent_.width) / static_cast<float>(m_swapChainExtent_.height);
		}

		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t *imageIndex);

		VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

		bool compareSwapFormats(const LveSwapChain &swapChain) const {
			return swapChain.m_swapChainDepthFormat_ == m_swapChainDepthFormat_ &&
			       swapChain.m_swapChainImageFormat_ == m_swapChainImageFormat_;
		}

	private:
		void init();

		void createSwapChain();

		void createImageViews();

		void createDepthResources();

		void createRenderPass();

		void createFramebuffers();

		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR> &availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(
				const std::vector<VkPresentModeKHR> &availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

		VkFormat m_swapChainImageFormat_;
		VkFormat m_swapChainDepthFormat_;
		VkExtent2D m_swapChainExtent_;

		std::vector<VkFramebuffer> m_swapChainFramebuffers_;
		VkRenderPass m_renderPass_;

		std::vector<VkImage> m_depthImages_;
		std::vector<VkDeviceMemory> m_depthImageMemorys_;
		std::vector<VkImageView> m_depthImageViews_;
		std::vector<VkImage> m_swapChainImages_;
		std::vector<VkImageView> m_swapChainImageViews_;

		LveDevice &m_device_;
		VkExtent2D m_windowExtent_;

		VkSwapchainKHR m_swapChain_;
		std::shared_ptr<LveSwapChain> m_oldSwapChain_;

		std::vector<VkSemaphore> m_imageAvailableSemaphores_;
		std::vector<VkSemaphore> m_renderFinishedSemaphores_;
		std::vector<VkFence> m_inFlightFences_;
		std::vector<VkFence> m_imagesInFlight_;
		size_t m_currentFrame_ = 0;
	};

}  // namespace lve