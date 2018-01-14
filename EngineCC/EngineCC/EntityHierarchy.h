#pragma once

#include <entityx/entityx.h>
#include <vector>
#include "btBulletDynamicsCommon.h"

using namespace std;

class EntityHierarchy {
public:
	EntityHierarchy(entityx::Entity root, const btTransform& local);
	~EntityHierarchy();

	void addChild(unique_ptr<EntityHierarchy> child);
	void deleteChild(entityx::Entity entity);

	void setLocalTransform(const btTransform& local);
	void computeTransformHierarchy(const btTransform& parent_transform = btTransform::getIdentity());

private:
	entityx::Entity m_entity;
	btTransform m_local;
	// Transformation matrix in the local space 
	vector<unique_ptr<EntityHierarchy>> m_children;
};

using EntityHierarchyPtr = std::unique_ptr<EntityHierarchy>;