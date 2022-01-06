//
// Created by wdoppenberg on 06-01-22.
//

#ifndef VULKAN_TEST_GRAVITY_HPP
#define VULKAN_TEST_GRAVITY_HPP

#include "FirstApp.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {
	class Vec2FieldSystem;

	class GravityPhysicsSystem {
		friend Vec2FieldSystem;
	public:

		void stepSimulation(std::vector<LveGameObject> &physicsObjs, float dt);

		GravityPhysicsSystem(float strength) : m_strengthGravity{strength} {};
		const float m_strengthGravity;

		void update(std::vector<LveGameObject> &objs, float dt, unsigned int substeps = 1);

	private:
		glm::vec2 computeForce(LveGameObject &fromObj, LveGameObject &toObj) const;
	};

	class Vec2FieldSystem {
	public:
		void update(
				const GravityPhysicsSystem &physicsSystem,
				std::vector<LveGameObject> &physicsObjs,
				std::vector<LveGameObject> &vectorField);

	};

	std::unique_ptr<LveModel> createSquareModel(LveDevice &device, glm::vec2 offset);

	std::unique_ptr<LveModel> createCircleModel(LveDevice &device, unsigned int numSides);
}

#endif //VULKAN_TEST_GRAVITY_HPP
