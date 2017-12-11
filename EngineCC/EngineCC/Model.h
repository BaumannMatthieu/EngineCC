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
		bool res = true;
		Assimp::Importer Importer;

		const aiScene* scene = Importer.ReadFile(m_filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

		if (scene) {
			loadVerticesData(scene);
			res = res & loadMaterials(scene);
			assert(m_meshes.size() == m_textures.size());
			return res;
		}
		else {
			printf("Error parsing '%s': '%s'\n", m_filename.c_str(), Importer.GetErrorString());
		}

		return false;
	}
private:
	bool loadMaterials(const aiScene* scene) {
		bool res = true;
		// Retrieve textures for the model
		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = scene->mMaterials[i];

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;
				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string filename = path.data;
					std::cout << filename << std::endl;
					std::shared_ptr<Texture> current_texture = std::make_shared<Texture>(filename);

					if (!current_texture->load()) {
						std::cout << "Error loading texture " << filename << std::endl;
						current_texture = std::make_shared<Texture>("test.png");
						res = false;
					}
					m_textures.push_back(current_texture);
				}
			}
		}

		return res;
	}
	void loadVerticesData(const aiScene* scene) {
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh* mesh = scene->mMeshes[i];
			MeshData current_mesh;
			aiVector3D zero(0.f, 0.f, 0.f);

			std::vector<glm::vec3> vertices;
			// Retrieve the data of the model which will be given to the VBO
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
				const aiVector3D* vertice = &(mesh->mVertices[j]);
				const aiVector3D* normal = &(mesh->mNormals[j]);
				if (!normal) {
					normal = &zero;
				}
				const aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &zero;

				vertices.push_back(glm::vec3(vertice->x, vertice->y, vertice->z));

				current_mesh.data.push_back(MeshData::VertexFormat(vertices.back(),
					glm::vec4(1.f),
					glm::vec3(normal->x, normal->y, normal->z),
					glm::vec2(texcoord->x, texcoord->y)));

				std::cout << texcoord->x << std::endl;
			}

			// Retrieve the vector of indexes which will be given to the IBO
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				const aiFace& face = mesh->mFaces[j];
				assert(face.mNumIndices == 3);

				current_mesh.indexes.push_back(face.mIndices[0]);
				current_mesh.indexes.push_back(face.mIndices[1]);
				current_mesh.indexes.push_back(face.mIndices[2]);
			}
			// Retrieve the index of the material the mesh is referred to
			current_mesh.material_index = mesh->mMaterialIndex;

			m_meshes.push_back(current_mesh);
		}


		// Update the mesh position so that it is centered in the scene's origin
		glm::vec3 vertice_max = m_meshes[0].data[0].vertice;
		glm::vec3 vertice_min = m_meshes[0].data[0].vertice;

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			for (unsigned int j = 0; j < m_meshes[i].data.size(); ++j) {
				vertice_max = glm::max(vertice_max, m_meshes[i].data[j].vertice);
				vertice_min = glm::min(vertice_min, m_meshes[i].data[j].vertice);
			}
		}
		glm::vec3 offset = 0.5f*(vertice_max + vertice_min);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			for (unsigned int j = 0; j < m_meshes[i].data.size(); ++j) {
				m_meshes[i].data[j].vertice -= offset;
			}
		}
	}


public:
	// Name of the model to load
	std::string m_filename;
};
