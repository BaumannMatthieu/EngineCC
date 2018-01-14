#pragma once
#include <memory>
#include <entityx/entityx.h>

#include "InputHandler.h"
#include "ScriptSystem.h"
#include "EntityHierarchy.h"

using namespace std;

struct InteractionEvent {
	InteractionEvent(entityx::Entity willingInteractionEntity, entityx::Entity interactionWithEntity) :
		willingInteractionEntity(willingInteractionEntity), interactionWithEntity(interactionWithEntity) {
	}

	entityx::Entity willingInteractionEntity;
	entityx::Entity interactionWithEntity;
};

// The picking system's role is to manage if the interactionWithEntity is no too far from the
// willingInteractionEntity (e.g. the player).
// If it is the case, the Interaction script from the interactionWithEntity can be launched
// If the interactionWithEntity is carryable and the willingInteractionEntity can handle objects
// then the willingInteractionEntity can equip the interactionWithEntity
class PickingSystem : public entityx::System<PickingSystem>, public entityx::Receiver<PickingSystem> {
public:
	PickingSystem(const Viewer& viewer, const InputHandler& input_handler, entityx::Entity player) : m_viewer(viewer), m_input_handler(input_handler), m_player(player) {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<InteractionEvent>(*this);
	}

	void receive(const InteractionEvent &event) {
		entityx::Entity interactionWithEntity = event.interactionWithEntity;
		entityx::Entity willingInteractionEntity = event.willingInteractionEntity;

		// Test if the two entities are not too far to interact with each other
		if (!hasComponent<Physics>(interactionWithEntity) || !hasComponent<Physics>(willingInteractionEntity)) {
			return;
		}

		Component<Physics> pPhysicsInteractionWithComponent = getComponent<Physics>(interactionWithEntity);
		Component<Physics> pPhysicsWillInteractionComponent = getComponent<Physics>(willingInteractionEntity);

		const btVector3& posInteractionWithEntity = pPhysicsInteractionWithComponent->rigid_body->getCenterOfMassPosition();
		const btVector3& posWillInteractionEntity = pPhysicsWillInteractionComponent->rigid_body->getCenterOfMassPosition();
#define DISTANCE_MIN_INTERACTION 10.f
		if ((posInteractionWithEntity - posWillInteractionEntity).norm() >= DISTANCE_MIN_INTERACTION) {
			return;
		}

		// The two entities are enough near.
		// We launch the interaction script of the interactionWithEntity if no script is currently running
		std::weak_ptr<ScriptSystem> scriptSystem = GameProgram::game->systems.system<ScriptSystem>();
		entityx::EventManager& ev = GameProgram::game->events;
		if (auto scriptSystemPtr = scriptSystem.lock()) {
			if (!scriptSystemPtr->isRunningScriptFrom(interactionWithEntity)) {
				ev.emit<LaunchEvent>({ interactionWithEntity, Script::INTERACTION });
				std::cout << "Launching interaction script !" << std::endl;
			}
		}

		if (hasComponent<Carryable>(interactionWithEntity) && hasComponent<Handler>(willingInteractionEntity)) {
			Component<Handler> pHandlerWillInteractionComponent = getComponent<Handler>(willingInteractionEntity);

			Component<Physics> pPhysicsWillInteractionComponent = getComponent<Physics>(willingInteractionEntity);
			Component<Physics> pPhysicsInteractionWithComponent = getComponent<Physics>(interactionWithEntity);

			EntityHierarchyManager& entityHierarchyManager = EntityHierarchyManager::getInstance();

			if (pHandlerWillInteractionComponent->left_arm.valid() && pHandlerWillInteractionComponent->left_arm != interactionWithEntity) {
				std::cout << "drop" << std::endl;
				entityx::Entity droppedLeftArmEntity = pHandlerWillInteractionComponent->left_arm;
				entityHierarchyManager.remove(willingInteractionEntity);

				Component<Physics> pPhysicsDroppedEntityComponent = getComponent<Physics>(droppedLeftArmEntity);
				pPhysicsWillInteractionComponent->rigid_body->setIgnoreCollisionCheck(pPhysicsDroppedEntityComponent->rigid_body, false);
			}

			pHandlerWillInteractionComponent->left_arm = interactionWithEntity;
			
			Component<Carryable> pCarryableInteractionWithComponent = getComponent<Carryable>(interactionWithEntity);
			EntityHierarchyPtr handlerHierarchy = make_unique<EntityHierarchy>(willingInteractionEntity,
				pPhysicsWillInteractionComponent->rigid_body->getWorldTransform());

			EntityHierarchyPtr carriedEntityNode = make_unique<EntityHierarchy>(interactionWithEntity,
				pCarryableInteractionWithComponent->local_tr);

			handlerHierarchy->addChild(move(carriedEntityNode));
			entityHierarchyManager.insertCopy(willingInteractionEntity, std::move(handlerHierarchy));

			// Disable collision between the WillInteraction and InteractionWith rigid bodies
			pPhysicsWillInteractionComponent->rigid_body->setIgnoreCollisionCheck(pPhysicsInteractionWithComponent->rigid_body, true);
		}

		/*if (interactionWithEntity.component<Carryable>()) {
			const btTransform& local_transform = interactionWithEntity.component<Carryable>()->local_tr;

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
			player_handler->left_arm = interactionWithEntity;

			player_compound_shape->addChildShape(local_transform, interactionWithEntity.component<Physics>()->collision_shape);

			player_physics->mass += interactionWithEntity.component<Physics>()->mass;
			// Recomputation of the inertia of the player compound shape
			btVector3 inertia;
			player_compound_shape->calculateLocalInertia(player_physics->mass, inertia);
			player_body->setMassProps(player_physics->mass, inertia);
			player_body->updateInertiaTensor();

			world.dynamic_world->removeRigidBody(interactionWithEntity.component<Physics>()->rigid_body);
		}*/
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
	}
	
	// Returns true if picked entity is at a minimal distance of interaction. Returns false otherwise (no picked entity or
	// too far).
	static bool isEntityPerInteraction(entityx::Entity& entity, const InputHandler& input_handler, const Viewer& viewer) {
		bool hit = false;
		btVector3 intersection_point;
		const std::string& name_entity = PickingSystem::getPickedEntity(input_handler, *GameProgram::m_current_viewer, hit, intersection_point);
		if (hit) {
			World& world = Singleton<World>::getInstance();
			entity = world.get(name_entity);
			return true;
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