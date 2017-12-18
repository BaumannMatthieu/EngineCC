#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

struct LocalTransform {
	LocalTransform() : m_tr_mat(glm::mat4(1.f)),
		m_rot_mat(glm::mat4(1.f)),
		m_scale_mat(glm::mat4(1.f)) {
	}
	~LocalTransform() {
	}

	void setScale(const glm::vec3& vec) {
		m_scale_mat = glm::scale(glm::mat4(1.f), vec);
	}

	void scale(const glm::vec3& vec) {
		m_scale_mat = glm::scale(m_scale_mat, vec);
	}

	void setRotation(const glm::vec3& vec, float theta) {
		m_rot_mat = glm::rotate(glm::mat4(1.f), theta, vec);
	}

	void rotate(const glm::vec3& vec, float theta) {
		m_rot_mat = glm::rotate(m_rot_mat, theta, vec);
	}

	void setTranslation(const glm::vec3& vec) {
		m_tr_mat = glm::translate(glm::mat4(1.f), vec);
	}

	void translate(const glm::vec3& vec) {
		m_tr_mat = glm::translate(m_tr_mat, vec);
	}

	const glm::mat4 getModelMatrix() const {
		return m_tr_mat * m_rot_mat * m_scale_mat;
	}

	void setTranslation(const glm::mat4& mat) {
		m_tr_mat = mat;
	}

	void setRotation(const glm::mat4& mat) {
		m_rot_mat = mat;
	}

	void setScale(const glm::mat4& mat) {
		m_scale_mat = mat;
	}

	const glm::mat4& getTranslationMatrix() const {
		return m_tr_mat;
	}

	const glm::mat4& getScaleMatrix() const {
		return m_scale_mat;
	}

	const glm::mat4& getRotationMatrix() const {
		return m_rot_mat;
	}

private:
	glm::mat4 m_rot_mat;
	glm::mat4 m_scale_mat;
	glm::mat4 m_tr_mat;
};
