#pragma once

#include "LveWindow.hpp"

// std lib headers
#include <string>
#include <vector>

namespace lve {

    struct SwapChainSupportDetails {
	    VkSurfaceCapabilitiesKHR m_capabilities;
	    std::vector<VkSurfaceFormatKHR> m_formats;
	    std::vector<VkPresentModeKHR> m_presentModes;
    };

	struct QueueFamilyIndices {
		uint32_t m_graphicsFamily;
		uint32_t m_presentFamily;
		bool m_graphicsFamilyHasValue = false;
		bool m_presentFamilyHasValue = false;

		bool isComplete() const { return m_graphicsFamilyHasValue && m_presentFamilyHasValue; }
	};

    class LveDevice {
    public:
#ifdef NDEBUG
		const bool m_enableValidationLayers = false;
#else
		const bool m_enableValidationLayers = true;
#endif

        explicit LveDevice(LveWindow &window);

        ~LveDevice();

        // Not copyable or movable
        LveDevice(const LveDevice &) = delete;

        LveDevice &operator=(const LveDevice &) = delete;

        LveDevice(LveDevice &&) = delete;

        LveDevice &operator=(LveDevice &&) = delete;

		VkCommandPool getCommandPool() { return m_commandPool_; }

		VkDevice device() { return m_device_; }

		VkSurfaceKHR surface() { return m_surface_; }

		VkQueue graphicsQueue() { return m_graphicsQueue_; }

		VkQueue presentQueue() { return m_presentQueue_; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_physicalDevice_); }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice_); }

        VkFormat findSupportedFormat(
                const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkBuffer &buffer,
                VkDeviceMemory &bufferMemory);

        VkCommandBuffer beginSingleTimeCommands();

        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void copyBufferToImage(
                VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

        void createImageWithInfo(
                const VkImageCreateInfo &imageInfo,
                VkMemoryPropertyFlags properties,
                VkImage &image,
                VkDeviceMemory &imageMemory);

		VkPhysicalDeviceProperties m_properties;

    private:
        void createInstance();

        void setupDebugMessenger();

        void createSurface();

        void pickPhysicalDevice();

        void createLogicalDevice();

        void createCommandPool();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);

        std::vector<const char *> getRequiredExtensions();

        bool checkValidationLayerSupport();

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

		void hasGflwRequiredInstanceExtensions();

		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

		VkInstance m_instance_;
		VkDebugUtilsMessengerEXT m_debugMessenger_;
		VkPhysicalDevice m_physicalDevice_ = VK_NULL_HANDLE;
		LveWindow &m_window_;
		VkCommandPool m_commandPool_;

		VkDevice m_device_;
		VkSurfaceKHR m_surface_;
		VkQueue m_graphicsQueue_;
		VkQueue m_presentQueue_;

		const std::vector<const char *> m_validationLayers_ = {"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char *> m_deviceExtensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	};

}  // namespace lve