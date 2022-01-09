//
// Created by wdoppenberg on 29-11-21.
//

#ifndef VULKAN_TEST_LVEGAMEOBJECT_HPP
#define VULKAN_TEST_LVEGAMEOBJECT_HPP

#include "LveModel.hpp"

// libs
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

// std
#include <memory>

namespace lve {

	struct TransformComponent {
		glm::vec3 m_translation{};
		glm::vec3 m_scale{1.f, 1.f, 1.f};
		glm::vec3 m_rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4() {
			const float c3 = glm::cos(m_rotation.z);
			const float s3 = glm::sin(m_rotation.z);
			const float c2 = glm::cos(m_rotation.x);
			const float s2 = glm::sin(m_rotation.x);
			const float c1 = glm::cos(m_rotation.y);
			const float s1 = glm::sin(m_rotation.y);
			return glm::mat4{
					{
							m_scale.x * (c1 * c3 + s1 * s2 * s3),
							                 m_scale.x * (c2 * s3),
							                                  m_scale.x * (c1 * s2 * s3 - c3 * s1),
							                                                   0.0f,
					},
					{
							m_scale.y * (c3 * s1 * s2 - c1 * s3),
							                 m_scale.y * (c2 * c3),
							                                  m_scale.y * (c1 * c3 * s2 + s1 * s3),
							                                                   0.0f,
					},
					{
							m_scale.z * (c2 * s1),
							                 m_scale.z * (-s2),
							                                  m_scale.z * (c1 * c2),
							                                                   0.0f,
					},
					{       m_translation.x, m_translation.y, m_translation.z, 1.0f}};
		}
	};

	struct RigidBody2dComponent {
		glm::vec2 velocity;
		float mass{1.0f};
	};

	class LveGameObject {
	public:
		using id_t = unsigned int;

		static LveGameObject createGameObject() {
			static id_t currentId = 0;
			return LveGameObject{currentId++};
		}

		LveGameObject(const LveGameObject &) = delete;

		LveGameObject &operator=(const LveGameObject &) = delete;

		LveGameObject(LveGameObject &&) = default;

		LveGameObject &operator=(LveGameObject &&) = default;

		id_t getId() const { return m_id_; }

		std::shared_ptr<LveModel> m_model{};
		glm::vec3 m_color{};
		TransformComponent m_transform{};
		RigidBody2dComponent m_rigidBody2D{glm::vec2{0, 0}};


	private:
		LveGameObject(id_t objId) : m_id_{objId} {};

		id_t m_id_;
	};
}

#endif //VULKAN_TEST_LVEGAMEOBJECT_HPP
