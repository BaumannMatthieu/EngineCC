#pragma once

/// Assimp includes
// C++ importer interface
#include <assimp/Importer.hpp>      
// Output data structure
#include <assimp/scene.h>           
// Post processing flags
#include <assimp/postprocess.h>   

#include "Primitive.h"
#include "Mesh.h"

class Cube : public Primitive {
public:
	Cube() {
		load();
	}
	~Cube() {
	}

	void draw(const std::weak_ptr<Shader> shader) const {
		Primitive::draw(shader);
	}

private:
	void load() {
		std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, -0.5, 0.5),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, -0.5, 0.5),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, 0.5, 0.5),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, 0.5, 0.5),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));

		// vertices for the back face of the cube
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, -0.5, -0.5),
			glm::vec4(1.0, 0.0, 1.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, -0.5, -0.5),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, 0.5, -0.5),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, 0.5, -0.5),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));

		GLuint indexes_arr[] = {
			// front
			0, 1, 2,
			2, 3, 0,
			// top
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// bottom
			4, 0, 3,
			3, 7, 4,
			// left
			4, 5, 1,
			1, 0, 4,
			// right
			3, 2, 6,
			6, 7, 3,
		};
		mesh->m_indexes = std::vector<GLuint>(indexes_arr, indexes_arr + 36);

		m_meshes.push_back(std::move(mesh));

		this->writeBuffers();
	}
};

class Plane : public Primitive {
public:
	Plane() {
		load();
	}
	~Plane() {
	}

	void draw(const std::weak_ptr<Shader> shader) const {
		Primitive::draw(shader);
	}

private:
	void load() {
		std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, 0, -0.5),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, 0, -0.5),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(0.5, 0, 0.5),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(-0.5, 0, 0.5),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));


		GLuint indexes_arr[] = {
			// front
			0, 1, 2,
			2, 3, 0
		};
		mesh->m_indexes = std::vector<GLuint>(indexes_arr, indexes_arr + 6);

		m_meshes.push_back(std::move(mesh));

		this->writeBuffers();
	}
};

class LinePrimitive : public Primitive {
public:
	LinePrimitive(const std::vector<glm::vec3>& points, const std::vector<glm::vec4>& colors) {
		load(points, colors);
	}
	~LinePrimitive() {
	}

	void draw(const std::weak_ptr<Shader> shader) const {
		Primitive::draw(shader);
	}

private:
	void load(const std::vector<glm::vec3>& points, const std::vector<glm::vec4>& colors) {
		std::unique_ptr<Line> mesh = std::make_unique<Line>();
		assert(points.size() == colors.size());
		for(uint32_t i = 0; i < points.size(); i+=2) {
			mesh->m_vertices.push_back(Mesh::VertexFormat(points[i],
				colors[i]));
			mesh->m_vertices.push_back(Mesh::VertexFormat(points[i + 1],
				colors[i + 1]));
		}

		m_meshes.push_back(std::move(mesh));
		this->writeBuffers();
	}
};