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
	        glm::vec3 m_position;
	        glm::vec3 m_color;

	        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

	        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        LveModel(LveDevice &device, const std::vector<Vertex> &vertices);

        ~LveModel();

        LveModel(const LveModel &) = delete;

	    LveModel &operator=(const LveModel &) = delete;

	    void bind(VkCommandBuffer commandBuffer);

	    void draw(VkCommandBuffer commandBuffer) const;

    private:
	    void createVertexBuffers(const std::vector<Vertex> &vertices);

	    LveDevice &m_lveDevice_;
	    VkBuffer m_vertexBuffer_;
	    VkDeviceMemory m_vertexBufferMemory_;
	    uint32_t m_vertexCount_;

    };
}

#endif //VULKAN_TEST_LVEMODEL_HPP
