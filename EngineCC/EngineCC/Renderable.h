#pragma once
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "Dependencies\glew\glew.h"

#include "Shader.h"
#include "Viewer.h"
#include "Texture.h"

#include "Primitive.h"

struct LocalTransform {
	LocalTransform() : m_tr_mat(glm::mat4(1.f)),
					   m_rot_mat(glm::mat4(1.f)),
					   m_scale_mat(glm::mat4(1.f)) {
	}
	~LocalTransform() {
	}

	void setScale(const glm::vec3& vec) {
		m_scale_mat = glm::scale(glm::mat4(1.f), vec);
	}

	void scale(const glm::vec3& vec) {
		m_scale_mat = glm::scale(m_scale_mat, vec);
	}

	void setRotation(const glm::vec3& vec, float theta) {
		m_rot_mat = glm::rotate(glm::mat4(1.f), theta, vec);
	}

	void rotate(const glm::vec3& vec, float theta) {
		m_rot_mat = glm::rotate(m_rot_mat, theta, vec);
	}

	void setTranslation(const glm::vec3& vec) {
		m_tr_mat = glm::translate(glm::mat4(1.f), vec);
	}

	void translate(const glm::vec3& vec) {
		m_tr_mat = glm::translate(m_tr_mat, vec);
	}

	const glm::mat4 getModelMatrix() const {
		return m_tr_mat * m_rot_mat * m_scale_mat;
	}

	void setTranslation(const glm::mat4& mat) {
		m_tr_mat = mat;
	}

	void setRotation(const glm::mat4& mat) {
		m_rot_mat = mat;
	}
	
	void setScale(const glm::mat4& mat) {
		m_scale_mat = mat;
	}

	const glm::mat4& getTranslationMatrix() const {
		return m_tr_mat;
	}

	const glm::mat4& getScaleMatrix() const {
		return m_scale_mat;
	}
	
	const glm::mat4& getRotationMatrix() const {
		return m_rot_mat;
	}

private:
	glm::mat4 m_rot_mat;
	glm::mat4 m_scale_mat;
	glm::mat4 m_tr_mat;
};

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;

	static std::unique_ptr<BoundingBox> create(const MeshData& mesh, const glm::mat4& m_model_mat) {
		std::unique_ptr<BoundingBox> box = std::make_unique<BoundingBox>();
		glm::vec3 world_space_vertice = glm::vec3(m_model_mat * glm::vec4(mesh.data[0].vertice, 1));
		box->min = world_space_vertice;
		box->max = world_space_vertice;
		for (unsigned int i = 1; i < mesh.data.size(); ++i) {
			glm::vec3 world_space_vertice = glm::vec3(m_model_mat * glm::vec4(mesh.data[i].vertice, 1));

			box->max = glm::max(box->max, world_space_vertice);
			box->min = glm::min(box->min, world_space_vertice);
		}

		return std::move(box);
	}
};

class RenderObject {
public:

	RenderObject() : m_polygon_mode(GL_FILL) {
	}
	
	virtual ~RenderObject() {
	}

	void setPolygonMode(GLuint polygon_mode) {
		m_polygon_mode = polygon_mode;
	}

	const LocalTransform& getLocalTransform() const {
		return m_transform;
	}

	virtual void setLocalTransform(const LocalTransform& local_tr) = 0;
	virtual BoundingBox getGlobalBoundingBox() const = 0;
	virtual void draw(const Viewer& viewer) const = 0;

protected:
	LocalTransform m_transform;
	// PolygonMode. For bounding boxes
	GLuint m_polygon_mode;
	std::vector<std::unique_ptr<BoundingBox>> m_boxes;
};



template<typename T>
class Renderable : public RenderObject
{
public:
	class Mesh {
	public:
		Mesh() : m_texture(nullptr) {
		}
		~Mesh() {
		}

		void createVao(const MeshData& mesh) {
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glBufferData(GL_ARRAY_BUFFER, sizeof(MeshData::VertexFormat) * mesh.data.size(), &mesh.data[0], GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexFormat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexFormat), (void*)(offsetof(MeshData::VertexFormat, MeshData::VertexFormat::color)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexFormat), (void*)(offsetof(MeshData::VertexFormat, MeshData::VertexFormat::normal)));
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexFormat), (void*)(offsetof(MeshData::VertexFormat, MeshData::VertexFormat::texcoords)));

			GLuint ibo;
			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indexes.size(), &mesh.indexes[0], GL_STATIC_DRAW);
		}

		void setTexture(const std::shared_ptr<Texture> texture) {
			m_texture = texture;
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

	private:
		GLuint m_vao;
		std::shared_ptr<Texture> m_texture;
	};

public:
	Renderable(const std::weak_ptr<Shader> shader) : m_shader(shader),
													 m_model_mat(glm::mat4(1.f)) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>();
		if (!m_render->load()) {
			std::cout << "Error when initialize primitive" << std::endl;
			exit(1);
		}

		this->init();
	}

	Renderable(const std::weak_ptr<Shader> shader, const std::string& filename) : m_shader(shader),
																				  m_model_mat(glm::mat4(1.f)) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>(filename);
		if (!m_render->load()) {
			std::cout << "Error when loading renderable : " << filename << std::endl;
			exit(1);
		}
		this->init();
	}

	~Renderable() {
	}

	void draw(const Viewer& viewer) const {
		if (auto shader_str = m_shader.lock()) {
			glPolygonMode(GL_FRONT_AND_BACK, m_polygon_mode);
			shader_str->bind();

			glUniformMatrix4fv(shader_str->getUniformLocation("modelview"), 1, false, glm::value_ptr(viewer.getViewMatrix() * m_model_mat));
			glUniformMatrix4fv(shader_str->getUniformLocation("projection"), 1, false, glm::value_ptr(Viewer::getProjectionMatrix()));

			for (unsigned int i = 0; i < m_meshes.size(); ++i) {
				m_meshes[i].draw(shader_str);
			}
		}
	}

	void setLocalTransform(const LocalTransform& local_tr) {
		m_transform = local_tr;
		m_model_mat = local_tr.getModelMatrix();

		m_boxes.clear();
		// Recalculate bounding boxes
		for (std::vector<MeshData>::iterator it = m_render->m_meshes.begin(); it != m_render->m_meshes.end(); ++it) {
			m_boxes.push_back(std::move(BoundingBox::create(*it, m_model_mat)));
		}
	}

	BoundingBox getGlobalBoundingBox() const {
		glm::vec3 min = m_boxes[0]->min;
		glm::vec3 max = m_boxes[0]->max;
		for (unsigned int i = 1; i < m_boxes.size(); ++i) {
			min = glm::min(min, m_boxes[i]->min);
			max = glm::max(max, m_boxes[i]->max);
		}

		BoundingBox global_box;
		global_box.min = min;
		global_box.max = max;

		return global_box;
	}

private:
	void init() {
		for (unsigned int i = 0; i < m_render->m_meshes.size(); ++i) {
			Mesh mesh;

			mesh.createVao(m_render->m_meshes[i]);
			GLuint material_index = m_render->m_meshes[i].material_index;
			if (!m_render->m_textures.empty() && material_index < m_render->m_textures.size()) {
				mesh.setTexture(m_render->m_textures[material_index]);
			}

			m_meshes.push_back(mesh);
			// Create bounding box
			m_boxes.push_back(std::move(BoundingBox::create(m_render->m_meshes[i], m_model_mat)));
		}
	}

private:
	// The model matrix relative to the renderable
	glm::mat4 m_model_mat;

	// Shader of the renderable
	std::weak_ptr<Shader> m_shader;

	// Mesh 
	std::vector<Mesh> m_meshes;

	std::unique_ptr<T> m_render;

};