//
// Created by wdoppenberg on 06-01-22.
//

#include "RenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {
	struct SimplePushConstantData {
		glm::mat4 m_transform{1.f};
		alignas(16) glm::vec3 m_color;
	};


	RenderSystem::RenderSystem(LveDevice &device, VkRenderPass renderPass) : m_lveDevice_{device} {
		createPipelineLayout();
		createPipeline(renderPass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(m_lveDevice_.device(), m_pipelineLayout_, nullptr);
	}

	void RenderSystem::createPipelineLayout() {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_lveDevice_.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout_) !=
		    VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void RenderSystem::createPipeline(VkRenderPass renderPass) {
#ifndef NDEBUG
		assert(m_pipelineLayout_ != nullptr && "Cannot create pipeline before pipeline layout");
#endif
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.m_renderPass = renderPass;
		pipelineConfig.m_pipelineLayout = m_pipelineLayout_;
		m_lvePipeline_ = std::make_unique<LvePipeline>(
				m_lveDevice_,
				"src/shaders/simple_vertex.vert.spv",
				"src/shaders/simple_fragment.frag.spv",
				pipelineConfig
		);
	}


	void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject> &gameObjects) {
		m_lvePipeline_->bind(commandBuffer);

		for (auto &obj: gameObjects) {
			obj.m_transform.m_rotation.y = glm::mod(obj.m_transform.m_rotation.y + 0.01f, glm::two_pi<float>());
			obj.m_transform.m_rotation.x = glm::mod(obj.m_transform.m_rotation.x + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.m_color = obj.m_color;
			push.m_transform = obj.m_transform.mat4();

			vkCmdPushConstants(commandBuffer,
			                   m_pipelineLayout_,
			                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			                   0,
			                   sizeof(SimplePushConstantData),
			                   &push);

			obj.m_model->bind(commandBuffer);
			obj.m_model->draw(commandBuffer);
		}
	}
}