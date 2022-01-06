//
// Created by wdoppenberg on 29-11-21.
//

#ifndef VULKAN_TEST_LVEGAMEOBJECT_HPP
#define VULKAN_TEST_LVEGAMEOBJECT_HPP

#include "LveModel.hpp"

#include <memory>

namespace lve {

	struct Transform2DComponent {
		glm::vec2 m_translation{};
		glm::vec2 m_scale{1.f, 1.f};
		float m_rotation;

		glm::mat2 mat2() const {
			const float s = glm::sin(m_rotation);
			const float c = glm::cos(m_rotation);

			glm::mat2 rotMatrix{{c,  s},
			                    {-s, c}};


			glm::mat2 scaleMat{{m_scale.x, 0.f},
			                   {.0f,       m_scale.y}};
			return rotMatrix * scaleMat;
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
		Transform2DComponent m_transform2D{};
		RigidBody2dComponent m_rigidBody2D{glm::vec2{0, 0}};


	private:
		LveGameObject(id_t objId) : m_id_{objId} {};

		id_t m_id_;
	};
}

#endif //VULKAN_TEST_LVEGAMEOBJECT_HPP
