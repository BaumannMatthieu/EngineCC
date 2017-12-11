#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Viewer
{
public:
	Viewer(const glm::vec3& position = glm::vec3(10.f, 10.f, 10.f),
		   const glm::vec3& center = glm::vec3(0.f));
	~Viewer();

	void setPosition(const glm::vec3& position);
	void translate(const glm::vec3& tr);
	void setDirection(const glm::vec3& direction);
	void setCenter(const glm::vec3& center);

	const glm::mat4& getViewMatrix() const;
	const glm::vec3& getPosition() const;
	static const glm::mat4& getProjectionMatrix();

private:
	void computeViewMatrix();

private:
	glm::mat4 m_view_mat;
	glm::vec3 m_position;
	glm::vec3 m_direction;

	static glm::mat4 m_projection_mat;
};

