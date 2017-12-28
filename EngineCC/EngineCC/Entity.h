#pragma once
#include <memory>

#include "Renderable.h"
#include "Cube.h"
#include "Manager.h"
#include "Shader.h"

class Entity
{
public:
	Entity() {
	}
	Entity(const std::string& name, std::unique_ptr<RenderObject> object) : m_name(name),
																			m_object(std::move(object)) {
		this->defineBoundingBox();
	}
	~Entity() {
	}

	void setRenderObject(std::unique_ptr<RenderObject> object) {
		m_object = std::move(object);
		this->defineBoundingBox();
	}

	const std::string& getName() const {
		return m_name;
	}

	const std::unique_ptr<RenderObject>& getRenderObject() const {
		return m_object;
	}

	const std::vector<std::unique_ptr<Renderable<Cube>>>& getBoxesObject() const {
		return m_boxes;
	}

	const std::unique_ptr<Renderable<Cube>>& getSelectionBoxObject() const {
		return m_selection_box;
	}

	// Set the transform for the object renderable and its boxes.
	// This function is always called for animated objects so that
	// its bounding boxes are always recomputed and fit to the movement
	void setLocalTransform(const LocalTransform& transform) {
		m_object->setLocalTransform(transform);

		// Scale to fit the render object
		this->setBoxLocalTransform(m_object->getLocalTransform());
	}

	const LocalTransform& getLocalTransform() const {
		return m_object->getLocalTransform();
	}

	bool intersect(const glm::vec3& I) const {
		const BoundingBox& box = m_object->getGlobalBoundingBox();

		if (I.x >= box.min.x && I.x <= box.max.x &&
			I.y >= box.min.y && I.y <= box.max.y &&
			I.z >= box.min.z && I.z <= box.max.z) {
			return true;
		}
		return false;
	}

	void update() {
		if (m_object->isAnimated()) {
			const LocalTransform& transform = m_object->getLocalTransform();
			this->setLocalTransform(transform);
		}
	}

private:
	void setBoxLocalTransform(const LocalTransform& objectTransform) {	
		const std::vector<BoundingBox>& boxes = m_object->getBoundingBoxes();

		for (unsigned int i = 0; i < m_boxes.size(); ++i) {
			LocalTransform transform;
			transform.setScale(boxes[i].max - boxes[i].min);
			transform.setTranslation((boxes[i].max + boxes[i].min)*0.5f);

			m_boxes[i]->setLocalTransform(transform);
		}

		// Global selection box
		const BoundingBox& global_box = m_object->getGlobalBoundingBox();
		LocalTransform transform;
		transform.setScale(global_box.max - global_box.min);
		transform.setTranslation((global_box.max + global_box.min)*0.5f);

		m_selection_box->setLocalTransform(transform);
	}

	void defineBoundingBox() {
		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
		m_boxes.clear();
		// Definition of the bounding boxes for each mesh of the model
		for (unsigned int i = 0; i < m_object->getBoundingBoxes().size(); ++i) {
			std::unique_ptr<Renderable<Cube>> box = std::make_unique<Renderable<Cube>>(shaders.get("simple"));
			box->setPolygonMode(GL_LINE);
			m_boxes.push_back(std::move(box));
		}

		// Definition of the global selection box
		m_selection_box = std::make_unique<Renderable<Cube>>(shaders.get("simple"));
		m_selection_box->setPolygonMode(GL_LINE);
		
		// Scale to fit the render object
		this->setBoxLocalTransform(m_object->getLocalTransform());
	}

private:
	std::unique_ptr<RenderObject> m_object;
	std::vector<std::unique_ptr<Renderable<Cube>>> m_boxes;
	std::unique_ptr<Renderable<Cube>> m_selection_box;

	std::string m_name;
};

