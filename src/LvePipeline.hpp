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
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
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

        void operator=(const LvePipeline &) = delete;

        void bind(VkCommandBuffer command_buffer);

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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
