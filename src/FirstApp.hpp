//
// Created by wdoppenberg on 20-11-21.
//
#pragma once

#ifndef VULKAN_TEST_FIRSTAPP_HPP
#define VULKAN_TEST_FIRSTAPP_HPP

#include "LveWindow.hpp"
#include "LvePipeline.hpp"
#include "LveDevice.hpp"

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 800, HEIGHT = 600;

        void run();

    private:
        LveWindow lve_window{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lve_device{lve_window};
        LvePipeline lve_pipeline{lve_device,
                                 "src/shaders/simple_shader.vert.spv",
                                 "src/shaders/simple_shader.frag.spv",
                                 LvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}

#endif //VULKAN_TEST_FIRSTAPP_HPP
