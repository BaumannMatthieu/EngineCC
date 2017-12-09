#pragma once
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Dependencies\glew\glew.h"

#include "Shader.h"
#include "Viewer.h"

struct Mesh {
	struct VertexFormat {
		VertexFormat(const glm::vec3& _vertice, const glm::vec4& _color) : vertice(_vertice),
																		   color(_color) {
		}
		glm::vec3 vertice;
		glm::vec4 color;
	};
	Mesh() {}
	~Mesh() {}

	std::vector<VertexFormat> data;
	/*std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texcoords;*/
};

template<typename T>
class Renderable : public T
{
public:
	Renderable() : m_shader("vertex_shader.glsl", "fragment_shader.glsl"),
				   m_model_mat(glm::mat4(1.f)) {
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		T::defineMesh(m_mesh);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::VertexFormat) * m_mesh.data.size(), &m_mesh.data[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::VertexFormat), (void*)(offsetof(Mesh::VertexFormat, Mesh::VertexFormat::color)));
	}
	~Renderable() {
	}

	void draw(const Viewer& viewer) const {
		m_shader.bind();

		glUniformMatrix4fv(m_shader.getUniformLocation("modelview"), 1, false, glm::value_ptr(viewer.getViewMatrix() * m_model_mat));
		glUniformMatrix4fv(m_shader.getUniformLocation("projection"), 1, false, glm::value_ptr(Viewer::getProjectionMatrix()));

		/* Texture 
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(program, "texture_grass"), 0);
		glBindTexture(GL_TEXTURE_2D, m_texture_grass);
		*/

		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, m_mesh.data.size());
	}

private:
	GLuint m_vao;
	// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
	Mesh m_mesh;
	
	// The model matrix relative to the renderable
	glm::mat4 m_model_mat;

	// Shader of the renderable
	Shader m_shader;
	// Texture ressource
};

struct Cube {
	void defineMesh(Mesh& mesh) {
		//vertices for the front face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, 1.0),
			glm::vec4(1.0, 0.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, 1.0),
			glm::vec4(0.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));

		//vertices for the right face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, -1.0),
			glm::vec4(1.0, 1.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, -1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, -1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, 1.0),
			glm::vec4(1.0, 0.0, 1.0, 1.0)));

		//vertices for the back face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, -1.0),
			glm::vec4(0.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, -1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, -1.0),
			glm::vec4(1.0, 1.0, 0.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, -1.0),
			glm::vec4(0.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, -1.0),
			glm::vec4(1.0, 1.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, -1.0),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));

		//vertices for the left face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, -1.0),
			glm::vec4(0.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, 1.0),
			glm::vec4(0.0, 1.0, 1.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, -1.0),
			glm::vec4(0.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, 1.0),
			glm::vec4(0.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, -1.0),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));

		//vertices for the upper face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, 1.0),
			glm::vec4(0.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, -1.0),
			glm::vec4(1.0, 1.0, 0.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, 1.0),
			glm::vec4(0.0, 1.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, 1.0, -1.0),
			glm::vec4(1.0, 1.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, 1.0, -1.0),
			glm::vec4(0.0, 1.0, 0.0, 1.0)));

		//vertices for the bottom face of the cube
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, -1.0),
			glm::vec4(0.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, -1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));

		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, -1.0),
			glm::vec4(1.0, 0.0, 0.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(-1.0, -1.0, 1.0),
			glm::vec4(0.0, 0.0, 1.0, 1.0)));
		mesh.data.push_back(Mesh::VertexFormat(glm::vec3(1.0, -1.0, 1.0),
			glm::vec4(1.0, 0.0, 1.0, 1.0)));
	}
};
