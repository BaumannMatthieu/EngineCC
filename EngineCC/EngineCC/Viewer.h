#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Viewer
{
public:
	Viewer(const glm::vec3& position_view = glm::vec3(5.f, 5.f, 5.f), const glm::vec3& focus_view = glm::vec3(0.f));
	~Viewer();
	const glm::mat4& getViewMatrix() const;

	static const glm::mat4& getProjectionMatrix();

private:
	glm::mat4 m_view_mat;

	static glm::mat4 m_projection_mat;
};

