#pragma once
#include <string>
#include <map>

#include <entityx/entityx.h>
#include "Components.h"
#include "World.h"
#include "Singleton.h"

using namespace std;
using namespace entityx;

/// AddConstraint event
struct AddConstraint {
	AddConstraint(Entity constrained_entity, const string& constraint_name, PhysicConstraint* pConstraint) :
		constrained_entity(constrained_entity), constraint_name(constraint_name), pConstraint(pConstraint) {
	}

	Entity constrained_entity;
	string constraint_name;
	PhysicConstraint* pConstraint;
};

struct StartImpulseHinge {
	StartImpulseHinge(Entity constrained_entity, const string& constraint_name, btScalar angle_offset, const btVector3& torque) :
		constrained_entity(constrained_entity), constraint_name(constraint_name), angle_offset(angle_offset), torque(torque) {
	}

	Entity constrained_entity;
	string constraint_name;

	btScalar angle_offset;
	btVector3 torque;
};
struct StartImpulseSlider;
struct StartImpulsePoint2point;



/// AttackSystem definifion
class PhysicConstraintSystem : public System<PhysicConstraintSystem>, public Receiver<PhysicConstraintSystem> {
public:
	PhysicConstraintSystem() {
	}
	~PhysicConstraintSystem() {
		// Remove all the constraints from the world
		for (map<Entity, map<string, PhysicConstraint*>>::iterator it = m_world_constraints.begin();
			it != m_world_constraints.end(); ++it) {
			map<string, PhysicConstraint*>& constraints = it->second;
			for (map<string, PhysicConstraint*>::iterator it2 = constraints.begin(); it2 != constraints.end(); ++it2) {
				const string& constraint_name = it2->first;
				removeConstraint(constraints, constraint_name);
				
				if (constraints.empty()) {
					break;
				}
			}
		}
	}

	void configure(EventManager &event_manager) {
		event_manager.subscribe<AddConstraint>(*this);
		event_manager.subscribe<StartImpulseHinge>(*this);
	}

	void receive(const AddConstraint &event) {
		addConstraint(m_world_constraints[event.constrained_entity], event.constraint_name, event.pConstraint);
	}

	void receive(const StartImpulseHinge &event) {
		const string& constraint_name = event.constraint_name;
		// We assert that the constraint name refers to a hinge constraint
		assert(constraint_name.find("pivot") != string::npos);

		Entity entity_to_impulse = event.constrained_entity;
		// We assert the constraint exists
		assert(isConstraintExists(m_world_constraints[entity_to_impulse], constraint_name));
		
		PhysicConstraint* pConstraint = m_world_constraints[entity_to_impulse][constraint_name];
		// The constraint exists and is of hinge type, we can cast it
		PhysicHingeConstraint* pHingeConstraint = (PhysicHingeConstraint*)pConstraint;
		pHingeConstraint->startImpulse(event.angle_offset, event.torque);
	}

	// Add a new constraint to the world. If the constraint already exist, we exit the procedure and do not replace it by the new one
	// The user just need to change the constraint but do not have to replace it.
	void addConstraint(map<string, PhysicConstraint*>& entity_constraints, const string& constraint_name, PhysicConstraint* pConstraint) {
		// If the constraint already exists we exit
		if (isConstraintExists(entity_constraints, constraint_name)) {
			return;
		}
		
		removeConstraint(entity_constraints, constraint_name);
		entity_constraints.insert(std::pair<string, PhysicConstraint*>(constraint_name, pConstraint));

		World& world = Singleton<World>::getInstance();
		world.dynamic_world->addConstraint(pConstraint->getTypedConstraint());
	}

	// Remove an already existing constraint from the world
	void removeConstraint(map<string, PhysicConstraint*>& entity_constraints, const string& constraint_name) {
		if (!isConstraintExists(entity_constraints, constraint_name)) {
			return;
		}
		PhysicConstraint* pConstraint = entity_constraints[constraint_name];
		entity_constraints.erase(constraint_name);

		World& world = Singleton<World>::getInstance();
		world.dynamic_world->removeConstraint(pConstraint->getTypedConstraint());
		
		delete pConstraint;
	}

	void update(EntityManager &es, EventManager &events, TimeDelta dt) override {
		for (map<Entity, map<string, PhysicConstraint*>>::const_iterator it = m_world_constraints.cbegin();
			it != m_world_constraints.cend(); ++it) {
			const map<string, PhysicConstraint*>& constraints = it->second;
			for (map<string, PhysicConstraint*>::const_iterator it2 = constraints.cbegin(); it2 != constraints.cend(); ++it2) {
				PhysicConstraint* pConstraint = it2->second;
				pConstraint->update();
				//std::cout << "azeaz" << std::endl;
			}
		}
	}

	bool isFinished(Entity entity, const string& constraint_name) {
		// We assert the constraint exists
		assert(isConstraintExists(m_world_constraints[entity], constraint_name));
		PhysicConstraint* pConstraint = m_world_constraints[entity][constraint_name];
		
		return pConstraint->isFinished();
	}

	btScalar getStartAngle(Entity entity, const string& constraint_name) {
		// We assert the constraint exists
		assert(isConstraintExists(m_world_constraints[entity], constraint_name));
		PhysicConstraint* pConstraint = m_world_constraints[entity][constraint_name];

		// We assert the constraint is of hinge type
		assert(constraint_name.find("pivot") != string::npos);
		PhysicHingeConstraint* pHingeConstraint = (PhysicHingeConstraint*)pConstraint;

		return pHingeConstraint->getStartAngle();
	}

private:
	bool isConstraintExists(const map<string, PhysicConstraint*>& entity_constraints, const std::string& constraint_name) {
		return (entity_constraints.find(constraint_name) != entity_constraints.end());
	}

private:
	map<Entity, map<string, PhysicConstraint*>> m_world_constraints;
};
