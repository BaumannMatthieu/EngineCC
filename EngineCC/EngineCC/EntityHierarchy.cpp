#pragma once

#include "EntityHierarchy.h"
#include "Components.h"
#include "World.h"
#include "Singleton.h"

EntityHierarchy::EntityHierarchy(entityx::Entity root, const btTransform& local) : m_entity(root), m_local(local) {
	assert(m_entity.component<Physics>());
}

EntityHierarchy::~EntityHierarchy() {
	// The entity can be affected by the gravity when it is deleted from a hierarchy (i.e. drop of an entity)
	if(m_entity.valid())
		m_entity.component<Physics>()->rigid_body->setLinearFactor(btVector3(1, 1, 1));

	std::cout << "delete entity hierarchy" << std::endl;
}

void EntityHierarchy::setLocalTransform(const btTransform& local) {
	m_local = local;
}

void EntityHierarchy::addChild(unique_ptr<EntityHierarchy> child) {
	// TODO : When adding a child entity to the tree
	// we need to assert the entity is not already in it

	// When we an entity in a hierarchy, we delete the effect of the gravity on it. We do not want the
	// entity to go down over the time.
	child->m_entity.component<Physics>()->rigid_body->setLinearFactor(btVector3(1, 0, 1));
	// We remove the ability to the added entity to enter in collision with other entity of the hierarchy
	World& world = Singleton<World>::getInstance();

	m_children.push_back(move(child));
}

void EntityHierarchy::deleteChild(entityx::Entity entity) {
	for (vector<unique_ptr<EntityHierarchy>>::iterator it = m_children.begin(); it != m_children.end(); it++) {
		if (entity == (*it)->m_entity) {
			it = m_children.erase(it);
			return;
		}
	}

	for (vector<unique_ptr<EntityHierarchy>>::iterator it = m_children.begin(); it != m_children.end(); it++) {
		(*it)->deleteChild(entity);
	}
}

void EntityHierarchy::computeTransformHierarchy(const btTransform& parent_transform) {
	btTransform global_transform = parent_transform * m_local;

	entityx::ComponentHandle<Physics> physics = m_entity.component<Physics>();
	physics->rigid_body->setWorldTransform(global_transform);
	//physics->motion_state->setWorldTransform(global_transform);

	for (vector<unique_ptr<EntityHierarchy>>::const_iterator it = m_children.cbegin(); it != m_children.cend(); it++) {
		(*it)->computeTransformHierarchy(global_transform);
	}
}

