//
// Created by wdoppenberg on 20-11-21.
//

#include "LvePipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lve {
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
                                             const PipeLineConfigInfo &config_info) {

        auto vert_code = readFile(vert_file_path);
        auto frag_code = readFile(frag_file_path);

        std::cout << "Vertex Shader Code Size: " << vert_code.size() << "\n";
        std::cout << "Fragment Shader Code Size: " << frag_code.size() << "\n";

    }

    LvePipeline::LvePipeline(LveDevice &device, const std::string &vert_file_path, const std::string &frag_file_path,
                             const PipeLineConfigInfo &config_info) : lve_device{device} {
        createGraphicsPipeline(vert_file_path, frag_file_path, config_info);
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

    PipeLineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipeLineConfigInfo config_info{};

        return config_info;
    }
}