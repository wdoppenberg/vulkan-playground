//
// Created by wdoppenberg on 20-11-21.
//
#pragma once

#ifndef VULKAN_TEST_FIRSTAPP_HPP
#define VULKAN_TEST_FIRSTAPP_HPP

#include "LveWindow.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"
#include "LveDevice.hpp"
#include "LveSwapChain.hpp"
#include "LveModel.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 800, HEIGHT = 600;

        FirstApp();

        ~FirstApp();

        FirstApp(const FirstApp &) = delete;

        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void loadGameObjects();

        void createPipelineLayout();

        void createPipeline();

        void createCommandBuffers();

        void drawFrame();

        void recreateSwapChain();

        void recordCommandBuffer(const int &image_index);

        void freeCommandBuffers();

        void renderGameObjects(VkCommandBuffer command_buffer);

        LveWindow lve_window{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lve_device{lve_window};
        std::unique_ptr<LveSwapChain> lve_swap_chain;
        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
        std::vector<VkCommandBuffer> command_buffers;
        std::vector<LveGameObject> game_objects;
    };
}

#endif //VULKAN_TEST_FIRSTAPP_HPP
