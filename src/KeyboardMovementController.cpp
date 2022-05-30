//
// Created by wdoppenberg on 30-5-22.
//

#include "KeyboardMovementController.hpp"


namespace lve {
	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow *window, float dt, LveGameObject &gameObject) {
		glm::vec3 rotate{0};
		if (glfwGetKey(window, m_keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, m_keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(window, m_keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, m_keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.m_transform.m_rotation += m_lookSpeed * dt * glm::normalize(rotate);
		}

		gameObject.m_transform.m_rotation.x = glm::clamp(gameObject.m_transform.m_rotation.x, -1.5f, 1.5f);
		gameObject.m_transform.m_rotation.y = glm::mod(gameObject.m_transform.m_rotation.y, glm::two_pi<float>());

		float yaw = gameObject.m_transform.m_rotation.y;
		const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};
		const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
		const glm::vec3 upDir{glm::cross(rightDir, forwardDir)};

		glm::vec3 moveDir{0.f};
		if (glfwGetKey(window, m_keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, m_keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(window, m_keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, m_keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(window, m_keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, m_keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			gameObject.m_transform.m_translation += m_moveSpeed * dt * glm::normalize(moveDir);
		}

	}
}