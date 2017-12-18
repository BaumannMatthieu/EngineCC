#include "Mesh.h"

Mesh::Mesh() : m_texture(nullptr) {
}
Mesh::~Mesh() {
}

void Mesh::createVao() {
	if (glIsVertexArray(m_vao))
		glDeleteVertexArrays(1, &m_vao);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::VertexFormat) * this->m_vertices.size(), &(this->m_vertices[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::color)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::normal)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::texcoord)));
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_INT, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::bones_indexes)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::weights)));

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * this->m_indexes.size(), &(this->m_indexes[0]), GL_STATIC_DRAW);

}

void Mesh::draw(const std::weak_ptr<Shader> shader) const {
	glBindVertexArray(m_vao);
	// bind the texture for the mesh
	if (m_texture)
		m_texture->bind(shader, "texture");

	int size;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size / sizeof(GLuint), GL_UNSIGNED_INT, 0);
}