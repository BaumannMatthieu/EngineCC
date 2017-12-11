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
	Entity(std::unique_ptr<RenderObject> object) : m_object(std::move(object)) {
		this->defineBoundingBox();
	}
	~Entity() {
	}

	void setRenderObject(std::unique_ptr<RenderObject> object) {
		m_object = std::move(object);
		this->defineBoundingBox();
	}

	const std::unique_ptr<RenderObject>& getRenderObject() const {
		return m_object;
	}

	const std::unique_ptr<Renderable<Cube>>& getBoxObject() const {
		return m_box;
	}

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

private:
	void setBoxLocalTransform(const LocalTransform& objectTransform) {
		const BoundingBox& box = m_object->getGlobalBoundingBox();
	
		LocalTransform transform;
		transform.setScale(box.max - box.min);
		transform.setTranslation((box.max + box.min)*0.5f);
		m_box->setLocalTransform(transform);
	}

	void defineBoundingBox() {
		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

		m_box = std::make_unique<Renderable<Cube>>(shaders.get("simple"));
		m_box->setPolygonMode(GL_LINE);
		// Scale to fit the render object
		this->setBoxLocalTransform(m_object->getLocalTransform());
	}

private:
	std::unique_ptr<RenderObject> m_object;
	std::unique_ptr<Renderable<Cube>> m_box;
};

