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

	Physics& operator=(const Physics& other) {
		std::cout << "HELOO" << std::endl;
		if (&other == this)
			return *this;

		collision_shape = new btConvexHullShape(dynamic_cast<btConvexHullShape&>(*(other.collision_shape)));
		motion_state = new btDefaultMotionState(*(other.motion_state));
		rigid_body = new btRigidBody(*(other.rigid_body));

		mass = other.mass;
		local_inertia = other.local_inertia;
		return *this;
	}
};

struct Movable {
	Movable(float speed) : speed(speed) {
	}

	float speed;
	//Pathfinding list of places to go
};