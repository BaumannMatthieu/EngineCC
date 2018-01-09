#include "PickingSystem.h"
#include "Game.h"

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
	//btVector3 I;
	hit = world.isEntityPicked(btFrom, btTo, hit_entity, intersection_point);
	/*
	// TODO Put that in the EditionWindow of the Editor code. Picking system needs to return the exact intersection point !
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
	}*/

	return hit_entity;
}

