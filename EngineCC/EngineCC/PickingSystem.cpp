#include "PickingSystem.h"
#include "Game.h"


/*
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

void PickingSystem::setSelectedId(int selected_id) {
	m_prev_picked = selected_id;
}

void PickingSystem::update(const InputHandler& input, const Viewer& viewer, bool snap_to_grid) {
	std::vector<std::unique_ptr<Entity>>& entities = GameProgram::m_scene->getEntities();

	if (input.m_button == SDL_BUTTON_LEFT) {
		// Normalized Device coords
		int mouse_X = input.m_mouse_X;
		glm::vec2 mouse_normalized_device(2.f * mouse_X / GameProgram::width - 1, 1.f - 2.f * input.m_mouse_Y / GameProgram::height);
		// Homogeneous Clip coords
		// The view direction points towards the negative z.
		// w = -z
		glm::vec4 mouse_ray_clip(mouse_normalized_device.x, mouse_normalized_device.y, -1, 1);
		// Eye coords
		glm::vec4 mouse_point_eye = glm::inverse(Viewer::getProjectionMatrix()) * mouse_ray_clip;
		// Set the w to 0 because we need a ray
		// mouse_point_eye.z = -1 => the vector is aligned with the forward direction of the viewer
		glm::vec4 mouse_ray_eye(mouse_point_eye.x, mouse_point_eye.y, mouse_point_eye.z, 0);

		glm::vec4 mouse_ray_model = glm::inverse(viewer.getViewMatrix()) * mouse_ray_eye;
		glm::vec3 t = glm::normalize(glm::vec3(mouse_ray_model.x, mouse_ray_model.y, mouse_ray_model.z));

		glm::vec3 n(0, 1, 0);
		glm::vec3 originToViewer(viewer.getPosition());

		float lambda = -glm::dot(n, originToViewer) / glm::dot(n, t);

		// Compute intersection point
		glm::vec3 I = viewer.getPosition() + lambda * t;
		if (snap_to_grid) {
			I.x = round(I.x);
			I.z = round(I.z);
			I.y = round(I.y);
		}
		if (!m_picked) {
			for (unsigned int i = 0; i < entities.size(); ++i) {
				if (entities[i]->intersect(I)) {
					m_picked = true;
					// Render the unselected box object as black


					m_picked_index = i;
					m_prev_picked = m_picked_index;


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
*/
std::string PickingSystem::getPickedEntity(const InputHandler& input, const Viewer& viewer, bool& hit, btVector3& intersection_point) {
	hit = false;

	// Normalized Device coords
	glm::vec2 mouse_normalized_device(2.f * input.m_mouse_X / GameProgram::width - 1, 1.f - 2.f * input.m_mouse_Y / GameProgram::height);
	// Homogeneous Clip coords
	// The view direction points towards the negative z.
	// w = -z
	glm::vec4 mouse_ray_clip(mouse_normalized_device.x, mouse_normalized_device.y, -1, 1);
	// Eye coords
	glm::vec4 mouse_point_eye = glm::inverse(Viewer::getProjectionMatrix()) * mouse_ray_clip;
	// Set the w to 0 because we need a ray
	// mouse_point_eye.z = -1 => the vector is aligned with the forward direction of the viewer
	glm::vec4 mouse_ray_eye(mouse_point_eye.x, mouse_point_eye.y, mouse_point_eye.z, 0);

	glm::vec4 mouse_ray_model = glm::inverse(viewer.getViewMatrix()) * mouse_ray_eye;
	btVector3 t(mouse_ray_model.x, mouse_ray_model.y, mouse_ray_model.z);
	t = t.normalize();

	btVector3 btFrom(viewer.getPosition().x, viewer.getPosition().y, viewer.getPosition().z);
#define DISTANCE_MAX_PICKING 1000.f
	btVector3 btTo = btFrom + t*DISTANCE_MAX_PICKING;

	World& world = Singleton<World>::getInstance();
	std::string hit_entity;

	// I is the coordinate of the intersected point
	btVector3 I;
	hit = world.isEntityPicked(btFrom, btTo, hit_entity, I);

	// Computation of the intersection point between the plane of normal ey and at a distance center_of_intersected_object.y from the origin and vector t.
	if (hit) {
		entityx::Entity picked_entity = world.get(hit_entity);
		// Get the center of mass of the picked entity
		entityx::ComponentHandle<Physics> physic = picked_entity.component<Physics>();
		btScalar center_of_mass_Y = physic->rigid_body->getCenterOfMassPosition().y();
		btVector3 n(0, 1, 0);
		btVector3 viewer_pos(viewer.getPosition().x, viewer.getPosition().y, viewer.getPosition().z);
		btScalar lambda = (center_of_mass_Y - n.dot(viewer_pos)) / n.dot(t);
		intersection_point = viewer_pos + lambda * t;
	}

	return hit_entity;
}

