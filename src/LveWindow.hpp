//
// Created by wdoppenberg on 18-11-21.
//
#pragma once
#ifndef VULKAN_TEST_LVEWINDOW_HPP
#define VULKAN_TEST_LVEWINDOW_HPP

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

namespace lve {
    class LveWindow {
    public:
        LveWindow(int w, int h, std::string name);

        ~LveWindow();

        LveWindow(const LveWindow &) = delete;

        LveWindow &operator=(const LveWindow &) = delete;

        void initWindow();

        bool shouldClose() {
            return glfwWindowShouldClose(window);
        }

        VkExtent2D getExtent() const {
            return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        GLFWwindow *window;
        const int width, height;

        std::string window_name;

    };
}

#endif //VULKAN_TEST_LVEWINDOW_HPP
