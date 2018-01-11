#pragma once
#include <memory>
#include <map>

#include "btBulletDynamicsCommon.h"
#include "Renderable.h"
#include "FiniteStateMachine.h"
#include "HierarchicalEntities.h"

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
	// - INTERACTION_CARRYABLE : the script is launched when the player clic on the mouse. Ths script of the carried entity is launched
	// This can be a magical spell that do crazy things, instantiate multi entities such as fireballs. This can be the effect of a potion that
	// can regain life, enhance the speed of the player, allow him to jump higher. In case of non-identified object it could do nasty things on the 
	// player such as launch a blur shader to disable the view for the player, create instant damage, dots, launch strange shaders (see in black-white)...
	enum Activation {
		INTERACTION,
		ATTACK,
		LOCATION,
		INTERACTION_CARRYABLE
	};

	std::map<Activation, std::weak_ptr<FiniteStateMachine>> m_scripts;
};

struct Carryable {
	std::string name;
	std::string description;
	// Objects that are not dentified can be carried and used but it could affect the user of strange effects.
	bool identified;

	// Local transform in its parent basis
	btTransform local_tr;
};

// Entities can be equipped of two weapons, swords, spears, a shield or just a potion
struct Handler {
	entityx::Entity left_arm;
	entityx::Entity right_arm;
};