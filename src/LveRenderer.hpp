//
// Created by wdoppenberg on 11-12-21.
//

#ifndef VULKAN_TEST_LVERENDERER_HPP
#define VULKAN_TEST_LVERENDERER_HPP


#include "LveWindow.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"
#include "LveDevice.hpp"
#include "LveSwapChain.hpp"
#include "LveModel.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace lve {
	class LveRenderer {
	public:

		LveRenderer(LveWindow &lveWindow, LveDevice &lveDevice);

		~LveRenderer();

		LveRenderer(const LveRenderer &) = delete;

		LveRenderer &operator=(const LveRenderer &) = delete;

		VkRenderPass getSwapChainRenderPass() const { return m_lveSwapChain_->getRenderPass(); }

		float getAspectRatio() const { return m_lveSwapChain_->extentAspectRatio(); };

		bool isFrameInProgress() const { return m_isFrameStarted_; }

		VkCommandBuffer getCurrentCommandBuffer() const {
#ifndef NDEBUG
			assert(m_isFrameStarted_ && "Cannot get command buffer when frame not in progress.");
#endif
			return m_commandBuffers_[m_currentFrameIndex_];
		}

		VkCommandBuffer beginFrame();

		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		int getFrameIndex() const {
#ifndef DEBUG
			assert(m_isFrameStarted_ && "Cannot get frame index when frame not in progress.");
#endif
			return m_currentFrameIndex_;
		}

	private:

		void createCommandBuffers();

		void recreateSwapChain();

		void freeCommandBuffers();

		LveWindow &m_lveWindow_;
		LveDevice &m_lveDevice_;
		std::unique_ptr<LveSwapChain> m_lveSwapChain_;
		std::vector<VkCommandBuffer> m_commandBuffers_;

		uint32_t m_currentImageIndex_;
		int m_currentFrameIndex_ = 0;
		bool m_isFrameStarted_{false};
	};
}

#endif //VULKAN_TEST_LVERENDERER_HPP
