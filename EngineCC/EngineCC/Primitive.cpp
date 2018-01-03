#include "Primitive.h"

#include "BoundingBox.h"
#include "Mesh.h"

Primitive::Primitive() {
}

Primitive::~Primitive() {
}

std::vector<glm::vec3> Primitive::getVertices() const {
	std::vector<glm::vec3> vertices;
	for (int i = 0; i < m_meshes.size(); ++i) {
		const std::vector<glm::vec3>& vertices_current_mesh = m_meshes[i]->getVertices();
		vertices.insert(vertices.end(), vertices_current_mesh.begin(), vertices_current_mesh.end());
	}
	return vertices;
}

void Primitive::draw(const std::weak_ptr<Shader> shader) const {
	for (unsigned int i = 0; i < m_meshes.size(); ++i) {
		m_meshes[i]->draw(shader);
	}
}

void Primitive::setColor(const glm::vec4& color) {
	for (unsigned int i = 0; i < m_meshes.size(); ++i) {
		for (unsigned int j = 0; j < m_meshes[i]->m_vertices.size(); ++j) {
			Mesh::VertexFormat& vertex = m_meshes[i]->m_vertices[j];

			vertex.color = color;
		}
	}

	this->writeBuffers();
}

void Primitive::computeBoundingBoxes(const glm::mat4& model_mat, std::vector<BoundingBox>& bounding_boxes) {
	bounding_boxes.clear();
	for (unsigned int i = 0; i < m_meshes.size(); ++i) {
		bounding_boxes.push_back(BoundingBox::create(m_meshes[i]->m_vertices, model_mat));
	}
}

void Primitive::writeBuffers() {
	for (unsigned int i = 0; i < m_meshes.size(); ++i) {
		m_meshes[i]->createVao();
	}
}

bool Primitive::isAnimated() const {
	return false;
}

