//
// Created by wdoppenberg on 06-01-22.
//

#ifndef VULKAN_TEST_RENDERSYSTEM_HPP
#define VULKAN_TEST_RENDERSYSTEM_HPP

#include "LvePipeline.hpp"
#include "LveGameObject.hpp"
#include "LveDevice.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

namespace lve {
	class RenderSystem {
	public:

		RenderSystem(LveDevice &device, VkRenderPass renderPass);

		~RenderSystem();

		RenderSystem(const RenderSystem &) = delete;

		RenderSystem &operator=(const RenderSystem &) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject> &gameObjects);

	private:
		void createPipelineLayout();

		void createPipeline(VkRenderPass renderPass);

		LveDevice &m_lveDevice_;
		std::unique_ptr<LvePipeline> m_lvePipeline_;
		VkPipelineLayout m_pipelineLayout_;


	};
}


#endif //VULKAN_TEST_RENDERSYSTEM_HPP
