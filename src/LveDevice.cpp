#include "LveDevice.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace lve {

// local callback functions
    static VKAPI_ATTR VkBool32

    VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult createDebugUtilsMessengerExt(
		    VkInstance instance,
		    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator,
		    VkDebugUtilsMessengerEXT *pDebugMessenger) {
	    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
			    instance,
			    "vkCreateDebugUtilsMessengerEXT");
	    if (func != nullptr) {
		    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	    } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

	void destroyDebugUtilsMessengerExt(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks *pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				instance,
				"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

// class member functions
	LveDevice::LveDevice(LveWindow &window) : m_window_{window} {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

    LveDevice::~LveDevice() {
	    vkDestroyCommandPool(m_device_, m_commandPool_, nullptr);
	    vkDestroyDevice(m_device_, nullptr);

	    if (m_enableValidationLayers) {
		    destroyDebugUtilsMessengerExt(m_instance_, m_debugMessenger_, nullptr);
	    }

	    vkDestroySurfaceKHR(m_instance_, m_surface_, nullptr);
	    vkDestroyInstance(m_instance_, nullptr);
    }

    void LveDevice::createInstance() {
	    if (m_enableValidationLayers && !checkValidationLayerSupport()) {
		    throw std::runtime_error("validation layers requested, but not available!");
	    }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "LittleVulkanEngine App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

	    VkInstanceCreateInfo createInfo = {};
	    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	    createInfo.pApplicationInfo = &appInfo;

	    auto extensions = getRequiredExtensions();
	    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	    createInfo.ppEnabledExtensionNames = extensions.data();

	    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	    if (m_enableValidationLayers) {
		    createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers_.size());
		    createInfo.ppEnabledLayerNames = m_validationLayers_.data();

		    populateDebugMessengerCreateInfo(debugCreateInfo);
		    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
	    } else {
		    createInfo.enabledLayerCount = 0;
		    createInfo.pNext = nullptr;
	    }

	    if (vkCreateInstance(&createInfo, nullptr, &m_instance_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create instance!");
	    }

        hasGflwRequiredInstanceExtensions();
    }

    void LveDevice::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
	    vkEnumeratePhysicalDevices(m_instance_, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::cout << "Device count: " << deviceCount << std::endl;
        std::vector<VkPhysicalDevice> devices(deviceCount);
	    vkEnumeratePhysicalDevices(m_instance_, &deviceCount, devices.data());

        for (const auto &device: devices) {
	        if (isDeviceSuitable(device)) {
		        m_physicalDevice_ = device;
		        break;
	        }
        }

	    if (m_physicalDevice_ == VK_NULL_HANDLE) {
		    throw std::runtime_error("failed to find a suitable GPU!");
	    }

	    vkGetPhysicalDeviceProperties(m_physicalDevice_, &m_properties);
	    std::cout << "physical device: " << m_properties.deviceName << std::endl;
    }

    void LveDevice::createLogicalDevice() {
	    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice_);

	    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	    std::set<uint32_t> uniqueQueueFamilies = {indices.m_graphicsFamily, indices.m_presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

	    VkPhysicalDeviceFeatures deviceFeatures = {};
	    deviceFeatures.samplerAnisotropy = VK_TRUE;

	    VkDeviceCreateInfo createInfo = {};
	    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	    createInfo.pQueueCreateInfos = queueCreateInfos.data();

	    createInfo.pEnabledFeatures = &deviceFeatures;
	    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions_.size());
	    createInfo.ppEnabledExtensionNames = m_deviceExtensions_.data();

	    // might not really be necessary anymore because device specific validation layers
	    // have been deprecated
	    if (m_enableValidationLayers) {
		    createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers_.size());
		    createInfo.ppEnabledLayerNames = m_validationLayers_.data();
	    } else {
		    createInfo.enabledLayerCount = 0;
	    }

	    if (vkCreateDevice(m_physicalDevice_, &createInfo, nullptr, &m_device_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create logical device!");
	    }

	    vkGetDeviceQueue(m_device_, indices.m_graphicsFamily, 0, &m_graphicsQueue_);
	    vkGetDeviceQueue(m_device_, indices.m_presentFamily, 0, &m_presentQueue_);
    }

    void LveDevice::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
	    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	    poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily;
	    poolInfo.flags =
			    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	    if (vkCreateCommandPool(m_device_, &poolInfo, nullptr, &m_commandPool_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create command pool!");
	    }
    }

	void LveDevice::createSurface() { m_window_.createWindowSurface(m_instance_, &m_surface_); }

    bool LveDevice::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
	        swapChainAdequate = !swapChainSupport.m_formats.empty() && !swapChainSupport.m_presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate &&
               supportedFeatures.samplerAnisotropy;
    }

    void LveDevice::populateDebugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void LveDevice::setupDebugMessenger() {
	    if (!m_enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
	    if (createDebugUtilsMessengerExt(m_instance_, &createInfo, nullptr, &m_debugMessenger_) != VK_SUCCESS) {
		    throw std::runtime_error("failed to set up debug messenger!");
	    }
    }

    bool LveDevice::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	    for (const char *layerName: m_validationLayers_) {
		    bool layerFound = false;

		    for (const auto &layerProperties: availableLayers) {
			    if (strcmp(layerName, layerProperties.layerName) == 0) {
				    layerFound = true;
				    break;
			    }
		    }

		    if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> LveDevice::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	    if (m_enableValidationLayers) {
		    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	    }

        return extensions;
    }

    void LveDevice::hasGflwRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto &extension: extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const auto &required: requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required glfw extension");
            }
        }
    }

    bool LveDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extensionCount,
                availableExtensions.data());

	    std::set<std::string> requiredExtensions(m_deviceExtensions_.begin(), m_deviceExtensions_.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices LveDevice::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily: queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	            indices.m_graphicsFamily = i;
	            indices.m_graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
	        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface_, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
	            indices.m_presentFamily = i;
	            indices.m_presentFamilyHasValue = true;
            }
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails LveDevice::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
	    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface_, &details.m_capabilities);

        uint32_t formatCount;
	    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface_, &formatCount, nullptr);

        if (formatCount != 0) {
	        details.m_formats.resize(formatCount);
	        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface_, &formatCount, details.m_formats.data());
        }

        uint32_t presentModeCount;
	    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface_, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
	        details.m_presentModes.resize(presentModeCount);
	        vkGetPhysicalDeviceSurfacePresentModesKHR(
			        device,
			        m_surface_,
			        &presentModeCount,
			        details.m_presentModes.data());
        }
        return details;
    }

    VkFormat LveDevice::findSupportedFormat(
            const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
	        vkGetPhysicalDeviceFormatProperties(m_physicalDevice_, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (
                    tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    uint32_t LveDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
	    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice_, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void LveDevice::createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	    if (vkCreateBuffer(m_device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create vertex buffer!");
	    }

        VkMemoryRequirements memRequirements;
	    vkGetBufferMemoryRequirements(m_device_, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	    if (vkAllocateMemory(m_device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		    throw std::runtime_error("failed to allocate vertex buffer memory!");
	    }

	    vkBindBufferMemory(m_device_, buffer, bufferMemory, 0);
    }

    VkCommandBuffer LveDevice::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	    allocInfo.commandPool = m_commandPool_;
	    allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
	    vkAllocateCommandBuffers(m_device_, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void LveDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	    vkEndCommandBuffer(commandBuffer);

	    VkSubmitInfo submitInfo{};
	    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	    submitInfo.commandBufferCount = 1;
	    submitInfo.pCommandBuffers = &commandBuffer;

	    vkQueueSubmit(m_graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
	    vkQueueWaitIdle(m_graphicsQueue_);

	    vkFreeCommandBuffers(m_device_, m_commandPool_, 1, &commandBuffer);
    }

    void LveDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void LveDevice::copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region);
        endSingleTimeCommands(commandBuffer);
    }

    void LveDevice::createImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &imageMemory) {
	    if (vkCreateImage(m_device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		    throw std::runtime_error("failed to create image!");
	    }

        VkMemoryRequirements memRequirements;
	    vkGetImageMemoryRequirements(m_device_, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	    if (vkAllocateMemory(m_device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		    throw std::runtime_error("failed to allocate image memory!");
	    }

	    if (vkBindImageMemory(m_device_, image, imageMemory, 0) != VK_SUCCESS) {
		    throw std::runtime_error("failed to bind image memory!");
	    }
    }

}  // namespace lve