//
// Created by wdoppenberg on 20-11-21.
//

#ifndef VULKAN_TEST_FIRSTAPP_HPP
#define VULKAN_TEST_FIRSTAPP_HPP

#include "LveWindow.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"
#include "LveDevice.hpp"
#include "LveSwapChain.hpp"
#include "LveModel.hpp"
#include "LveRenderer.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

namespace lve {
    class FirstApp {
    public:
	    static constexpr int m_width = 800, m_height = 600;

        FirstApp();

        ~FirstApp();

	    FirstApp(const FirstApp &) = delete;

	    FirstApp &operator=(const FirstApp &) = delete;

	    void run();

    private:
	    void loadGameObjects();

	    LveWindow m_lveWindow_{m_width, m_height, "Hello Vulkan!"};
	    LveDevice m_lveDevice_{m_lveWindow_};
	    LveRenderer m_lveRenderer_{m_lveWindow_, m_lveDevice_};
	    std::vector<LveGameObject> m_gameObjects_;
    };
}

#endif //VULKAN_TEST_FIRSTAPP_HPP
