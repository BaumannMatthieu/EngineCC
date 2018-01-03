#pragma once

#include <string>
#include <map>
#include <queue>
#include <chrono>
#include <ctime>

/// Assimp includes
// C++ importer interface
#include <assimp/Importer.hpp>      
// Output data structure
#include <assimp/scene.h>           
// Post processing flags
#include <assimp/postprocess.h>   

#include "Primitive.h"
#include "Mesh.h"
#include "BoundingBox.h"

class Model : public Primitive {
public:
	Model(const std::string& filename) : m_filename(filename) {
		this->load();
	}
	Model() : m_scene(NULL),
			  m_animated(false), 
			  m_start(std::chrono::high_resolution_clock::now()) {
	}
	~Model() {
	}

	bool isAnimated() const {
		return m_animated;
	}

	inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) const
	{
		glm::mat4 to;

		to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
		to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
		to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
		to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;

		return to;
	}

	virtual std::vector<glm::vec3> getVertices() const {
		std::vector<glm::vec3> vertices;
		//glm::mat4& glm_globalRootTransform = aiMatrix4x4ToGlm(m_scene->mRootNode->mTransformation);
		glm::mat4& glm_globalRootTransform = aiMatrix4x4ToGlm(m_scene->mRootNode->mTransformation.Inverse());
		for (int i = 0; i < m_meshes.size(); ++i) {
			std::vector<glm::vec3>& vertices_current_mesh = m_meshes[i]->getVertices();
			// A loaded model contains a m_globalRootTransform that must be applied to the vertices before computing the convex hull collision shape
			for (int j = 0; j < vertices_current_mesh.size(); ++j) {
				vertices_current_mesh[j] = glm::vec3(glm_globalRootTransform * glm::vec4(vertices_current_mesh[j], 1.f));
			}

			vertices.insert(vertices.end(), vertices_current_mesh.begin(), vertices_current_mesh.end());
		}
		return vertices;
	}

	void computeBoundingBoxes(const glm::mat4& model_mat, std::vector<BoundingBox>& bounding_boxes) {
		bounding_boxes.clear();
		if (m_animated) {
			for (std::map<int, std::vector<Mesh::VertexFormat>>::iterator it = m_bones_vertices.begin(); it != m_bones_vertices.end(); ++it) {
				bounding_boxes.push_back(BoundingBox::create(it->second, model_mat * aiMatrix4x4ToGlm(m_transforms[it->first])));
			}
		}
		else {
			Primitive::computeBoundingBoxes(model_mat, bounding_boxes);
		}
	}

	void load() {
		m_scene = m_Importer.ReadFile(m_filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

		if (m_scene) {
			this->loadVerticesData();
			std::cout << "kjkj" << std::endl;
			m_globalRootTransform = m_scene->mRootNode->mTransformation.Inverse();

			m_animated = m_scene->HasAnimations();
			const std::vector<std::shared_ptr<Texture>>& materials = this->loadMaterials();

			// Give to each mesh its texture 
			for (unsigned int i = 0; i < m_meshes.size(); ++i) {
				Mesh& mesh = dynamic_cast<Mesh&>(*(m_meshes[i]));
				GLuint material_index = mesh.m_material_index;
				if (!materials.empty() && material_index < materials.size())
					mesh.m_texture = materials[material_index];
				else
					std::cout << "material index out of range among the vector of textures load" << std::endl;
			}

			this->writeBuffers();
		}
		else {
			printf("Error parsing '%s': '%s'\n", m_filename.c_str(), m_Importer.GetErrorString());
			exit(1);
		}
	}

	void draw(const std::weak_ptr<Shader> shader) {
		updateBonesTransforms(m_globalRootTransform,
							  m_scene->mRootNode,
							  m_scene->mRootNode->mTransformation);

		if (auto program = shader.lock()) {
			glUniformMatrix4fv(program->getUniformLocation("bonesTransform"), m_transforms.size(), GL_TRUE, reinterpret_cast<const GLfloat*>(m_transforms.data()));
			glUniform1i(program->getUniformLocation("animated"), static_cast<int>(m_animated));
		}
		Primitive::draw(shader);
	}

private:
	const std::vector<std::shared_ptr<Texture>> loadMaterials() {
		std::vector<std::shared_ptr<Texture>> materials(m_scene->mNumMaterials);
		// Retrieve textures for the model
		for (unsigned int i = 0; i < m_scene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = m_scene->mMaterials[i];

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;  
				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string filename = "Content/";
					filename += path.data;
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

	void loadBones() {
		// Assign all the bones with an id. What we will give to the vertex shader is :
		// - an array of glm::mat4 describing the matrix transformation of each bone
		// - a set of id referring to the bones that has an importance on the vertex + their corresponding weights of importance
		unsigned num_bones = 0;
		for (unsigned int i = 0; i < m_scene->mNumMeshes; ++i) {
			for (unsigned int k = 0; k < m_scene->mMeshes[i]->mNumBones; ++k) {
				const aiBone* current_bone = m_scene->mMeshes[i]->mBones[k];
				const std::string& name = current_bone->mName.C_Str();

				// Find the corresponding bones by their name
				// And collect them in a vector. These will be used for all the updates of the transformation matrices.
				// So we do this only once when loading the model.
				if (m_bones_map.find(name) == m_bones_map.end()) {
					m_bones_map[name] = num_bones;

					m_offset_bones.push_back(current_bone->mOffsetMatrix);
					num_bones++;
				}
			}
		}
	}

	void computeRotationInterp(double elapsed_time_sec, double duration_anim_sec, aiNodeAnim* node_animation, aiQuaternion& rotation) {
		double current_key = (elapsed_time_sec / duration_anim_sec) * node_animation->mNumRotationKeys;
		unsigned int index = static_cast<unsigned int>(current_key);
		double factor_interp = current_key - index;
		
		unsigned int prev = index % node_animation->mNumRotationKeys;
		unsigned int next = (index + 1) % node_animation->mNumRotationKeys;

		const aiQuaternion& start = node_animation->mRotationKeys[prev].mValue;
		const aiQuaternion& end = node_animation->mRotationKeys[next].mValue;
		
		aiQuaternion::Interpolate(rotation, start, end, static_cast<float>(factor_interp));
	}

	void computeTranslationInterp(double elapsed_time_sec, double duration_anim_sec, aiNodeAnim* node_animation, aiVector3D& translation) {
		double current_key = (elapsed_time_sec / duration_anim_sec) * node_animation->mNumPositionKeys;
		unsigned int index = static_cast<unsigned int>(current_key);
		float factor_interp = current_key - index;

		unsigned int prev = index % node_animation->mNumPositionKeys;
		unsigned int next = (index + 1) % node_animation->mNumPositionKeys;

		aiVector3D& start = node_animation->mPositionKeys[prev].mValue;
		aiVector3D& end = node_animation->mPositionKeys[next].mValue;

		translation = end * factor_interp + start * (1 - factor_interp);
	}

	void computeScalingInterp(double elapsed_time_sec, double duration_anim_sec, aiNodeAnim* node_animation, aiVector3D& scaling) {
		double current_key = (elapsed_time_sec / duration_anim_sec) * node_animation->mNumScalingKeys;
		unsigned int index = static_cast<unsigned int>(current_key);
		float factor_interp = current_key - index;

		unsigned int prev = index % node_animation->mNumScalingKeys;
		unsigned int next = (index + 1) % node_animation->mNumScalingKeys;

		aiVector3D& start = node_animation->mScalingKeys[prev].mValue;
		aiVector3D& end = node_animation->mScalingKeys[next].mValue;

		scaling = end * factor_interp + start * (1 - factor_interp);
	}

	void updateBonesTransforms(const aiMatrix4x4& global_inv, aiNode* node, const aiMatrix4x4& parentTransform) {
		// Find the corresponding bone by searching through the map : name -> int
		// for the id first and then for the Bone in the indexed vector of Bones
		if (node) {
			const std::string& name = node->mName.C_Str();
			aiMatrix4x4 transform = node->mTransformation;

			if (m_bones_map.find(name) != m_bones_map.end()) {
				int index = m_bones_map[name];

				aiMatrix4x4 translation_mat, rotation_mat, scaling_mat;
				double duration_ticks = m_scene->mAnimations[0]->mDuration;
				double num_ticks_per_sec = m_scene->mAnimations[0]->mTicksPerSecond;
				
				double duration_anim_sec = duration_ticks / num_ticks_per_sec;

				std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
				double elapsed_time_sec = std::chrono::duration_cast< std::chrono::duration<double> >(now - m_start).count();

				for (unsigned int i = 0; i < m_scene->mAnimations[0]->mNumChannels; ++i) {
					aiNodeAnim* node_animation = m_scene->mAnimations[0]->mChannels[i];
					if (node_animation->mNodeName.C_Str() == name) {
						aiVector3D scaling_vec;
						this->computeScalingInterp(elapsed_time_sec, duration_anim_sec, node_animation, scaling_vec);
						aiMatrix4x4::Scaling(scaling_vec, scaling_mat);
						
						aiQuaternion rotation_quat;
						this->computeRotationInterp(elapsed_time_sec, duration_anim_sec, node_animation, rotation_quat);
						rotation_mat = aiMatrix4x4(rotation_quat.GetMatrix());
						
						aiVector3D translation_vec;
						this->computeTranslationInterp(elapsed_time_sec, duration_anim_sec, node_animation, translation_vec);
						aiMatrix4x4::Translation(translation_vec, translation_mat);
						
						transform = translation_mat * rotation_mat * scaling_mat;

						break;
					}
				}
				
				m_transforms[index] = global_inv * parentTransform * transform * m_offset_bones[index];
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++i) {
				updateBonesTransforms(global_inv, node->mChildren[i], parentTransform * transform);
			}
		}
	}

	void loadVerticesData() {
		this->loadBones();

		m_transforms.clear();
		m_transforms.resize(m_bones_map.size(), aiMatrix4x4());

		aiVector3D zero(0.f, 0.f, 0.f);

		//Vector indexed by ID vertices
		std::vector<std::multimap<float, int>> weights;
		unsigned int starting_index = 0;
		for (unsigned int i = 0; i < m_scene->mNumMeshes; ++i) {
			const aiMesh* mesh = m_scene->mMeshes[i];
			std::unique_ptr<Mesh> current_mesh = std::make_unique<Mesh>();

			// Retrieve the data of the model which will be given to the VBO
			// Test if the mesh has been correctly triangulated
			assert(mesh->mNumVertices == mesh->mNumFaces * 3);

			unsigned int num_vertices = mesh->mNumVertices;
			for (unsigned int j = 0; j < num_vertices; ++j) {
				// A mesh surely has vertices but not always normals nor texcoords
				const aiVector3D* vertex = &(mesh->mVertices[j]);
				const aiVector3D* normal = mesh->HasNormals() ? &(mesh->mNormals[j]) : &zero;
				const aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &zero;

				current_mesh->m_vertices.push_back(Mesh::VertexFormat(glm::vec3(vertex->x, vertex->y, vertex->z),
					glm::vec4(1.f),
					glm::vec3(normal->x, normal->y, normal->z),
					glm::vec2(texcoord->x, texcoord->y)));
			}

			weights.resize(weights.size() + num_vertices);
			
			for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
				aiString name = mesh->mBones[j]->mName;
				int bone_index = m_bones_map[name.C_Str()];

				for (unsigned int k = 0; k < mesh->mBones[j]->mNumWeights; ++k) {
					unsigned int vertex_local_id = mesh->mBones[j]->mWeights[k].mVertexId;
					unsigned int vertex_global_id = vertex_local_id + starting_index;
					float weight = mesh->mBones[j]->mWeights[k].mWeight;
					
					weights[vertex_global_id].insert(std::pair<float, int>(weight, bone_index));

					Mesh::VertexFormat& vertex = current_mesh->m_vertices[vertex_local_id];
					if (m_bones_vertices.find(bone_index) == m_bones_vertices.end()) {
						std::vector<Mesh::VertexFormat> vertices(1, vertex);
						m_bones_vertices.insert(std::pair<int, std::vector<Mesh::VertexFormat>>(bone_index, vertices));
					}
					else {
						std::vector<Mesh::VertexFormat>& vertices = m_bones_vertices[bone_index];
						vertices.push_back(vertex);
					}
				}
			}

			starting_index += num_vertices;

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

		starting_index = 0;
		for (unsigned int i = 0; i < m_meshes.size(); ++i) {
			unsigned int num_vertices = m_meshes[i]->m_vertices.size();
			for (unsigned int j = 0; j < num_vertices; ++j) {
				glm::vec4 vertex_weights = glm::vec4(0);
				glm::ivec4 bone_id = glm::ivec4(0);

				unsigned int index = 0;
				unsigned int vertexID = starting_index + j;
				for (std::multimap<float, int>::reverse_iterator it = weights[vertexID].rbegin(); it != weights[vertexID].rend(); it++) {
					if (index == 4)
						break;
					vertex_weights[index] = it->first;
					bone_id[index] = it->second;

					index++;
				}

				m_meshes[i]->m_vertices[j].weights = vertex_weights;
				m_meshes[i]->m_vertices[j].bones_indexes = bone_id;

			}
			starting_index += num_vertices;
		}
	}
private:
	// Assimp model importer
	Assimp::Importer m_Importer;
public:
	// Name of the model to load
	std::string m_filename;

	const aiScene* m_scene;
	aiMatrix4x4 m_globalRootTransform;

	// Bones infomation
	// A map between bones names and global indexes
	std::map<std::string, int> m_bones_map;
	// Offset matrices of all the bones indexed by the global bone index
	std::vector<aiMatrix4x4> m_offset_bones;
	// Final transform of each bone to send to GPU indexed by the global bone index
	std::vector<aiMatrix4x4> m_transforms;

	// When animating a model
	std::chrono::high_resolution_clock::time_point m_start;
	bool m_animated;

	// Information of which vertices can be impacted by a bone
	std::map<int, std::vector<Mesh::VertexFormat>> m_bones_vertices;
};
