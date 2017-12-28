#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

struct LocalTransform {
	LocalTransform() : m_tr_mat(glm::mat4(1.f)),
		m_rot_mat(glm::mat4(1.f)),
		m_scale_mat(glm::mat4(1.f)),
		m_scale_vec(glm::vec3(1.f)){
	}
	~LocalTransform() {
	}

	void setScale(const glm::vec3& vec) {
		m_scale_mat = glm::scale(glm::mat4(1.f), vec);
		m_scale_vec = vec;
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
		m_scale_vec = glm::vec3(mat[0][0], mat[1][1], mat[2][2]);
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

	const glm::vec3& getScaleVec() const {
		return m_scale_vec;
	}

private:
	glm::mat4 m_rot_mat;
	glm::mat4 m_scale_mat;
	glm::mat4 m_tr_mat;

	glm::vec3 m_scale_vec;
};
