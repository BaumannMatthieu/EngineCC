#pragma once
#include <memory>
#include <entityx/entityx.h>

#include "InputHandler.h"
#include "ScriptSystem.h"

struct InteractionEvent {
	entityx::Entity entity_emitting;
};

class PickingSystem : public entityx::System<PickingSystem>, public entityx::Receiver<PickingSystem> {
public:
	PickingSystem(const Viewer& viewer, const InputHandler& input_handler, entityx::Entity player) : m_viewer(viewer), m_input_handler(input_handler), m_player(player) {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<InteractionEvent>(*this);
	}

	void receive(const InteractionEvent &event) {
		entityx::Entity entity;
		if (isEntityPerInteraction(entity, m_input_handler, m_viewer)) {
			std::weak_ptr<ScriptSystem> script_system = GameProgram::game->systems.system<ScriptSystem>();
			entityx::EventManager& ev = GameProgram::game->events;
			if (auto script_system_ptr = script_system.lock()) {
				if (!script_system_ptr->isRunningScriptFrom(entity)) {
					ev.emit<LaunchEvent>({ entity, Script::INTERACTION });
					std::cout << "interaction" << std::endl;
				}
			}

			if (entity.component<Carryable>()) {
				const btTransform& local_transform = entity.component<Carryable>()->local_tr;

				// TODO : exploit event.entity_emitting instead of the player
				World& world = Singleton<World>::getInstance();
				entityx::ComponentHandle<Handler> player_handler = m_player.component<Handler>();
				entityx::ComponentHandle<Physics> player_physics = m_player.component<Physics>();

				btCompoundShape* player_compound_shape = (btCompoundShape*)player_physics->collision_shape;
				btRigidBody* player_body = player_physics->rigid_body;
				if (player_handler->left_arm.valid()) {
					entityx::ComponentHandle<Physics> picked_physics = player_handler->left_arm.component<Physics>();

					player_compound_shape->removeChildShape(picked_physics->collision_shape);
					player_physics->mass -= picked_physics->mass;

					picked_physics->rigid_body->setWorldTransform(player_body->getWorldTransform() * local_transform);
					world.dynamic_world->addRigidBody(picked_physics->rigid_body);
				}
				player_handler->left_arm = entity;

				player_compound_shape->addChildShape(local_transform, entity.component<Physics>()->collision_shape);

				player_physics->mass += entity.component<Physics>()->mass;
				// Recomputation of the inertia of the player compound shape
				btVector3 inertia;
				player_compound_shape->calculateLocalInertia(player_physics->mass, inertia);
				player_body->setMassProps(player_physics->mass, inertia);
				player_body->updateInertiaTensor();
				
				world.dynamic_world->removeRigidBody(entity.component<Physics>()->rigid_body);
			}
		}
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		
		/*es.each<Carryable, Physics>([this](entityx::Entity entity, Carryable& handler, Physics& physic) {
			entityx::ComponentHandle<Physics> physics = m_player.component<Physics>();
			btCompoundShape* player_compound_shape = (btCompoundShape*)physics->collision_shape;

			entity.component<Physics>()->rigid_body->setWorldTransform(player_compound_shape->getChildTransform(0));
			//handler.root->setLocalTransform(physic.rigid_body->getWorldTransform());
			//handler.root->computeTransformHierarchy();

			World& world = Singleton<World>::getInstance();
			world.dynamic_world->updateAabbs();
		});*/
	}
	
	// Returns true if picked entity is at a minimal distance of interaction. Returns false otherwise (no picked entity or
	// too far).
	static bool isEntityPerInteraction(entityx::Entity& entity, const InputHandler& input_handler, const Viewer& viewer) {
		bool hit = false;
		btVector3 intersection_point;
		const std::string& name_entity = PickingSystem::getPickedEntity(input_handler, *GameProgram::m_current_viewer, hit, intersection_point);
		if (hit) {
			btVector3 player_pos(viewer.getPosition().x, viewer.getPosition().y, viewer.getPosition().z);
			btScalar distance = (player_pos - intersection_point).norm();
#define DISTANCE_MIN_INTERACTION 10.f
			if (distance < DISTANCE_MIN_INTERACTION) {
				World& world = Singleton<World>::getInstance();
				entity = world.get(name_entity);
				return true;
			}
		}
		return false;
	}

	// Get the entity that has been picked by the user when hitting the
	// left button of the mouse
	// return "" if nothing has been hit
	static std::string getPickedEntity(const InputHandler& input, const Viewer& viewer, bool& hit, btVector3& intersection_point) {
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

private:
	entityx::Entity m_player;
	const Viewer& m_viewer;
	const InputHandler& m_input_handler;
};