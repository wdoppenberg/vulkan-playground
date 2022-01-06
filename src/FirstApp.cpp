#include "FirstApp.hpp"
#include "RenderSystem.hpp"
#include "Gravity.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace lve {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		// create some models
		std::shared_ptr<LveModel> squareModel = createSquareModel(
				m_lveDevice_,
				{.5f, .0f});  // offset model by .5 so rotation occurs at edge rather than center of square
		std::shared_ptr<LveModel> circleModel = createCircleModel(m_lveDevice_, 64);

		// create physics objects
		std::vector<LveGameObject> physicsObjects{};
		auto red = LveGameObject::createGameObject();
		red.m_transform2D.m_scale = glm::vec2{.05f};
		red.m_transform2D.m_translation = {.5f, .5f};
		red.m_color = {1.f, 0.f, 0.f};
		red.m_rigidBody2D.velocity = {-.5f, .0f};
		red.m_model = circleModel;
		physicsObjects.push_back(std::move(red));
		auto blue = LveGameObject::createGameObject();
		blue.m_transform2D.m_scale = glm::vec2{.05f};
		blue.m_transform2D.m_translation = {-.45f, -.25f};
		blue.m_color = {0.f, 0.f, 1.f};
		blue.m_rigidBody2D.velocity = {.5f, .0f};
		blue.m_model = circleModel;
		physicsObjects.push_back(std::move(blue));

		// create vector field
		std::vector<LveGameObject> vectorField{};
		int gridCount = 40;
		for (int i = 0; i < gridCount; i++) {
			for (int j = 0; j < gridCount; j++) {
				auto vf = LveGameObject::createGameObject();
				vf.m_transform2D.m_scale = glm::vec2(0.005f);
				vf.m_transform2D.m_translation = {
						-1.0f + (i + 0.5f) * 2.0f / gridCount,
						-1.0f + (j + 0.5f) * 2.0f / gridCount};
				vf.m_color = glm::vec3(1.0f);
				vf.m_model = squareModel;
				vectorField.push_back(std::move(vf));
			}
		}

		GravityPhysicsSystem gravitySystem{0.81f};
		Vec2FieldSystem vecFieldSystem{};

		RenderSystem simpleRenderSystem{m_lveDevice_, m_lveRenderer_.getSwapChainRenderPass()};

		while (!m_lveWindow_.shouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = m_lveRenderer_.beginFrame()) {
				// update systems
				gravitySystem.update(physicsObjects, 1.f / 60, 5);
				vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

				// render system
				m_lveRenderer_.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
				simpleRenderSystem.renderGameObjects(commandBuffer, vectorField);
				m_lveRenderer_.endSwapChainRenderPass(commandBuffer);
				m_lveRenderer_.endFrame();
			}
		}

		vkDeviceWaitIdle(m_lveDevice_.device());
	}


	void FirstApp::loadGameObjects() {
		std::vector<LveModel::Vertex> vertices{
				{{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
				{{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}}
		};

		auto lveModel = std::make_shared<LveModel>(m_lveDevice_, vertices);

		auto triangle = LveGameObject::createGameObject();
		triangle.m_model = lveModel;
		triangle.m_color = {.1f, .8f, .1f};
		triangle.m_transform2D.m_translation.x = .2f;
		triangle.m_transform2D.m_scale = {2.f, .5f};
		triangle.m_transform2D.m_rotation = .25f * glm::two_pi<float>();

		m_gameObjects_.push_back(std::move(triangle));
	}

}