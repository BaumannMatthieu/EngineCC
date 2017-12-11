#pragma once
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"

struct MeshData {
	struct VertexFormat {
		VertexFormat(const glm::vec3& _vertice,
			const glm::vec4& _color = glm::vec4(0.f),
			const glm::vec3& _normal = glm::vec3(0.f),
			const glm::vec2& _texcoords = glm::vec2(0.f)) : vertice(_vertice),
			color(_color),
			normal(_normal),
			texcoords(_texcoords) {
		}

		glm::vec3 vertice;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 texcoords;
	};
	MeshData() {}
	~MeshData() {}

	std::vector<VertexFormat> data;
	std::vector<GLuint> indexes;

	GLuint material_index;
};

class Primitive {
public:
	Primitive() {
	}
	virtual ~Primitive() {
	}

	virtual bool load() = 0;
public:
	// A model is defined by its data vertices
	std::vector<MeshData> m_meshes;
	// And its materials. Two mesh of a model can refer to the same texture
	// Thus textures can be shared among several meshes. 
	std::vector<std::shared_ptr<Texture>> m_textures;
};
