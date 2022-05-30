//
// Created by wdoppenberg on 09-01-22.
//

#ifndef VULKAN_TEST_LVECAMERA_HPP
#define VULKAN_TEST_LVECAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>


namespace lve {
	class LveCamera {
	public:

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		const glm::mat4 &getProjection() const { return m_projectionMatrix_; }

	private:
		glm::mat4 m_projectionMatrix_{1.f};
	};

}

#endif //VULKAN_TEST_LVECAMERA_HPP
