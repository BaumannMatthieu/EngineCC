#include "Viewer.h"
#include "GameProgram.h"

glm::mat4 Viewer::m_projection_mat = 
	glm::perspective(20.0f, static_cast<float>(GameProgram::win_width) / GameProgram::win_height, 0.1f, 100.f);

Viewer::Viewer(const glm::vec3& position_view, const glm::vec3& focus_view)
{
	// View matrix definition
	m_view_mat = glm::lookAt(position_view, focus_view, glm::vec3(0.f, 1.f, 0.f));
}


Viewer::~Viewer()
{
}

const glm::mat4& Viewer::getViewMatrix() const {
	return m_view_mat;
}

const glm::mat4& Viewer::getProjectionMatrix() {
	return m_projection_mat;
}
