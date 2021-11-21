//
// Created by wdoppenberg on 21-11-21.
//

#ifndef VULKAN_TEST_LVEMODEL_HPP
#define VULKAN_TEST_LVEMODEL_HPP

#include "LveDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

#include <vector>

namespace lve {
    class LveModel {
    public:
        struct Vertex {
            glm::vec2 position;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        LveModel(LveDevice &device, const std::vector<Vertex> &vertices);

        ~LveModel();

        LveModel(const LveModel &) = delete;

        LveModel &operator=(const LveModel &) = delete;

        void bind(VkCommandBuffer command_buffer);

        void draw(VkCommandBuffer command_buffer) const;

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        LveDevice &lve_device;
        VkBuffer vertex_buffer;
        VkDeviceMemory vertex_buffer_memory;
        uint32_t vertex_count;

    };
}

#endif //VULKAN_TEST_LVEMODEL_HPP
