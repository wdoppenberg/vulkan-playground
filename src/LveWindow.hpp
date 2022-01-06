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
            return glfwWindowShouldClose(m_window_);
        }

        VkExtent2D getExtent() const {
	        return {static_cast<uint32_t>(m_width_), static_cast<uint32_t>(m_height_)};
        }

	    bool wasWindowResized() const { return m_frameBufferResized_; }

	    void resetWindowResizedFlag() { m_frameBufferResized_ = false; }

	    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
	    static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

	    GLFWwindow *m_window_;
	    int m_width_, m_height_;
	    bool m_frameBufferResized_ = false;

	    std::string m_windowName_;

    };
}

#endif //VULKAN_TEST_LVEWINDOW_HPP
