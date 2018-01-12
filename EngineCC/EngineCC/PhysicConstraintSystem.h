#pragma once
#include <string>
#include <map>

#include <entityx/entityx.h>
#include "Components.h"
#include "World.h"
#include "Singleton.h"

/// AddConstraint event
struct AddConstraint {
	AddConstraint(entityx::Entity constrained_entity, const std::string& constraint_name, PhysicConstraint* pConstraint) :
		constrained_entity(constrained_entity), constraint_name(constraint_name), pConstraint(pConstraint) {
	}

	entityx::Entity constrained_entity;
	std::string constraint_name;
	PhysicConstraint* pConstraint;
};

struct StartImpulseHinge {
	StartImpulseHinge(entityx::Entity constrained_entity, const std::string& constraint_name, btScalar to_angle) :
		constrained_entity(constrained_entity), constraint_name(constraint_name), to_angle(to_angle) {
	}

	entityx::Entity constrained_entity;
	std::string constraint_name;

	btScalar to_angle;
};
struct StartImpulseSlider;
struct StartImpulsePoint2point;

using namespace std;
using namespace entityx;

/// AttackSystem definifion
class PhysicConstraintSystem : public System<PhysicConstraintSystem>, public Receiver<PhysicConstraintSystem> {
public:
	PhysicConstraintSystem() {
	}
	~PhysicConstraintSystem() {
		// Remove all the constraints from the world
	}

	void configure(EventManager &event_manager) {
		event_manager.subscribe<AddConstraint>(*this);
		event_manager.subscribe<StartImpulseHinge>(*this);
	}

	void receive(const AddConstraint &event) {
		std::cout << m_world_constraints.size() << std::endl;
		addConstraint(m_world_constraints[event.constrained_entity], event.constraint_name, event.pConstraint);
	}

	void receive(const StartImpulseHinge &event) {
		const string& constraint_name = event.constraint_name;
		// We assert that the constraint name refers to a hinge constraint
		assert(constraint_name.find("pivot") != string::npos);

		entityx::Entity entity_to_impulse = event.constrained_entity;
		// We assert the constraint exists
		std::cout << m_world_constraints.size() << std::endl;
		assert(isConstraintExists(m_world_constraints[entity_to_impulse], constraint_name));
		
		PhysicConstraint* pConstraint = m_world_constraints[entity_to_impulse][constraint_name];
		// The constraint exists and is of hinge type, we can cast it
		PhysicHingeConstraint* pHingeConstraint = (PhysicHingeConstraint*)pConstraint;
		pHingeConstraint->startImpulse(event.to_angle);
	}

	// Add a new constraint to the world
	void addConstraint(map<string, PhysicConstraint*>& entity_constraints, const string& constraint_name, PhysicConstraint* pConstraint) {
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

private:
	bool isConstraintExists(const map<string, PhysicConstraint*>& entity_constraints, const std::string& constraint_name) {
		return (entity_constraints.find(constraint_name) != entity_constraints.end());
	}

private:
	map<Entity, map<string, PhysicConstraint*>> m_world_constraints;
};
