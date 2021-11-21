//
// Created by wdoppenberg on 20-11-21.
//

#include "LvePipeline.hpp"
#include "LveModel.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lve {

    LvePipeline::LvePipeline(LveDevice &device,
                             const std::string &vert_file_path,
                             const std::string &frag_file_path,
                             const PipelineConfigInfo &config_info) : lve_device{device} {
        createGraphicsPipeline(vert_file_path, frag_file_path, config_info);
    }

    LvePipeline::~LvePipeline() {
        vkDestroyShaderModule(lve_device.device(), vert_shader_module, nullptr);
        vkDestroyShaderModule(lve_device.device(), frag_shader_module, nullptr);
        vkDestroyPipeline(lve_device.device(), graphics_pipeline, nullptr);
    }

    std::vector<char> LvePipeline::readFile(const std::string &file_path) {
        std::ifstream file{file_path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + file_path);
        }

        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }

    void LvePipeline::createGraphicsPipeline(const std::string &vert_file_path, const std::string &frag_file_path,
                                             const PipelineConfigInfo &config_info) {

#ifndef NDEBUG
        assert(
                config_info.pipelineLayout != VK_NULL_HANDLE &&
                "Cannot create graphics pipeline: no pipelineLayout provided in config_info");
        assert(
                config_info.renderPass != VK_NULL_HANDLE &&
                "Cannot create graphics pipeline: no renderPass provided in config_info");
#endif

        auto vert_code = readFile(vert_file_path);
        auto frag_code = readFile(frag_file_path);
        createShaderModule(vert_code, &vert_shader_module);
        createShaderModule(frag_code, &frag_shader_module);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vert_shader_module;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = frag_shader_module;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto binding_descriptions = LveModel::Vertex::getBindingDescriptions();
        auto attribute_descriptions = LveModel::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
        vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

        VkPipelineViewportStateCreateInfo viewport_info{};
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.pNext = nullptr;
        viewport_info.flags = 0;
        viewport_info.viewportCount = 1;
        viewport_info.pViewports = &config_info.viewport;
        viewport_info.scissorCount = 1;
        viewport_info.pScissors = &config_info.scissor;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shaderStages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &config_info.inputAssemblyInfo;
        pipeline_info.pViewportState = &viewport_info;
        pipeline_info.pRasterizationState = &config_info.rasterizationInfo;
        pipeline_info.pMultisampleState = &config_info.multisampleInfo;
        pipeline_info.pColorBlendState = &config_info.colorBlendInfo;
        pipeline_info.pDepthStencilState = &config_info.depthStencilInfo;
        pipeline_info.pDynamicState = nullptr;

        pipeline_info.layout = config_info.pipelineLayout;
        pipeline_info.renderPass = config_info.renderPass;
        pipeline_info.subpass = config_info.subpass;

        pipeline_info.basePipelineIndex = -1;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(lve_device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                      &graphics_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }

    }


    void LvePipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shader_module) {
        VkShaderModuleCreateInfo create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(lve_device.device(), &create_info, nullptr, shader_module) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module");
        }
    }

    PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipelineConfigInfo configInfo{};

        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;

        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};


        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        configInfo.colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        return configInfo;
    }

    void LvePipeline::bind(VkCommandBuffer command_buffer) {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
    }
}