#include "BoundingBox.h"

#include "Mesh.h"

BoundingBox BoundingBox::create(const std::vector<Mesh::VertexFormat>& vertices, const glm::mat4& transform_mat) {
	BoundingBox box;
	glm::vec3 world_space_vertice = glm::vec3(transform_mat * glm::vec4(vertices[0].point, 1));
	box.min = world_space_vertice;
	box.max = world_space_vertice;
	for (unsigned int i = 1; i < vertices.size(); ++i) {
		glm::vec3 world_space_vertice = glm::vec3(transform_mat * glm::vec4(vertices[i].point, 1));

		box.max = glm::max(box.max, world_space_vertice);
		box.min = glm::min(box.min, world_space_vertice);
	}

	return box;
}