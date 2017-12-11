#pragma once

#include <string>

/// Assimp includes
// C++ importer interface
#include <assimp/Importer.hpp>      
// Output data structure
#include <assimp/scene.h>           
// Post processing flags
#include <assimp/postprocess.h>   

#include "Primitive.h"

class Model : public Primitive {
public:
	Model(const std::string& filename) : m_filename(filename) {
	}
	~Model() {
	}

	bool load() {
		Assimp::Importer Importer;

		const aiScene* scene = Importer.ReadFile(m_filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

		if (scene) {
			loadVerticesData(scene);
			const std::vector<std::shared_ptr<Texture>>& materials = loadMaterials(scene);
			assert(m_meshes.size() == materials.size());

			// Give to each mesh its texture 
			for (unsigned int i = 0; i < m_meshes.size(); ++i) {
				GLuint material_index = m_meshes[i]->m_material_index;
				if (!materials.empty() && material_index < materials.size())
					m_meshes[i]->m_texture = materials[material_index];
				else
					std::cout << "material index out of range among the vector of textures load" << std::endl;
			}

			return true;
		}
		else {
			printf("Error parsing '%s': '%s'\n", m_filename.c_str(), Importer.GetErrorString());
		}

		return false;
	}
private:
	const std::vector<std::shared_ptr<Texture>> loadMaterials(const aiScene* scene) {
		std::vector<std::shared_ptr<Texture>> materials(scene->mNumMaterials);
		// Retrieve textures for the model
		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = scene->mMaterials[i];

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;
				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string filename = path.data;
					std::cout << filename << std::endl;
					materials[i] = std::make_shared<Texture>(filename);

					if (!materials[i]->load()) {
						std::cout << "Error loading texture " << filename << std::endl;
						materials[i] = std::make_shared<Texture>("test.png");
					}
				}
			}
		}

		return materials;
	}

	void loadVerticesData(const aiScene* scene) {
		aiVector3D zero(0.f, 0.f, 0.f);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh* mesh = scene->mMeshes[i];

			std::unique_ptr<Mesh> current_mesh = std::make_unique<Mesh>();

			std::vector<glm::vec3> vertices;
			// Retrieve the data of the model which will be given to the VBO
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
				const aiVector3D* point = &(mesh->mVertices[j]);
				const aiVector3D* normal = &(mesh->mNormals[j]);
				if (!normal) {
					normal = &zero;
				}
				const aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &zero;

				vertices.push_back(glm::vec3(point->x, point->y, point->z));

				current_mesh->m_vertices.push_back(Mesh::VertexFormat(vertices.back(),
					glm::vec4(1.f),
					glm::vec3(normal->x, normal->y, normal->z),
					glm::vec2(texcoord->x, texcoord->y)));

				std::cout << texcoord->x << std::endl;
			}

			// Retrieve the vector of indexes which will be given to the IBO
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				const aiFace& face = mesh->mFaces[j];
				assert(face.mNumIndices == 3);

				current_mesh->m_indexes.push_back(face.mIndices[0]);
				current_mesh->m_indexes.push_back(face.mIndices[1]);
				current_mesh->m_indexes.push_back(face.mIndices[2]);
			}
			// Retrieve the index of the material the mesh is referred to
			current_mesh->m_material_index = mesh->mMaterialIndex;

			m_meshes.push_back(std::move(current_mesh));
		}

		// Update the mesh position so that it is centered in the scene's origin
		glm::vec3 vertice_max = m_meshes[0]->m_vertices[0].point;
		glm::vec3 vertice_min = m_meshes[0]->m_vertices[0].point;

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			for (unsigned int j = 0; j < m_meshes[i]->m_vertices.size(); ++j) {
				vertice_max = glm::max(vertice_max, m_meshes[i]->m_vertices[j].point);
				vertice_min = glm::min(vertice_min, m_meshes[i]->m_vertices[j].point);
			}
		}
		glm::vec3 offset = 0.5f*(vertice_max + vertice_min);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			for (unsigned int j = 0; j < m_meshes[i]->m_vertices.size(); ++j) {
				m_meshes[i]->m_vertices[j].point -= offset;
			}
		}
	}

public:
	// Name of the model to load
	std::string m_filename;
};
