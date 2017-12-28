#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct BoundingBox;
struct Mesh;

class Primitive {
public:
	Primitive();

	virtual ~Primitive();

	virtual void draw(const std::weak_ptr<Shader> shader) const;

	void setColor(const glm::vec4& color);

	virtual void computeBoundingBoxes(const glm::mat4& model_mat, std::vector<BoundingBox>& bounding_boxes);

	virtual bool isAnimated() const;

protected:
	virtual void load() = 0;
	void writeBuffers();

public:
	// A model is defined by its data vertices
	std::vector<std::unique_ptr<Mesh>> m_meshes;
};
