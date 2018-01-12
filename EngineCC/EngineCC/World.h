#pragma once
#include <string>
#include <map>
#include <algorithm>

#include <entityx/entityx.h>
#include "btBulletDynamicsCommon.h"

#include "Components.h"
#include "PhysicConstraint.h"

class World {
public:
	World() {
		/// Definitions of the dynamic world
		// Discrete dynamic world instanciation
		m_collision_configuration = new btDefaultCollisionConfiguration();
		m_dispatcher = new btCollisionDispatcher(m_collision_configuration);
		m_overlapping_pair_cache = new btDbvtBroadphase();
		m_solver = new btSequentialImpulseConstraintSolver();

		dynamic_world = new btDiscreteDynamicsWorld(m_dispatcher,
			m_overlapping_pair_cache,
			m_solver,
			m_collision_configuration);

		std::cout << "Dynamic world instanciated" << std::endl;
	}
	~World() {
		//delete dynamics world
		delete dynamic_world;

		//delete solver
		delete m_solver;
		//delete broadphase
		delete m_overlapping_pair_cache;
		//delete dispatcher
		delete m_dispatcher;
		delete m_collision_configuration;
	}

	void free() {
		for (std::map<std::string, entityx::Entity>::iterator it = m_entities.begin(); it != m_entities.end(); it++) {
			entityx::Entity entity = it->second;
			removeFromWorld(entity);
			entity.destroy();
		}
		m_entities.clear();
	}

	// Add an entity including a Physics component to the dynamic world
	// Precondition : the entity has a Physics component
	void addEntity(const std::string& name, entityx::Entity entity) {
		assert(entity.valid());
		if (m_entities.find(name) != m_entities.end()) {
			deleteEntity(name);
		}

		Physics* physic = entity.component<Physics>().get();
		if (physic == NULL)
			return;
		dynamic_world->addRigidBody(physic->rigid_body);
		
		// Add entity constraints to the world
		/*for (std::map<std::string, PhysicConstraint*>::iterator it = physic->constraints.begin(); it != physic->constraints.end(); ++it) {
			dynamic_world->addConstraint(it->second->getTypedConstraint());
		}*/

		m_entities[name] = entity;
	}

	// Delete an entity including a Physics component to the dynamic world
	// Precondition : the entity has a Physics component
	void deleteEntity(const std::string& name) {
		if (m_entities.find(name) == m_entities.end())
			return;
		entityx::Entity entity = m_entities[name];
		removeFromWorld(entity);

		m_entities[name].destroy();
		m_entities.erase(name);
	}

	entityx::Entity get(const std::string& name) {
		assert(m_entities.find(name) != m_entities.end());
		assert(m_entities[name].valid());
		return m_entities[name];
	}

	bool isEntityPicked(const btVector3& btFrom, const btVector3& btTo, std::string& hit_entity, btVector3& I) const {
		// Before doing a picking, we ensure all AABBs are updated 
		// because the editor do not call the physic system that process a stepSimulation
		dynamic_world->updateAabbs();

		btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);
		dynamic_world->rayTest(btFrom, btTo, res);
		bool hit = res.hasHit();
		if (hit) {
			I = res.m_hitPointWorld;

			const btCollisionObject* obj_collided = res.m_collisionObject;
			for (std::map<std::string, entityx::Entity>::const_iterator it = m_entities.cbegin(); it != m_entities.cend(); it++) {
				entityx::Entity entity = it->second;
				const std::string& name = it->first;

				entityx::ComponentHandle<Physics> physics = entity.component<Physics>();
				if (physics->rigid_body == obj_collided) {
					hit_entity = name;

					return true;
				}
			}
		}
		
		return false;
	}

	void setGroupMaskCollision(entityx::Entity entity, int group, int mask) {
		entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
		dynamic_world->removeCollisionObject(physic->rigid_body);
		dynamic_world->addRigidBody(physic->rigid_body, group, mask);
	}


private:
	void removeFromWorld(entityx::Entity entity) {
		entityx::ComponentHandle<Physics> physic = entity.component<Physics>();

		btRigidBody* rigid_body = physic->rigid_body;
		if (rigid_body && rigid_body->getMotionState())
			delete rigid_body->getMotionState();

		dynamic_world->removeCollisionObject(rigid_body);

		delete physic->collision_shape;

		/*for (std::map<std::string, PhysicConstraint*>::iterator it = physic->constraints.begin(); it != physic->constraints.end(); ++it) {
			delete it->second;
		}*/
	}
public:
	/// Bullet dynamic world
	btDiscreteDynamicsWorld* dynamic_world;
private:

	btDefaultCollisionConfiguration* m_collision_configuration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_overlapping_pair_cache;
	btSequentialImpulseConstraintSolver* m_solver;

	std::map<std::string, entityx::Entity> m_entities;
};
