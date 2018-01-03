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
	virtual BoundingBox getGlobalBoundingBox() const = 0;
	virtual void draw(const Viewer& viewer) const = 0;
	virtual bool isAnimated() const = 0;
	virtual const Primitive& getPrimitive() const = 0;

	const std::vector<BoundingBox>& getBoundingBoxes() const {
		return m_boxes;
	}
	const LocalTransform& getLocalTransform() const {
		return m_transform;
	}
protected:
	std::vector<BoundingBox> m_boxes;
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

private:
	void init() {
		m_polygon_mode = GL_FILL;
		m_boxes = std::vector<BoundingBox>();
		m_render->computeBoundingBoxes(m_model_mat, m_boxes);
	}
public:
	bool isAnimated() const {
		return m_render->isAnimated();
	}

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


		// Recalculate bounding boxes
		//m_render->computeBoundingBoxes(m_model_mat, m_boxes);
		m_render->computeBoundingBoxes(m_model_mat, m_boxes);
	}

	BoundingBox getGlobalBoundingBox() const {
		glm::vec3 min = m_boxes[0].min;
		glm::vec3 max = m_boxes[0].max;
		for (unsigned int i = 1; i < m_boxes.size(); ++i) {
			min = glm::min(min, m_boxes[i].min);
			max = glm::max(max, m_boxes[i].max);
		}

		BoundingBox global_box;
		global_box.min = min;
		global_box.max = max;

		return global_box;
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