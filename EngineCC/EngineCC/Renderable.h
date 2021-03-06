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

#include "BoundingBox.h"
#include "LocalTransform.h"
#include "Primitive.h"

class RenderObject {
public:
	RenderObject() {
	}
	virtual ~RenderObject() {
	}

	virtual void setLocalTransform(const LocalTransform& local_tr) = 0;
	virtual const LocalTransform& getLocalTransform() const = 0;

	virtual void setTexcoordsFactor(const glm::vec3& texcoords_factor) = 0;

	virtual const Primitive& getPrimitive() const = 0;

	virtual void draw(const Viewer& viewer) const = 0;
	
	virtual void setInvisible(bool visible=false) = 0;
};

template<typename T>
class Renderable : public RenderObject
{
public:

	// Renderable constructor for a "well-defined" primitive object such as Cubes, Planes, Spheres...
	Renderable(const std::weak_ptr<Shader> shader) : m_shader(shader) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>();
		init();
	}

	// Renderable constructor for a non "well-defined" primitive object such as a line.
	// In the case of lines we ask for the 2 points and ModelMatrix is equal to Identity.
	Renderable(const std::weak_ptr<Shader> shader, const std::vector<glm::vec3>& points, const std::vector<glm::vec4>& colors) : m_shader(shader) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>(points, colors);
		init();
	}

	// Renderable construction for a model 3D object a.k.a. Renderable<Model>
	// TODO : Create a manager of textures map<string, std::shared_ptr<Texture>> instead of creating two times the same
	// TODO : textures for two instances of the same model.
	Renderable(const std::weak_ptr<Shader> shader, const std::string& filename) : m_shader(shader) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>(filename);
		init();
	}

	~Renderable() {
	}

	void setTexture(const std::string& texture_path) {
		m_render->setTexture(texture_path);
	}

public:

	void setPolygonMode(GLuint polygon_mode) {
		m_polygon_mode = polygon_mode;
	}

	const glm::mat4& getModelMatrix() const {
		return m_model_mat;
	}

	void draw(const Viewer& viewer) const {
		if (!m_visible)
			return;

		if (auto shader_str = m_shader.lock()) {
			glPolygonMode(GL_FRONT_AND_BACK, m_polygon_mode);
			shader_str->bind();
			glUniformMatrix4fv(shader_str->getUniformLocation("model"), 1, false, glm::value_ptr(m_model_mat));
			glUniformMatrix4fv(shader_str->getUniformLocation("view"), 1, false, glm::value_ptr(viewer.getViewMatrix()));
			glUniformMatrix4fv(shader_str->getUniformLocation("modelview"), 1, false, glm::value_ptr(viewer.getViewMatrix() * m_model_mat));
			glUniformMatrix4fv(shader_str->getUniformLocation("projection"), 1, false, glm::value_ptr(Viewer::getProjectionMatrix()));

			glUniform3fv(shader_str->getUniformLocation("tex_factor"), 1, glm::value_ptr(m_texcoords_factor));
		}

		m_render->draw(m_shader);
	}

	void setLocalTransform(const LocalTransform& local_tr) {
		m_transform = local_tr;
		m_model_mat = local_tr.getModelMatrix();
	}

	void setInvisible(bool visible = false) {
		m_visible = visible;
	}

	const LocalTransform& getLocalTransform() const {
		return m_transform;
	}

	virtual void setTexcoordsFactor(const glm::vec3& texcoords_factor) {
		m_texcoords_factor = texcoords_factor;
	}

	void setColor(const glm::vec4& color) {
		m_render->setColor(color);
	}

	const Primitive& getPrimitive() const {
		return dynamic_cast<Primitive&>(*m_render);
	}

private:
	void init() {
		m_model_mat = glm::mat4(1.f);
		m_polygon_mode = GL_FILL;
		m_texcoords_factor = glm::vec3(1);
		m_visible = true;
	}

private:
	LocalTransform m_transform;
	glm::vec3 m_texcoords_factor;
	// The model matrix relative to the renderable
	glm::mat4 m_model_mat;
	// Shader of the renderable
	std::weak_ptr<Shader> m_shader;

	std::unique_ptr<T> m_render;

	// PolygonMode. For bounding boxes
	GLuint m_polygon_mode;

	bool m_visible;
};