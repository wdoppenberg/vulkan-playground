//
// Created by wdoppenberg on 21-11-21.
//

#include "LveModel.hpp"
#include "LveDevice.hpp"

#ifndef NDEBUG

#include <cassert>

#endif

#include <cstring>

namespace lve {

    LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : lve_device{device} {
        createVertexBuffers(vertices);
    }

    LveModel::~LveModel() {
        vkDestroyBuffer(lve_device.device(), vertex_buffer, nullptr);
        vkFreeMemory(lve_device.device(), vertex_buffer_memory, nullptr);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertex_count = static_cast<uint32_t>(vertices.size());
#ifndef NDEBUG
        assert(vertex_count >= 3 && "Vertex count must be at least 3");
#endif
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;
        lve_device.createBuffer(
                buffer_size,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vertex_buffer,
                vertex_buffer_memory
        );

        void *data;
        vkMapMemory(lve_device.device(), vertex_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(lve_device.device(), vertex_buffer_memory);
    }

    void LveModel::bind(VkCommandBuffer command_buffer) {
        VkBuffer buffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
    }

    void LveModel::draw(VkCommandBuffer command_buffer) const {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }


    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
//        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
//        binding_descriptions[0].binding = 0;
//        binding_descriptions[0].stride = sizeof(Vertex);
//        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//        return binding_descriptions;
        return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        return {{0, 0, VK_FORMAT_R32G32_SFLOAT, 0}};
    }
}