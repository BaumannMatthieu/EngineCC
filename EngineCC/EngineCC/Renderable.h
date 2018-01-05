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
	const LocalTransform& getLocalTransform() const {
		return m_transform;
	}

	virtual const Primitive& getPrimitive() const = 0;

	virtual void draw(const Viewer& viewer) const = 0;

protected:
	LocalTransform m_transform;
};

template<typename T>
class Renderable : public RenderObject
{
public:

	// Renderable constructor for a "well-defined" primitive object such as Cubes, Planes, Spheres...
	Renderable(const std::weak_ptr<Shader> shader) : m_shader(shader),
													 m_model_mat(glm::mat4(1.f)) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>();
		init();
	}

	// Renderable constructor for a non "well-defined" primitive object such as a line.
	// In the case of lines we ask for the 2 points and ModelMatrix is equal to Identity.
	Renderable(const std::weak_ptr<Shader> shader, const std::vector<glm::vec3>& points, const std::vector<glm::vec4>& colors) : m_shader(shader),
																																m_model_mat(glm::mat4(1.f)) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>(points, colors);
		init();
	}

	// Renderable construction for a model 3D object a.k.a. Renderable<Model>
	Renderable(const std::weak_ptr<Shader> shader, const std::string& filename) : m_shader(shader),
																				  m_model_mat(glm::mat4(1.f)) {
		// Mesh contains all the data of the renderable (vertex, colors, normals, ...)
		m_render = std::make_unique<T>(filename);
		init();
	}

	~Renderable() {
	}

	void setTexture(const std::string& texture_path) {
		m_render->setTexture(texture_path);
	}

private:
	void init() {
		m_polygon_mode = GL_FILL;
	}
public:

	void setPolygonMode(GLuint polygon_mode) {
		m_polygon_mode = polygon_mode;
	}

	const glm::mat4& getModelMatrix() const {
		return m_model_mat;
	}

	void draw(const Viewer& viewer) const {
		if (auto shader_str = m_shader.lock()) {
			glPolygonMode(GL_FRONT_AND_BACK, m_polygon_mode);
			shader_str->bind();
			glUniformMatrix4fv(shader_str->getUniformLocation("model"), 1, false, glm::value_ptr(m_model_mat));
			glUniformMatrix4fv(shader_str->getUniformLocation("view"), 1, false, glm::value_ptr(viewer.getViewMatrix()));
			glUniformMatrix4fv(shader_str->getUniformLocation("modelview"), 1, false, glm::value_ptr(viewer.getViewMatrix() * m_model_mat));
			glUniformMatrix4fv(shader_str->getUniformLocation("projection"), 1, false, glm::value_ptr(Viewer::getProjectionMatrix()));
		}

		m_render->draw(m_shader);
	}

	void setLocalTransform(const LocalTransform& local_tr) {
		m_transform = local_tr;
		m_model_mat = local_tr.getModelMatrix();
	}

	void setColor(const glm::vec4& color) {
		m_render->setColor(color);
	}

	const Primitive& getPrimitive() const {
		return dynamic_cast<Primitive&>(*m_render);
	}

private:

	// The model matrix relative to the renderable
	glm::mat4 m_model_mat;
	// Shader of the renderable
	std::weak_ptr<Shader> m_shader;

	std::unique_ptr<T> m_render;

	// PolygonMode. For bounding boxes
	GLuint m_polygon_mode;
};