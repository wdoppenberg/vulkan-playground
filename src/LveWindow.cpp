#include "LveWindow.hpp"

#include <utility>
#include <stdexcept>


namespace lve {
    LveWindow::LveWindow(int w, int h, std::string name) : width{w}, height{h}, window_name{std::move(name)} {
        initWindow();
    };

    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface.");
        }

    }

    void LveWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto lve_window = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        lve_window->frame_buffer_resized = true;
        lve_window->width = width;
        lve_window->height = height;
    }
}