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
		const float tanHalfFovy = tan(fovy / 2.f);
		m_projectionMatrix_ = glm::mat4{0.0f};
		m_projectionMatrix_[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix_[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix_[2][2] = far / (far - near);
		m_projectionMatrix_[2][3] = 1.f;
		m_projectionMatrix_[3][2] = -(far * near) / (far - near);
	}

}