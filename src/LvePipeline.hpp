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

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

namespace lve {
    class LvePipeline {
    public:
        LvePipeline(LveDevice &device,
                    const std::string &vert_file_path,
                    const std::string &frag_file_path,
                    const PipelineConfigInfo &config_info);

        ~LvePipeline();

        LvePipeline(const LvePipeline &) = delete;

        LvePipeline &operator=(const LvePipeline &) = delete;

        void bind(VkCommandBuffer command_buffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

    private:
        static std::vector<char> readFile(const std::string &file_path);

        void createGraphicsPipeline(const std::string &vert_file_path,
                                    const std::string &frag_file_path,
                                    const PipelineConfigInfo &config_info);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shader_module);

        LveDevice &lve_device;
        VkPipeline graphics_pipeline;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;
    };

};

#endif //VULKAN_TEST_LVEPIPELINE_HPP
