//
// Created by wdoppenberg on 09-01-22.
//

#include "LveCamera.hpp"

// std
#include <cassert>
#include <limits>


namespace lve {
	void LveCamera::setOrthographicProjection(float left, float right, float top, float bottom, float near,
	                                          float far) {
		m_projectionMatrix_ = glm::mat4{1.0f};

		m_projectionMatrix_[0][0] = 2.f / (right - left);
		m_projectionMatrix_[1][1] = 2.f / (bottom - top);
		m_projectionMatrix_[2][2] = 1.f / (far - near);
		m_projectionMatrix_[3][0] = -(right + left) / (right - left);
		m_projectionMatrix_[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix_[3][2] = -near / (far - near);
	}

	void LveCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
#ifndef NDEBUG
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
#endif
		const float tanHalfFovy = std::tan(fovy / 2.f);
		m_projectionMatrix_ = glm::mat4{0.0f};
		m_projectionMatrix_[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix_[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix_[2][2] = far / (far - near);
		m_projectionMatrix_[2][3] = 1.f;
		m_projectionMatrix_[3][2] = -(far * near) / (far - near);
	}

	void LveCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{glm::normalize(direction)};
		const glm::vec3 u{glm::normalize(glm::cross(w, up))};
		const glm::vec3 v{glm::cross(w, u)};

		m_viewMatrix_ = glm::mat4{1.f};
		m_viewMatrix_[0][0] = u.x;
		m_viewMatrix_[1][0] = u.y;
		m_viewMatrix_[2][0] = u.z;
		m_viewMatrix_[0][1] = v.x;
		m_viewMatrix_[1][1] = v.y;
		m_viewMatrix_[2][1] = v.z;
		m_viewMatrix_[0][2] = w.x;
		m_viewMatrix_[1][2] = w.y;
		m_viewMatrix_[2][2] = w.z;
		m_viewMatrix_[3][0] = -glm::dot(u, position);
		m_viewMatrix_[3][1] = -glm::dot(v, position);
		m_viewMatrix_[3][2] = -glm::dot(w, position);
	}

	void LveCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
		setViewDirection(position, target - position, up);
	}

	void LveCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
		const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
		const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
		m_viewMatrix_ = glm::mat4{1.f};
		m_viewMatrix_[0][0] = u.x;
		m_viewMatrix_[1][0] = u.y;
		m_viewMatrix_[2][0] = u.z;
		m_viewMatrix_[0][1] = v.x;
		m_viewMatrix_[1][1] = v.y;
		m_viewMatrix_[2][1] = v.z;
		m_viewMatrix_[0][2] = w.x;
		m_viewMatrix_[1][2] = w.y;
		m_viewMatrix_[2][2] = w.z;
		m_viewMatrix_[3][0] = -glm::dot(u, position);
		m_viewMatrix_[3][1] = -glm::dot(v, position);
		m_viewMatrix_[3][2] = -glm::dot(w, position);
	}

}