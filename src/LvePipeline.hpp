//
// Created by wdoppenberg on 20-11-21.
//
#pragma once
#ifndef VULKAN_TEST_LVEPIPELINE_HPP
#define VULKAN_TEST_LVEPIPELINE_HPP

#include "LveDevice.hpp"

#include <string>
#include <vector>

struct PipelineConfigInfo {
	PipelineConfigInfo() = default;

	PipelineConfigInfo(const PipelineConfigInfo &) = delete;

	PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

	VkPipelineViewportStateCreateInfo m_viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo m_rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo m_multisampleInfo;
	VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo m_colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo;
	std::vector<VkDynamicState> m_dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo m_dynamicStateInfo;
	VkPipelineLayout m_pipelineLayout = nullptr;
	VkRenderPass m_renderPass = nullptr;
	uint32_t m_subpass = 0;
};

namespace lve {
    class LvePipeline {
    public:
	    LvePipeline(LveDevice &device,
	                const std::string &vertFilePath,
	                const std::string &fragFilePath,
	                const PipelineConfigInfo &configInfo);

        ~LvePipeline();

        LvePipeline(const LvePipeline &) = delete;

	    LvePipeline &operator=(const LvePipeline &) = delete;

	    void bind(VkCommandBuffer commandBuffer);

	    static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

    private:
	    static std::vector<char> readFile(const std::string &filePath);

	    void createGraphicsPipeline(const std::string &vertFilePath,
	                                const std::string &fragFilePath,
	                                const PipelineConfigInfo &configInfo);

	    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

	    LveDevice &m_lveDevice_;
	    VkPipeline m_graphicsPipeline_;
	    VkShaderModule m_vertShaderModule_;
	    VkShaderModule m_fragShaderModule_;
    };

}

#endif //VULKAN_TEST_LVEPIPELINE_HPP
