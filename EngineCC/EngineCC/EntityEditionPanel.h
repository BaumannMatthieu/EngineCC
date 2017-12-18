#pragma once
#include <memory>
#include <vector>
// Include Imgui
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "GameProgram.h"
#include "PickingSystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

class EntityEditionPanel {
public:
	EntityEditionPanel() : m_entities(GameProgram::m_scene->getEntities()) {
		m_tr.resize(m_entities.size(), glm::vec3(0));
		m_scale.resize(m_entities.size(), glm::vec3(1));
		m_rot.resize(m_entities.size(), glm::vec3(0));
	}
	~EntityEditionPanel() {
	}

	void render() {
		int selected_id = PickingSystem::getSelectedId();
		std::unique_ptr<Entity> selected = std::move(m_entities[selected_id]);
		m_tr[selected_id] = glm::column(selected->getLocalTransform().getTranslationMatrix(), 3);
		{
			ImGui::Begin("Entity Selected Edition");
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

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

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
private:
	std::vector<std::unique_ptr<Entity>>& m_entities;

	std::vector<glm::vec3> m_rot;
	std::vector<glm::vec3> m_tr;
	std::vector<glm::vec3> m_scale;
};

