#include "FirstApp.hpp"
#include "RenderSystem.hpp"
#include "LveCamera.hpp"
#include "KeyboardMovementController.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>


namespace lve {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		RenderSystem simpleRenderSystem{m_lveDevice_, m_lveRenderer_.getSwapChainRenderPass()};
		LveCamera camera{};
		camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});
		auto viewerObject = LveGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		float aspect = m_lveRenderer_.getAspectRatio();


		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!m_lveWindow_.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			frameTime = std::min(frameTime, 0.1f);

			cameraController.moveInPlaneXZ(m_lveWindow_.getWindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.m_transform.m_translation, viewerObject.m_transform.m_rotation);

			aspect = m_lveRenderer_.getAspectRatio();
//			camera.setOrthographicProjection(-aspect, aspect, -1., 1., -1., 1.);
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
			if (auto commandBuffer = m_lveRenderer_.beginFrame()) {
				// render system
				m_lveRenderer_.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects_, camera);
				m_lveRenderer_.endSwapChainRenderPass(commandBuffer);
				m_lveRenderer_.endFrame();
			}
		}

		vkDeviceWaitIdle(m_lveDevice_.device());
	}


	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<LveModel> createCubeModel(LveDevice &device, glm::vec3 offset) {
		std::vector<LveModel::Vertex> vertices{

				// left face (white)
				{{-.5f, -.5f, -.5f},  {.9f, .9f, .9f}},
				{{-.5f, .5f,  .5f},   {.9f, .9f, .9f}},
				{{-.5f, -.5f, .5f},   {.9f, .9f, .9f}},
				{{-.5f, -.5f, -.5f},  {.9f, .9f, .9f}},
				{{-.5f, .5f,  -.5f},  {.9f, .9f, .9f}},
				{{-.5f, .5f,  .5f},   {.9f, .9f, .9f}},

				// right face (yellow)
				{{.5f,  -.5f, -.5f},  {.8f, .8f, .1f}},
				{{.5f,  .5f,  .5f},   {.8f, .8f, .1f}},
				{{.5f,  -.5f, .5f},   {.8f, .8f, .1f}},
				{{.5f,  -.5f, -.5f},  {.8f, .8f, .1f}},
				{{.5f,  .5f,  -.5f},  {.8f, .8f, .1f}},
				{{.5f,  .5f,  .5f},   {.8f, .8f, .1f}},

				// top face (orange, remember y axis points down)
				{{-.5f, -.5f, -.5f},  {.9f, .6f, .1f}},
				{{.5f,  -.5f, .5f},   {.9f, .6f, .1f}},
				{{-.5f, -.5f, .5f},   {.9f, .6f, .1f}},
				{{-.5f, -.5f, -.5f},  {.9f, .6f, .1f}},
				{{.5f,  -.5f, -.5f},  {.9f, .6f, .1f}},
				{{.5f,  -.5f, .5f},   {.9f, .6f, .1f}},

				// bottom face (red)
				{{-.5f, .5f,  -.5f},  {.8f, .1f, .1f}},
				{{.5f,  .5f,  .5f},   {.8f, .1f, .1f}},
				{{-.5f, .5f,  .5f},   {.8f, .1f, .1f}},
				{{-.5f, .5f,  -.5f},  {.8f, .1f, .1f}},
				{{.5f,  .5f,  -.5f},  {.8f, .1f, .1f}},
				{{.5f,  .5f,  .5f},   {.8f, .1f, .1f}},

				// nose face (blue)
				{{-.5f, -.5f, 0.5f},  {.1f, .1f, .8f}},
				{{.5f,  .5f,  0.5f},  {.1f, .1f, .8f}},
				{{-.5f, .5f,  0.5f},  {.1f, .1f, .8f}},
				{{-.5f, -.5f, 0.5f},  {.1f, .1f, .8f}},
				{{.5f,  -.5f, 0.5f},  {.1f, .1f, .8f}},
				{{.5f,  .5f,  0.5f},  {.1f, .1f, .8f}},

				// tail face (green)
				{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
				{{.5f,  .5f,  -0.5f}, {.1f, .8f, .1f}},
				{{-.5f, .5f,  -0.5f}, {.1f, .8f, .1f}},
				{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
				{{.5f,  -.5f, -0.5f}, {.1f, .8f, .1f}},
				{{.5f,  .5f,  -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto &v: vertices) {
			v.m_position += offset;
		}
		return std::make_unique<LveModel>(device, vertices);
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<LveModel> lveModel = createCubeModel(m_lveDevice_, {0.f, 0.f, 0.f});

		auto cube = LveGameObject::createGameObject();
		cube.m_model = lveModel;
		cube.m_transform.m_translation = {0.f, 0.f, 2.5f};
		cube.m_transform.m_scale = {.5, .5f, .5f};
		m_gameObjects_.push_back(std::move(cube));
	}

}