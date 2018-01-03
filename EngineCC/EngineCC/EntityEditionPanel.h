#pragma once
#include <memory>
#include <vector>
#include <filesystem>
#include <sstream>
#include <stdio.h>
#include <unordered_set>

// Include Imgui
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "Singleton.h"
#include "World.h"
#include "GameProgram.h"

#include "PhysicSystem.h"
#include "Components.h"

#include "Renderable.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

class HierarchyEntity {
public:
	class Node {
	public:
		Node(const std::string& folder_name = "scene") : m_folder_name(folder_name) {
		}
		~Node() {
		}

		void addChild(std::unique_ptr<Node> child) {
			m_children.push_back(std::move(child));
		}

		void deleteEntity(const std::string& name) {
			std::vector<std::string>::iterator it = std::find(m_entity_names.begin(), m_entity_names.end(), name);

			if (it != m_entity_names.end()) {
				m_entity_names.erase(it);
			}
			else {
				for (int i = 0; i < m_children.size(); ++i) {
					m_children[i]->deleteEntity(name);
				}
			}
		}

		void addEntity(const std::string& name) {
			m_entity_names.push_back(name);
		}

		void drawImGUI() const {
			/*if (ImGui::TreeNode(m_folder_name.c_str()))
			{
				for (int i = 0; i < m_entity_names.size(); ++i) {
					if (ImGui::Button(m_entity_names[i].c_str())) {
						PickingSystem::setSelectedId(i);
					}
				}

				for (int i = 0; i < m_children.size(); ++i) {
					m_children[i]->drawImGUI();
				}
				ImGui::TreePop();
			}*/
		}
	private:
		std::vector<std::unique_ptr<Node>> m_children;

		std::string m_folder_name;
		std::vector<std::string> m_entity_names;
	};

	void drawImGUI() const {
		ImGui::Text("Scene hierarchy :");
		m_root->drawImGUI();
	}

	static HierarchyEntity& getHierarchyEntity() {
		return hierarchy;
	}

private:
	HierarchyEntity() : m_root(std::make_unique<Node>()) {
	}
	~HierarchyEntity() {
	}
public:
	std::unique_ptr<Node> m_root;
private:
	static HierarchyEntity hierarchy;
};

class EntityEditionPanel {
public:
	EntityEditionPanel();
	~EntityEditionPanel();

	void addEntity(const std::string& name);
	void deleteEntity(const std::string& name);

	void render(const InputHandler& input);
	void reset();
	void setTransform(const std::string& name);

private:
	struct Transform {
		glm::vec3 tr;
		glm::vec3 scale;
		glm::vec3 rot;
	};
	std::map<std::string, Transform> m_transforms;
	std::string m_name_entity_picked;
};

namespace fs = std::experimental::filesystem;
class EntityCreationPanel {
public:
	EntityCreationPanel() : m_model_filepath("") {
	}

	~EntityCreationPanel() {
	}

	void render_filesystem(char* path) {
		for (auto &p : fs::directory_iterator(path)) {
			std::string file_path = p.path().string();
			std::string filename = file_path;
			filename.erase(0, strlen(path));
			//ImGui::AlignTextToFramePadding();
			ImGui::PushID(id);
			id++;
			if (fs::is_directory(p.path())) {
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0xff888888));
				if (ImGui::Button(filename.c_str()))
				{
					std::cout << file_path << std::endl;
					strcpy_s(path, 256, (file_path + "\\").c_str());
					//ImGui::TreePop();
				}
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button(filename.c_str())) {
					m_model_filepath = file_path;
				}
			}
			ImGui::PopID();

			ImGui::NextColumn();
		}
	}

	void render(entityx::EntityManager& es) {
		bool open = true;

		ImVec2 window_pos = ImVec2(10.f, 300.f);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		//ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		int size_col = 250;
		int num_col = 3;
		ImGui::SetNextWindowSize(ImVec2(size_col * num_col, 500));
		if (!ImGui::Begin("Entity Creation", &open))
		{
			ImGui::End();
			return;
		}
		std::string component_names[] = { "Render", "Physics", "Movable" };
		static std::vector<std::string> component_chosen;
		static std::unordered_set<int> component_chosen_ids;

		int size_component_array = sizeof(component_names) / sizeof(component_names[0]);
		for (int i = 0; i < size_component_array; ++i) {
			bool check = (component_chosen_ids.find(i) != component_chosen_ids.end());

			ImGui::Checkbox(component_names[i].c_str(), &check);
			if (check) {
				component_chosen_ids.insert(i);
				if(std::find(component_chosen.begin(), component_chosen.end(), component_names[i]) == component_chosen.end())
					component_chosen.push_back(component_names[i]);
			}
			else {
				component_chosen_ids.erase(i);
				std::vector<std::string>::iterator it = std::find(component_chosen.begin(), component_chosen.end(), component_names[i]);
				if (it != component_chosen.end())
					component_chosen.erase(it);
			}
		}

		static char name[32] = "name";
		ImGui::InputText("Entity Name", name, static_cast<int>(sizeof(name) / sizeof(*name)));
		
		// Render component variables
		static bool check_model = false;
		static bool check_cube = false;
		static bool check_plane = false;
		// Physic component variables
		static float mass = 0.f;

		for (int i = 0; i < component_chosen.size(); ++i) {
			ImGui::Separator();
			ImGui::PushID(id);
			id++;

			if (component_chosen[i] == "Render") {
				if (ImGui::TreeNode("Render")) {
					ImGui::Checkbox("Model", &check_model);
					if (check_model) {
						check_cube = false;
						check_plane = false;
						{
							static char path[256] = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Content\\";
							ImGui::InputText("", path, 256);

							std::string path_str = path;
							std::stringstream test(path_str);
							std::string segment;
							int pos = 0;
							ImGui::Spacing();

							do {
								if (!segment.empty()) {
									pos += segment.size() + 1;
									ImGui::PushID(id);
									id++;
									ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0xff888888));
									if (ImGui::Button(segment.c_str())) {
										std::string updated_path = path_str;
										updated_path.erase(pos, updated_path.size());
										strcpy_s(path, 256, updated_path.c_str());
									}
									ImGui::PopStyleColor();
									ImGui::PopID();

									ImGui::SameLine();
									ImGui::Text("\\");
									ImGui::SameLine();
								}
							} while (std::getline(test, segment, '\\'));

							ImGui::Spacing();

							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
							ImGui::Columns(num_col);

							for (int i = 0; i < num_col; ++i)
								ImGui::SetColumnWidth(i, 250);

							render_filesystem(path);

							ImGui::Columns(1);
							ImGui::PopStyleVar();
						}

						ImGui::Separator();
					}
					ImGui::Checkbox("Cube", &check_cube);
					if (check_cube) {
						check_model = false;
						check_plane = false;
					}
					ImGui::Checkbox("Plane", &check_plane);
					if (check_plane) {
						check_cube = false;
						check_model = false;
					}
					ImGui::TreePop();
				}
			}
			else if (component_chosen[i] == "Physics") {
				if (ImGui::TreeNode("Physics")) {
					ImGui::InputFloat("Mass : ", &mass);
					ImGui::TreePop();
				}
				
			}
			else if (component_chosen[i] == "Movable") {
				if (ImGui::TreeNode("Movable")) {
					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
		id = 0;

		ImGui::Separator();
		if (ImGui::Button("Create")) {
			entityx::Entity entity = es.create();
			/// Add render component
			if (check_model) {
				Render render_component = createRenderComponentModel(m_model_filepath);
				assert(render_component != nullptr);
				entity.assign<Render>(render_component);
			}
			else if(check_cube) {
				Render render_component = createRenderComponent<Cube>();
				assert(render_component != nullptr);
				entity.assign<Render>(render_component);
			}
			else if (check_plane) {
				Render render_component = createRenderComponent<Plane>();
				assert(render_component != nullptr);
				entity.assign<Render>(render_component);
			}
			/// Add physic component
			// Collision shape computed from the mesh of the entity
			entityx::ComponentHandle<Render> render = entity.component<Render>();
			assert(*render != nullptr);
			std::vector<glm::vec3>& vertices = (*render)->getPrimitive().getVertices();

			btConvexHullShape* entity_shape = new btConvexHullShape();
			for (int i = 0; i < vertices.size(); ++i) {
				entity_shape->addPoint(btVector3(vertices[i].x, vertices[i].y, vertices[i].z));
			}
			entity_shape->optimizeConvexHull();
			//btCollisionShape* entity_shape = new btBoxShape(btVector3(1/2.f,1/2.f,1/2.f));

			//btCollisionShape* entity_shape = new btBoxShape(btVector3(1,1,1));
			btTransform entity_transform;
			entity_transform.setIdentity();
			entity_transform.setOrigin(btVector3(0, 0, 0));
			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool is_dynamic = (mass != 0.f);
			btVector3 local_inertia(0, 0, 0);
			if (is_dynamic)
				entity_shape->calculateLocalInertia(mass, local_inertia);

			//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* motion_state = new btDefaultMotionState(entity_transform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, entity_shape, local_inertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			// Active rigid body
			body->setActivationState(DISABLE_DEACTIVATION);

			Physics physics = { entity_shape, motion_state, body, mass, local_inertia };
			entity.assign<Physics>(physics);

			World& world = Singleton<World>::getInstance();
			world.addEntity(name, entity);
			EntityEditionPanel& edition_panel = Singleton<EntityEditionPanel>::getInstance();
			edition_panel.addEntity(name);
		}

		ImGui::End();
	}

	Render createRenderComponentModel(const std::string& file_path) const {
		if (file_path.empty())
			return nullptr;
		if (file_path.find(".obj") == std::string::npos && file_path.find(".md5") == std::string::npos)
			return nullptr;

		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

		std::shared_ptr<Renderable<Model>> render_component = std::make_shared<Renderable<Model>>(shaders.get("texture"), file_path);
		return render_component;
	}

	template<typename T>
	Render createRenderComponent() const {
		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

		std::shared_ptr<Renderable<T>> render_component = std::make_shared<Renderable<T>>(shaders.get("simple"));
		return render_component;
	}

private:
	std::string m_model_filepath;
	static int id;
};

