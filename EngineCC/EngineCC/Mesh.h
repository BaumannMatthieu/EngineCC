#pragma once

#include <vector>
#include <memory>
#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"

struct Drawable {
	struct VertexFormat {
		VertexFormat(const glm::vec3& _point,
			const glm::vec4& _color = glm::vec4(0.f),
			const glm::vec3& _normal = glm::vec3(0.f),
			const glm::vec2& _texcoord = glm::vec2(0.f),
			const glm::ivec4& _bones_indexes = glm::ivec4(0),
			const glm::vec4& _weights = glm::vec4(0.f)) : point(_point),
			color(_color),
			normal(_normal),
			texcoord(_texcoord),
			bones_indexes(_bones_indexes),
			weights(_weights) {

		}

		glm::vec3 point;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 texcoord;

		// Vertex animation
		glm::ivec4 bones_indexes;
		glm::vec4 weights;
	};

	Drawable();
	virtual ~Drawable();

	virtual void createVao() = 0;
	virtual void draw(const std::weak_ptr<Shader> shader) const = 0;

	virtual std::vector<glm::vec3> getVertices() const = 0;

public:
	GLuint m_vao;
	std::vector<VertexFormat> m_vertices;

};

struct Mesh : public Drawable {
public:
	Mesh();
	virtual ~Mesh();

	void createVao();
	void draw(const std::weak_ptr<Shader> shader) const;
	std::vector<glm::vec3> getVertices() const;

public:
	std::vector<GLuint> m_indexes;

	// Two meshes can reference the same texture => shared_ptr
	GLuint m_material_index;
	std::shared_ptr<Texture> m_texture;
};

struct Line : public Drawable {
public:
	Line();
	virtual ~Line();

	void createVao();
	void draw(const std::weak_ptr<Shader> shader) const;
	std::vector<glm::vec3> getVertices() const;
};


