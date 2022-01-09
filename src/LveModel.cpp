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

    LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : m_lveDevice_{device} {
	    createVertexBuffers(vertices);
    }

    LveModel::~LveModel() {
	    vkDestroyBuffer(m_lveDevice_.device(), m_vertexBuffer_, nullptr);
	    vkFreeMemory(m_lveDevice_.device(), m_vertexBufferMemory_, nullptr);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
	    m_vertexCount_ = static_cast<uint32_t>(vertices.size());
#ifndef NDEBUG
	    assert(m_vertexCount_ >= 3 && "Vertex count must be at least 3");
#endif
	    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount_;
	    m_lveDevice_.createBuffer(
			    bufferSize,
			    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			    m_vertexBuffer_,
			    m_vertexBufferMemory_
	    );

	    void *data;
	    vkMapMemory(m_lveDevice_.device(), m_vertexBufferMemory_, 0, bufferSize, 0, &data);
	    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	    vkUnmapMemory(m_lveDevice_.device(), m_vertexBufferMemory_);
    }

	void LveModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {m_vertexBuffer_};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LveModel::draw(VkCommandBuffer commandBuffer) const {
		vkCmdDraw(commandBuffer, m_vertexCount_, 1, 0, 0);
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
        return {
		        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_position)},
		        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_color)}
        };
    }
}