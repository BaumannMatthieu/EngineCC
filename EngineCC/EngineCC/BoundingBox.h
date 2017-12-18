#pragma once
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "Mesh.h"

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;

	static BoundingBox create(const std::vector<Mesh::VertexFormat>& vertices, const glm::mat4& transform_mat);
};