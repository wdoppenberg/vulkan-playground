#include "LveWindow.hpp"

#include <utility>
#include <stdexcept>


namespace lve {
    LveWindow::LveWindow(int w, int h, std::string name) : m_width_{w}, m_height_{h}, m_windowName_{std::move(name)} {
	    initWindow();
    };

    LveWindow::~LveWindow() {
	    glfwDestroyWindow(m_window_);
	    glfwTerminate();
    }

    void LveWindow::initWindow() {
	    glfwInit();
	    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	    m_window_ = glfwCreateWindow(m_width_, m_height_, m_windowName_.c_str(), nullptr, nullptr);
	    glfwSetWindowUserPointer(m_window_, this);
	    glfwSetFramebufferSizeCallback(m_window_, frameBufferResizeCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
	    if (glfwCreateWindowSurface(instance, m_window_, nullptr, surface) != VK_SUCCESS) {
		    throw std::runtime_error("Failed to create window surface.");
	    }

    }

    void LveWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
	    auto lveWindow = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
	    lveWindow->m_frameBufferResized_ = true;
	    lveWindow->m_width_ = width;
	    lveWindow->m_height_ = height;
    }
}