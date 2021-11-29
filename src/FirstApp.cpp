#include "FirstApp.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct SimplePushConstantData {
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    FirstApp::~FirstApp() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void FirstApp::run() {
        while (!lve_window.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(lve_device.device());
    }


    void FirstApp::createPipelineLayout() {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);


        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void FirstApp::createPipeline() {
#ifndef NDEBUG
        assert(lve_swap_chain != nullptr && "Cannot create pipeline before swapchain");
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
#endif
        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);
        pipeline_config.renderPass = lve_swap_chain->getRenderPass();
        pipeline_config.pipelineLayout = pipeline_layout;
        lve_pipeline = std::make_unique<LvePipeline>(
                lve_device,
                "src/shaders/simple_shader.vert.spv",
                "src/shaders/simple_shader.frag.spv",
                pipeline_config
        );
    }


    void FirstApp::createCommandBuffers() {
        command_buffers.resize(lve_swap_chain->imageCount());
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = lve_device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(lve_device.device(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }

    }

    void FirstApp::drawFrame() {
        uint32_t image_index;
        auto result = lve_swap_chain->acquireNextImage(&image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        recordCommandBuffer(static_cast<int>(image_index));
        result = lve_swap_chain->submitCommandBuffers(&command_buffers[image_index], &image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lve_window.wasWindowResized()) {
            lve_window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image");
        }
    }

    void FirstApp::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices{
                {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
        };

        auto lve_model = std::make_shared<LveModel>(lve_device, vertices);

        auto triangle = LveGameObject::createGameObject();
        triangle.model = lve_model;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        game_objects.push_back(std::move(triangle));
    }

    void FirstApp::recordCommandBuffer(const int &image_index) {

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffers[image_index], &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = lve_swap_chain->getRenderPass();
        render_pass_info.framebuffer = lve_swap_chain->getFrameBuffer(image_index);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = lve_swap_chain->getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lve_swap_chain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lve_swap_chain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lve_swap_chain->getSwapChainExtent()};
        vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
        vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

        lve_pipeline->bind(command_buffers[image_index]);
        renderGameObjects(command_buffers[image_index]);

        vkCmdEndRenderPass(command_buffers[image_index]);

        if (vkEndCommandBuffer(command_buffers[image_index]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }

    void FirstApp::freeCommandBuffers() {
        vkFreeCommandBuffers(
                lve_device.device(),
                lve_device.getCommandPool(),
                static_cast<uint32_t>(command_buffers.size()),
                command_buffers.data());
        command_buffers.clear();
    }

    void FirstApp::recreateSwapChain() {
        auto extent = lve_window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = lve_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(lve_device.device());

        if (lve_swap_chain == nullptr) {
            lve_swap_chain = std::make_unique<LveSwapChain>(lve_device, extent);
        } else {
            lve_swap_chain = std::make_unique<LveSwapChain>(lve_device, extent, std::move(lve_swap_chain));
            if (lve_swap_chain->imageCount() != command_buffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }

    void FirstApp::renderGameObjects(VkCommandBuffer command_buffer) {
        lve_pipeline->bind(command_buffer);

        for (auto &obj: game_objects) {
            obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(command_buffer,
                               pipeline_layout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePushConstantData),
                               &push);

            obj.model->bind(command_buffer);
            obj.model->draw(command_buffer);
        }
    }
}