#include "PickingSystem.h"
#include "InputHandler.h"

bool PickingSystem::m_picked = false;
int PickingSystem::m_picked_index = -1;
int PickingSystem::m_prev_picked = 0;

PickingSystem PickingSystem::m_entity;

const std::unique_ptr<Entity>& PickingSystem::getPickedEntity() {
	const std::vector<std::unique_ptr<Entity>>& entities = GameProgram::m_scene->getEntities();

	return entities[m_prev_picked];
}

int PickingSystem::getSelectedId() {
	return m_prev_picked;
}

void PickingSystem::update(InputHandler* input) {
	std::vector<std::unique_ptr<Entity>>& entities = GameProgram::m_scene->getEntities();

	if (input->m_button == SDL_BUTTON_LEFT) {
		// Normalized Device coords
		int mouse_X = input->m_mouse_X;
		glm::vec2 mouse_normalized_device(2.f * mouse_X / GameProgram::width - 1, 1.f - 2.f * input->m_mouse_Y / GameProgram::height);
		// Homogeneous Clip coords
		// The view direction points towards the negative z.
		// w = -z
		glm::vec4 mouse_ray_clip(mouse_normalized_device.x, mouse_normalized_device.y, -1, 1);
		// Eye coords
		glm::vec4 mouse_point_eye = glm::inverse(Viewer::getProjectionMatrix()) * mouse_ray_clip;
		// Set the w to 0 because we need a ray
		// mouse_point_eye.z = -1 => the vector is aligned with the forward direction of the viewer
		glm::vec4 mouse_ray_eye(mouse_point_eye.x, mouse_point_eye.y, mouse_point_eye.z, 0);

		glm::vec4 mouse_ray_model = glm::inverse(GameProgram::m_viewer.getViewMatrix()) * mouse_ray_eye;
		glm::vec3 t = glm::normalize(glm::vec3(mouse_ray_model.x, mouse_ray_model.y, mouse_ray_model.z));

		glm::vec3 n(0, 1, 0);
		glm::vec3 originToViewer(GameProgram::m_viewer.getPosition());

		float lambda = -glm::dot(n, originToViewer) / glm::dot(n, t);

		// Compute intersection point
		glm::vec3 I = GameProgram::m_viewer.getPosition() + lambda * t;

		if (!m_picked) {
			for (unsigned int i = 0; i < entities.size(); ++i) {
				if (entities[i]->intersect(I)) {
					m_picked = true;

					// Render the unselected box object as black
					{
						const std::unique_ptr<Renderable<Cube>>& selection_box = entities[m_prev_picked]->getSelectionBoxObject();
						selection_box->setColor(glm::vec4(0, 0, 0, 1));
					}

					m_picked_index = i;
					m_prev_picked = m_picked_index;

					// Render the selected box object as green
					{
						const std::unique_ptr<Renderable<Cube>>& selection_box = entities[m_picked_index]->getSelectionBoxObject();
						selection_box->setColor(glm::vec4(0, 1, 0, 1));
					}
					break;
				}
			}
		}

		if (m_picked_index != -1) {
			LocalTransform transform = entities[m_picked_index]->getLocalTransform();
			transform.setTranslation(I);
			entities[m_picked_index]->setLocalTransform(transform);
		}
	}
	else {
		m_picked_index = -1;
		m_picked = false;
	}
}

PickingSystem::PickingSystem() {
}
PickingSystem::~PickingSystem() {
}

