#pragma once
#include <memory>

#include "btBulletDynamicsCommon.h"
#include "Renderable.h"
#include "FiniteStateMachine.h"

/// Component Definitions
using Render = std::shared_ptr<RenderObject>;

struct Physics {
	// if nullptr => no collision
	btCollisionShape* collision_shape;
	// One constraint per entity possible
	btDefaultMotionState* motion_state;
	btRigidBody* rigid_body;

	float mass;
	btVector3 local_inertia;

	btTypedConstraint* constraint;

	Physics& operator=(const Physics& other) {
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

struct Script {
	// An entity can have multiple scripts that can be triggered
	// in different ways :
	// - INTERACTION : the player interact with the entity by pressing 'E'
	// - ATTACK : the player attack the entity. The entity can response by launching a special ATTACK script
	// that depicts its strategy during the fight
	// - LOCATION : the script is launched when the player is in a particular location
	enum Activation {
		INTERACTION,
		ATTACK,
		LOCATION
	};

	std::map<Activation, std::weak_ptr<FiniteStateMachine>> m_scripts;
};