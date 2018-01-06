#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct Drawable;

class Primitive {
public:
	Primitive();

	virtual ~Primitive();

	virtual void draw(const std::weak_ptr<Shader> shader) const;

	void setColor(const glm::vec4& color);

	virtual std::vector<glm::vec3> getVertices() const;


	virtual void setTexture(const std::string& filepath) = 0;

protected:
	void writeBuffers();

public:
	// A model is defined by its data vertices
	std::vector<std::unique_ptr<Drawable>> m_meshes;
};
