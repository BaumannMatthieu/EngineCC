#pragma once
#include <memory>
#include <vector>
#include <filesystem>
#include <sstream>
#include <stdio.h>

// Include Imgui
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "GameProgram.h"
#include "PickingSystem.h"
#include "Singleton.h"

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
			if (ImGui::TreeNode(m_folder_name.c_str()))
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
			}
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
	EntityEditionPanel() : m_num_edition_entities(0) {
	}

	~EntityEditionPanel() {
	}

	void addEntity(const std::unique_ptr<Entity>& entity) {
		m_tr.push_back(glm::vec3(0));
		m_scale.push_back(entity->getLocalTransform().getScaleVec());
		m_rot.push_back(glm::vec3(0));

		HierarchyEntity& hierarchy = HierarchyEntity::getHierarchyEntity();
		hierarchy.m_root->addEntity(entity->getName());

		m_num_edition_entities++;
	}

	void deleteEntity(int selected_id) {
		m_tr.erase(m_tr.begin() + selected_id);
		m_scale.erase(m_scale.begin() + selected_id);
		m_rot.erase(m_rot.begin() + selected_id);

		m_num_edition_entities--;
	}

	void render() {
		std::vector<std::unique_ptr<Entity>>& m_entities = GameProgram::m_scene->getEntities();
		int selected_id = PickingSystem::getSelectedId();

		// Can only edit entities which have been inserted in the edition mode and not by the scripts or the player
		if (m_num_edition_entities != 0 && selected_id < m_num_edition_entities) {
			std::unique_ptr<Entity> selected = std::move(m_entities[selected_id]);
			m_tr[selected_id] = glm::column(selected->getLocalTransform().getTranslationMatrix(), 3);
			{
				bool open = true;

				ImVec2 window_pos = ImVec2(10.f, 10.f);
				ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
				ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

				ImGui::Begin("Entity edition", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
				ImGui::Text("Name : %s", selected->getName().c_str());
				if (ImGui::TreeNode("Rotation"))
				{
					ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&m_rot[selected_id]));
					ImGui::TreePop();
				}
				ImGui::Separator();
				if (ImGui::TreeNode("Scale"))
				{
					ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&m_scale[selected_id]));
					ImGui::TreePop();
				}
				ImGui::Separator();
				if (ImGui::TreeNode("Translation"))
				{
					ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&m_tr[selected_id]));
					ImGui::TreePop();
				}
				ImGui::Separator();

				HierarchyEntity& hierarchy = HierarchyEntity::getHierarchyEntity();
				hierarchy.drawImGUI();

				if (ImGui::Button("Delete")) {
					hierarchy.m_root->deleteEntity(selected->getName());

					std::unique_ptr<Entity> entity_deleted = std::move(selected);
					m_entities.erase(m_entities.begin() + selected_id);

					this->deleteEntity(selected_id);

					if (!m_entities.empty()) {
						selected_id = selected_id % m_entities.size();

						PickingSystem::setSelectedId(selected_id);
						selected = std::move(m_entities[selected_id]);
					}
				}

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			if (m_entities.empty())
				return;

			const LocalTransform& prev_transform = selected->getLocalTransform();

			LocalTransform new_transform = prev_transform;

			// Set translation
			glm::mat4 translation_mat(1.f);
			translation_mat = glm::translate(glm::mat4(1), m_tr[selected_id]);
			new_transform.setTranslation(translation_mat);

			// Set rotation
			glm::mat4 rotation_mat(1.f);
			float theta_X = m_rot[selected_id].x * (2 * M_PI) / 360;
			float theta_Y = m_rot[selected_id].y * (2 * M_PI) / 360;
			float theta_Z = m_rot[selected_id].z * (2 * M_PI) / 360;
			rotation_mat = glm::rotate(glm::mat4(1), theta_X, glm::vec3(1, 0, 0));
			rotation_mat = glm::rotate(rotation_mat, theta_Y, glm::vec3(0, 1, 0));
			rotation_mat = glm::rotate(rotation_mat, theta_Z, glm::vec3(0, 0, 1));
			new_transform.setRotation(rotation_mat);

			// Set scale
			glm::mat4 scale_mat(1.f);
			scale_mat = glm::scale(glm::mat4(1), m_scale[selected_id]);
			new_transform.setScale(scale_mat);

			selected->setLocalTransform(new_transform);
			m_entities[selected_id] = std::move(selected);
		}
	}

private:
	std::vector<glm::vec3> m_rot;
	std::vector<glm::vec3> m_tr;
	std::vector<glm::vec3> m_scale;

	int m_num_edition_entities;
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
			id++;

			ImGui::NextColumn();
		}
	}

	void render() {
		bool open = true;

		ImVec2 window_pos = ImVec2(10.f, 300.f);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		//ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		int size_col = 250;
		int num_col = 3;
		ImGui::SetNextWindowSize(ImVec2(size_col * num_col, 500));
		if (!ImGui::Begin("Entity Creation", &open, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::End();
			return;
		}

		static char name[32] = "name";
		ImGui::InputText("Entity Name", name, static_cast<int>(sizeof(name) / sizeof(*name)));
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
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0xff888888));
				if (ImGui::Button(segment.c_str())) {
					std::string updated_path = path_str;
					updated_path.erase(pos, updated_path.size());
					strcpy_s(path, 256, updated_path.c_str());
				}
				ImGui::PopStyleColor();
				ImGui::PopID();
				id++;
				ImGui::SameLine();
				ImGui::Text("\\");
				ImGui::SameLine();
			}
		} while (std::getline(test, segment, '\\'));
		ImGui::Spacing();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

		ImGui::Columns(num_col);
		for(int i = 0; i < num_col; ++i)
			ImGui::SetColumnWidth(i, 250);

		render_filesystem(path);

		id = 0;
		ImGui::Columns(1);
		ImGui::PopStyleVar();

		ImGui::Separator();

		if (ImGui::Button("Create")) {
			EntityCreationPanel::addNewModelEntity(std::string(name), m_model_filepath);
		}
		ImGui::SameLine();
		if (ImGui::Button("Create Cube")) {
			EntityCreationPanel::addNewEntity<Cube>(std::string(name));
		}
		ImGui::SameLine();
		if (ImGui::Button("Create Plane")) {
			EntityCreationPanel::addNewEntity<Plane>(std::string(name));
		}

		ImGui::End();
	}

	template<typename T>
	static void addNewEntity(const std::string& name) {
		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

		std::unique_ptr<Renderable<T>> new_renderable = std::make_unique<Renderable<T>>(shaders.get("grid"));
		std::unique_ptr<Entity> new_entity = std::make_unique<Entity>(name, std::move(new_renderable));

		//Add into EntityEditionPanel
		EntityEditionPanel& entity_panel = Singleton<EntityEditionPanel>::getInstance();
		entity_panel.addEntity(new_entity);

		GameProgram::m_scene->addEntity(std::move(new_entity));
	}

	static void addNewModelEntity(const std::string& name, const std::string& file_path) {
		if (file_path.empty())
			return;
		if (file_path.find(".obj") == std::string::npos && file_path.find(".md5") == std::string::npos)
			return;

		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

		std::unique_ptr<Renderable<Model>> new_renderable = std::make_unique<Renderable<Model>>(shaders.get("texture"), file_path);
		std::unique_ptr<Entity> new_entity = std::make_unique<Entity>(name, std::move(new_renderable));

		//Add into EntityEditionPanel
		EntityEditionPanel& entity_panel = Singleton<EntityEditionPanel>::getInstance();
		entity_panel.addEntity(new_entity);

		GameProgram::m_scene->addEntity(std::move(new_entity));
	}

private:
	std::string m_model_filepath;
	static int id;
};

