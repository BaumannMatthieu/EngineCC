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

// Include TinyXML2 for saving/loading entity scenes
#include "tinyxml2.h"
using namespace tinyxml2;

#include "Singleton.h"
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

struct EditionWindow {
	struct Transform {
		glm::vec3 tr;
		glm::vec3 scale;
		glm::vec3 rot;
	};

	void addEntity(const std::string& name, const Transform& tr, entityx::Entity entity);
	void deleteEntity(const std::string& name);

	void render(const InputHandler& input);
	void reset();
	void clear();

	void setTransform(const std::string& name, const Transform& tr);

private:
	void updateEntity(const std::string& name);

public:
	std::map<std::string, Transform> m_transforms;
	std::string m_name_entity_picked;
};

std::ostream& operator<<(std::ostream& stream_out, const EditionWindow::Transform& transform);

namespace fs = std::experimental::filesystem;
class EntityCreationPanel {
public:
	EntityCreationPanel() : m_model_filepath("") {
	}

	~EntityCreationPanel() {
	}

	char* searchFileWindow(bool* open) {
		if (!ImGui::Begin("Filesystem Window", open))
		{
			ImGui::End();
			return NULL;
		}
		
		static char path[256] = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Content\\";
		static char file[256];

		int num_col = 3;
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

		for (auto &p : fs::directory_iterator(path)) {
			std::string file_path = p.path().string();
			std::string filename = file_path;
			filename.erase(0, strlen(path));
			ImGui::PushID(id);
			id++;
			if (fs::is_directory(p.path())) {
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0xff888888));
				if (ImGui::Button(filename.c_str()))
				{
					std::cout << file_path << std::endl;
					strcpy_s(path, 256, (file_path + "\\").c_str());
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

		ImGui::Columns(1);
		ImGui::PopStyleVar();

		if (ImGui::Button("Ok")) {
			*open = false;
		}

		ImGui::End();

		strcpy_s(file, 256, m_model_filepath.c_str());
		return file;
	}

	struct ComponentsData {
		// Name of the entity
		std::string name;
		// Render component
		enum RenderableType {
			MODEL,
			CUBE,
			PLANE
		};
		RenderableType renderable_type;
		std::string filename;

		// Physics component
		float mass;
		bool disable_angular_rotation;
	};

	/// Add a render component to the entity passed to the function
	// Precondition : a render component is associated with an entity only when
	// - the entity is valid
	void addRenderComponent(const ComponentsData& data, entityx::Entity entity) {
		assert(entity.valid());
		Render render = nullptr;
		if (data.renderable_type == ComponentsData::MODEL) {
			render = createRenderComponentModel(data.filename);
		}
		else if (data.renderable_type == ComponentsData::CUBE) {
			render = createRenderComponent<Cube>();
		}
		else if (data.renderable_type == ComponentsData::PLANE) {
			render = createRenderComponent<Plane>();
		}

		entity.assign<Render>(render);
	}

	/// Add a physics component to the entity passed to the function
	// Precondition : a physic component is associated with an entity only when
	// - the entity is valid
	// - the entity contains a render component
	void addPhysicsComponent(const ComponentsData& data, entityx::Entity entity) {
		assert(entity.valid());
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

		btTransform entity_transform;
		entity_transform.setIdentity();
		entity_transform.setOrigin(btVector3(0, 0, 0));
		//rigidbody is dynamic if and only if mass is non zero, otherwise static

		bool is_dynamic = (data.mass != 0.f);
		btVector3 local_inertia(0, 0, 0);
		if (is_dynamic)
			entity_shape->calculateLocalInertia(data.mass, local_inertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* motion_state = new btDefaultMotionState(entity_transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(data.mass, motion_state, entity_shape, local_inertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		// Active rigid body
		body->setActivationState(DISABLE_DEACTIVATION);
		if (data.disable_angular_rotation)
			body->setAngularFactor(0.f);

		Physics physics = { entity_shape, motion_state, body, data.mass, local_inertia };
		entity.assign<Physics>(physics);
	}

	void creationEntity(entityx::EntityManager& es, const ComponentsData& data, const EditionWindow::Transform& tr) {
		entityx::Entity entity = es.create();
		addRenderComponent(data, entity);
		addPhysicsComponent(data, entity);

		/// Add into the edition panel so that we can pick the entity and change its transformation
		EditionWindow& edition_panel = Singleton<EditionWindow>::getInstance();
		edition_panel.addEntity(data.name, tr, entity);
	}

	void loadEntity(entityx::EntityManager& es, const std::string& name, const EditionWindow::Transform& tr) {
		std::string filename = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Scenes\\" + std::string(name) + ".xml";

		XMLDocument doc;
		XMLError eResult = doc.LoadFile(filename.c_str());
		if (eResult != XML_SUCCESS) {
			std::cout << "Scene file not found at : " << filename << std::endl;
		}
		else {
			ComponentsData::RenderableType renderable_type;
			std::string filename;
			float mass;
			bool disable_angular_rotation;

			XMLElement* root = doc.FirstChildElement("Root");
			XMLElement* components = root->FirstChildElement("Components");
			
			/// Load each component one by one by reading into the DOM data structure obtained from the XML file
			// Render component
			XMLElement* render_component = components->FirstChildElement("Render");
			assert(render_component != nullptr);
			const char* renderable_type_str = nullptr;
			renderable_type_str = render_component->Attribute("renderable_type");
			assert(renderable_type_str != nullptr);

			if (std::strcmp(renderable_type_str, "model") == 0) {
				const char* filename_cstr = render_component->Attribute("filename");
				assert(filename_cstr != nullptr);
				renderable_type = ComponentsData::MODEL;
				filename = std::string(filename_cstr);
			}
			else if(std::strcmp(renderable_type_str, "cube") == 0) {
				renderable_type = ComponentsData::CUBE;
			}
			else if (std::strcmp(renderable_type_str, "plane") == 0) {
				renderable_type = ComponentsData::PLANE;
			}

			// Physics component
			XMLElement* physics_component = components->FirstChildElement("Physics");
			assert(physics_component != nullptr);

			XMLElement* mass_elt = physics_component->FirstChildElement("mass");
			assert(mass_elt != nullptr);
			mass_elt->QueryFloatText(&mass);

			XMLElement* angular_rot_elt = physics_component->FirstChildElement("angular_rotation_disabled");
			assert(angular_rot_elt != nullptr);
			angular_rot_elt->QueryBoolText(&disable_angular_rotation);

			/// Create the entity with all the data retrieved
			ComponentsData data = {name, renderable_type, filename, mass, disable_angular_rotation};
			creationEntity(es, data, tr);
		}
	}

	void loadSaveSceneWindow(entityx::EntityManager& es) {
		static bool open_scene_window = false;
		if (!ImGui::Begin("Scene Manager Window", &open_scene_window))
		{
			ImGui::End();
			return;
		}

		if (ImGui::Button("New Scene")) {
			EditionWindow& edition_window = Singleton<EditionWindow>::getInstance();
			edition_window.clear();
		}
		ImGui::Separator();
		static char save_filename[256] = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Scenes\\scene1.xml";
		ImGui::PushID(id);
		id++;
		ImGui::InputText("", save_filename, 256);
		ImGui::PopID();
		if (ImGui::Button("Save Scene")) {
			/// Edition of the XML scene file
			// A XML file contains the list of entities that are inserted in the scene
			// Each entity is definied by a unique name referring to a set of specific components and values
			// Each entity is associated with a transform (i.e. its model matrix) characterized by 
			// a rotation, translation and scale vector.
			XMLDocument doc;
			XMLNode* pRoot = doc.NewElement("Root");
			doc.InsertFirstChild(pRoot);
			XMLElement* entityList = doc.NewElement("ListEntities");

			EditionWindow& edition_win = Singleton<EditionWindow>::getInstance();
			for (std::map<std::string, EditionWindow::Transform>::const_iterator it = edition_win.m_transforms.begin();
				it != edition_win.m_transforms.end(); ++it) {
				const std::string& name = it->first;
				const EditionWindow::Transform& tr = it->second;

				XMLElement* entity = doc.NewElement("Entity");
				entity->SetAttribute("name", name.c_str());

				XMLElement* rot_info = doc.NewElement("Rotation");
				rot_info->SetAttribute("X", tr.rot.x);
				rot_info->SetAttribute("Y", tr.rot.y);
				rot_info->SetAttribute("Z", tr.rot.z);

				XMLElement* translation_info = doc.NewElement("Translation");
				translation_info->SetAttribute("X", tr.tr.x);
				translation_info->SetAttribute("Y", tr.tr.y);
				translation_info->SetAttribute("Z", tr.tr.z);

				XMLElement* scale_info = doc.NewElement("Scale");
				scale_info->SetAttribute("X", tr.scale.x);
				scale_info->SetAttribute("Y", tr.scale.y);
				scale_info->SetAttribute("Z", tr.scale.z);

				entity->InsertEndChild(rot_info);
				entity->InsertEndChild(translation_info);
				entity->InsertEndChild(scale_info);

				entityList->InsertEndChild(entity);
			}
			pRoot->InsertEndChild(entityList);
			
			XMLError eResult = doc.SaveFile(save_filename);
			assert(eResult == XML_SUCCESS);
		}
		ImGui::Separator();
		static char load_filename[256] = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Scenes\\scene1.xml";
		ImGui::PushID(id);
		id++;
		ImGui::InputText("", load_filename, 256);
		ImGui::PopID();
		if (ImGui::Button("Load Scene")) {
			XMLDocument doc;
			XMLError eResult = doc.LoadFile(load_filename);
			if (eResult != XML_SUCCESS) {
				std::cout << "Scene file not found at : " << load_filename << std::endl;
			}
			else {
				// We clear the current scene
				EditionWindow& edition_window = Singleton<EditionWindow>::getInstance();
				edition_window.clear();
				// Once the entity manager contains no valid entity, we add
				// the entities that are in the xml file

				XMLElement* root = doc.FirstChildElement("Root");
				XMLElement* entityList = root->FirstChildElement("ListEntities");
				XMLElement* current_entity = entityList->FirstChildElement("Entity");
				while (current_entity != nullptr) {
					const char* name_attribute = nullptr;
					name_attribute = current_entity->Attribute("name");
					assert(name_attribute != nullptr);
					std::string name = name_attribute;


					EditionWindow::Transform transform;
					XMLElement* rotation = current_entity->FirstChildElement("Rotation");
					rotation->QueryFloatAttribute("X", &transform.rot.x);
					rotation->QueryFloatAttribute("Y", &transform.rot.y);
					rotation->QueryFloatAttribute("Z", &transform.rot.z);

					XMLElement* translation = current_entity->FirstChildElement("Translation");
					translation->QueryFloatAttribute("X", &transform.tr.x);
					translation->QueryFloatAttribute("Y", &transform.tr.y);
					translation->QueryFloatAttribute("Z", &transform.tr.z);

					XMLElement* scale = current_entity->FirstChildElement("Scale");
					scale->QueryFloatAttribute("X", &transform.scale.x);
					scale->QueryFloatAttribute("Y", &transform.scale.y);
					scale->QueryFloatAttribute("Z", &transform.scale.z);

					loadEntity(es, name, transform);

					//edition_window.setTransform(name, transform);
					
					current_entity = current_entity->NextSiblingElement("Entity");
				}
			}
		}
		
		ImGui::End();
	}

	void render(entityx::EntityManager& es) {
		bool open = true;

		ImVec2 window_pos = ImVec2(10.f, 300.f);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		//ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		//int size_col = 250;

		//ImGui::SetNextWindowSize(ImVec2(size_col * num_col, 500));
		if (!ImGui::Begin("Entity Creation Window", &open))
		{
			ImGui::End();
			return;
		}

		static char name[32] = "name";
		ImGui::InputText(": Entity Name", name, static_cast<int>(sizeof(name) / sizeof(*name)));
		ImGui::Separator();
		ImGui::Text("Entity components :");
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

		// Render component variables
		static bool check_model = false;
		static bool check_cube = false;
		static bool check_plane = false;

		// Physic component variables
		static float mass = 0.f;
		static bool disable_angular_rotation = false;

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

						ImGui::SameLine();
						static bool open_file_search = false;
						if (ImGui::Button("Open file"))
							open_file_search = true;
						static char* path = NULL;
						if (open_file_search) {
							path = searchFileWindow(&open_file_search);
						}
						if (path)
							ImGui::InputText("Path to the model file : ", path, static_cast<int>(sizeof(path) / sizeof(*path)));

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
					ImGui::InputFloat(": mass", &mass);
					ImGui::Checkbox(": disable angular rotation", &disable_angular_rotation);
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
			/// Automatic save of the entity in an XML file
			// An entity is saved to an XML file with its components
			// Each of its components refers to an XMLElement with value/sub-elements etc...
			XMLDocument doc;
			XMLNode* pRoot = doc.NewElement("Root");
			doc.InsertFirstChild(pRoot);

			XMLElement* name_elt = doc.NewElement("Name");
			name_elt->SetText(name);
			pRoot->InsertEndChild(name_elt);

			XMLElement* components = doc.NewElement("Components");

			// Render component
			XMLElement* render_elt = doc.NewElement("Render");
			components->InsertEndChild(render_elt);
			// Physics component
			XMLElement* physic_elt = doc.NewElement("Physics");
			components->InsertEndChild(physic_elt);

			pRoot->InsertEndChild(components);
			ComponentsData::RenderableType renderable_type;

			if (check_model) {
				renderable_type = ComponentsData::MODEL;

				render_elt->SetAttribute("renderable_type", "model");
				render_elt->SetAttribute("filename", m_model_filepath.c_str());
			}
			else if(check_cube) {
				renderable_type = ComponentsData::CUBE;
				
				render_elt->SetAttribute("renderable_type", "cube");
			}
			else if (check_plane) {
				renderable_type = ComponentsData::PLANE;			
				
				render_elt->SetAttribute("renderable_type", "plane");
			}

			XMLElement* mass_elt = doc.NewElement("mass");
			mass_elt->SetText(mass);
			physic_elt->InsertEndChild(mass_elt);
			XMLElement* ang_rot_elt = doc.NewElement("angular_rotation_disabled");
			ang_rot_elt->SetText(disable_angular_rotation);
			physic_elt->InsertEndChild(ang_rot_elt);

			std::string filename = "C:\\Users\\Matthieu\\source\\repos\\EngineCC\\EngineCC\\EngineCC\\Scenes\\" + std::string(name) + ".xml";
			XMLError eResult = doc.SaveFile(filename.c_str());
			assert(eResult == XML_SUCCESS);

			ComponentsData data = {name, renderable_type, m_model_filepath, mass, disable_angular_rotation};
			EditionWindow::Transform transform = {glm::vec3(0), glm::vec3(1), glm::vec3(0)};
			creationEntity(es, data, transform);
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

