//
// Created by wdoppenberg on 06-01-22.
//

#include "Gravity.hpp"

namespace lve {

	// dt stands for delta time, and specifies the amount of time to advance the simulation
	// substeps is how many intervals to divide the forward time step in. More substeps result in a
	// more stable simulation, but takes longer to compute
	void GravityPhysicsSystem::update(std::vector<LveGameObject> &objs, float dt, unsigned int substeps) {
		const float stepDelta = dt / substeps;
		for (int i = 0; i < substeps; i++) {
			stepSimulation(objs, stepDelta);
		}
	}

	glm::vec2 GravityPhysicsSystem::computeForce(LveGameObject &fromObj, LveGameObject &toObj) const {
		auto offset = fromObj.m_transform2D.m_translation - toObj.m_transform2D.m_translation;
		float distanceSquared = glm::dot(offset, offset);

		// clown town - just going to return 0 if objects are too close together...
		if (glm::abs(distanceSquared) < 1e-10f) {
			return {.0f, .0f};
		}

		float force =
				m_strengthGravity * toObj.m_rigidBody2D.mass * fromObj.m_rigidBody2D.mass / distanceSquared;
		return force * offset / glm::sqrt(distanceSquared);
	}

	void GravityPhysicsSystem::stepSimulation(std::vector<LveGameObject> &physicsObjs, float dt) {
		// Loops through all pairs of objects and applies attractive force between them
		for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
			auto &objA = *iterA;
			for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
				if (iterA == iterB) continue;
				auto &objB = *iterB;

				auto force = computeForce(objA, objB);
				objA.m_rigidBody2D.velocity += dt * -force / objA.m_rigidBody2D.mass;
				objB.m_rigidBody2D.velocity += dt * force / objB.m_rigidBody2D.mass;
			}
		}

		// update each objects position based on its final velocity
		for (auto &obj: physicsObjs) {
			obj.m_transform2D.m_translation += dt * obj.m_rigidBody2D.velocity;
		}
	}

	void Vec2FieldSystem::update(const GravityPhysicsSystem &physicsSystem, std::vector<LveGameObject> &physicsObjs,
	                             std::vector<LveGameObject> &vectorField) {
		// For each field line we caluclate the net graviation force for that point in space
		for (auto &vf: vectorField) {
			glm::vec2 direction{};
			for (auto &obj: physicsObjs) {
				direction += physicsSystem.computeForce(obj, vf);
			}

			// This scales the length of the field line based on the log of the length
			// values were chosen just through trial and error based on what i liked the look
			// of and then the field line is rotated to point in the direction of the field
			vf.m_transform2D.m_scale.x =
					0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
			vf.m_transform2D.m_rotation = atan2(direction.y, direction.x);
		}
	}

	std::unique_ptr<LveModel> createSquareModel(LveDevice &device, glm::vec2 offset) {
		std::vector<LveModel::Vertex> vertices = {
				{{-0.5f, -0.5f}},
				{{0.5f,  0.5f}},
				{{-0.5f, 0.5f}},
				{{-0.5f, -0.5f}},
				{{0.5f,  -0.5f}},
				{{0.5f,  0.5f}},  //
		};
		for (auto &v: vertices) {
			v.m_position += offset;
		}
		return std::make_unique<LveModel>(device, vertices);
	}

	std::unique_ptr<LveModel> createCircleModel(LveDevice &device, unsigned int numSides) {
		std::vector<LveModel::Vertex> uniqueVertices{};
		for (int i = 0; i < numSides; i++) {
			float angle = i * glm::two_pi<float>() / numSides;
			uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle)}});
		}
		uniqueVertices.push_back({});  // adds center vertex at 0, 0

		std::vector<LveModel::Vertex> vertices{};
		for (int i = 0; i < numSides; i++) {
			vertices.push_back(uniqueVertices[i]);
			vertices.push_back(uniqueVertices[(i + 1) % numSides]);
			vertices.push_back(uniqueVertices[numSides]);
		}
		return std::make_unique<LveModel>(device, vertices);
	}
}
