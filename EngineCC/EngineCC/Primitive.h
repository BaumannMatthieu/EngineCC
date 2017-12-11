#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"

struct Mesh {
	struct VertexFormat {
		VertexFormat(const glm::vec3& _point,
			const glm::vec4& _color = glm::vec4(0.f),
			const glm::vec3& _normal = glm::vec3(0.f),
			const glm::vec2& _texcoord = glm::vec2(0.f)) : point(_point),
			color(_color),
			normal(_normal),
			texcoord(_texcoord) {
		}

		glm::vec3 point;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	Mesh() : m_texture(nullptr) {
	}
	~Mesh() {
	}

	void createVao() {
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

		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * this->m_indexes.size(), &(this->m_indexes[0]), GL_STATIC_DRAW);
	}

	void draw(const std::weak_ptr<Shader> shader) const {
		glBindVertexArray(m_vao);
		// bind the texture for the mesh
		if (m_texture)
			m_texture->bind(shader, "texture");

		int size;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glDrawElements(GL_TRIANGLES, size / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	}

public:
	GLuint m_vao;
	std::vector<VertexFormat> m_vertices;
	std::vector<GLuint> m_indexes;
	// Two meshes can reference the same texture => shared_ptr
	GLuint m_material_index;
	std::shared_ptr<Texture> m_texture;
};

class Primitive {
public:
	Primitive() {
	}

	virtual ~Primitive() {
	}

	virtual bool load() = 0;

public:
	// A model is defined by its data vertices
	std::vector<std::unique_ptr<Mesh>> m_meshes;
};
