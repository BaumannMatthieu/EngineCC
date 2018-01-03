#pragma once
#include <memory>

#include "btBulletDynamicsCommon.h"
#include "Renderable.h"

/// Component Definitions
using Render = std::shared_ptr<RenderObject>;

struct Physics {
	// if nullptr => no collision
	btCollisionShape* collision_shape;
	btDefaultMotionState* motion_state;
	btRigidBody* rigid_body;

	float mass;
	btVector3 local_inertia;
};

struct Movable {
	Movable(float speed) : speed(speed) {
	}

	float speed;
	//Pathfinding list of places to go
};