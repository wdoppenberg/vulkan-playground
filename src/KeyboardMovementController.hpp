//
// Created by wdoppenberg on 30-5-22.
//

#ifndef VULKAN_TEST_KEYBOARDMOVEMENTCONTROLLER_HPP
#define VULKAN_TEST_KEYBOARDMOVEMENTCONTROLLER_HPP

#include "LveGameObject.hpp"
#include "LveWindow.hpp"

namespace lve {
	class KeyboardMovementController {
	public:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};

		void moveInPlaneXZ(GLFWwindow *window, float dt, LveGameObject &gameObject);


		KeyMappings m_keys{};
		float m_moveSpeed{3.f};
		float m_lookSpeed{1.5f};
	};

}


#endif //VULKAN_TEST_KEYBOARDMOVEMENTCONTROLLER_HPP
