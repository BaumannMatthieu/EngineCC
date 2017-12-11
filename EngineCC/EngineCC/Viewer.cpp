#include "Viewer.h"
#include "GameProgram.h"

glm::mat4 Viewer::m_projection_mat = 
	glm::perspective(45.f, static_cast<float>(GameProgram::width) / GameProgram::height, 0.1f, 100.f);

Viewer::Viewer(const glm::vec3& position, const glm::vec3& center) : m_position(position),
																	 m_direction(glm::normalize(center - position))
{
	computeViewMatrix();
}

Viewer::~Viewer()
{
}

const glm::mat4& Viewer::getViewMatrix() const {
	return m_view_mat;
}

const glm::vec3& Viewer::getPosition() const {
	return m_position;
}

const glm::mat4& Viewer::getProjectionMatrix() {
	return m_projection_mat;
}

void Viewer::setPosition(const glm::vec3& position) {
	m_position = position;
	computeViewMatrix();
}

void Viewer::translate(const glm::vec3& tr) {
	m_position += tr;
	computeViewMatrix();
}

void Viewer::setDirection(const glm::vec3& direction) {
	m_direction = direction;
	computeViewMatrix();
}

void Viewer::setCenter(const glm::vec3& center) {
	m_direction = glm::normalize(center - m_position);
	computeViewMatrix();
}

void Viewer::computeViewMatrix() {
	m_view_mat = glm::lookAt(m_position, m_position + m_direction, glm::vec3(0.f, 1.f, 0.f));
}
