#pragma once

#include <entityx/entityx.h>
#include "PhysicConstraintSystem.h"
#include "Components.h"
#include "Viewer.h"
#include <set>

using namespace entityx;
using namespace std;

struct AttackEvent {
	Entity attacker;
};

/// AttackSystem definifion
class AttackSystem : public System<AttackSystem>, public Receiver<AttackSystem> {
public:
	AttackSystem() {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<AttackEvent>(*this);
	}

	void receive(const AttackEvent &event) {
		assert(event.attacker.valid());
		entityx::Entity attacker = event.attacker;
		if (hasComponent<Handler>(attacker)) {
			ComponentHandle<Handler> handler = getComponent<Handler>(attacker);

			// Check if a left arm entity is currently equipped
			if (handler->left_arm.valid()) {
				Entity mainCarriedEntity = handler->left_arm;
				EventManager& eventManager = GameProgram::game->events;

				btVector3 axis(1, 0, 0);
				btVector3 pivot(0, 0, 0);

				eventManager.emit<AddConstraint>(AddConstraint(mainCarriedEntity,
					"pivotX",
					new PhysicHingeConstraint(getComponent<Physics>(mainCarriedEntity)->rigid_body, axis, pivot, -M_PI, M_PI)));

				
				eventManager.emit<StartImpulseHinge>(StartImpulseHinge(mainCarriedEntity, "pivotX", M_PI / 4, btVector3(20, 0, 0)));
				std::cout << GameProgram::game->systems.system<PhysicConstraintSystem>()->isFinished(mainCarriedEntity, "pivotX") << std::endl;
			}
		}

		attackers.insert(event.attacker);
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		for (set<Entity>::iterator it = attackers.begin(); it != attackers.end(); it++) {
			ComponentHandle<Physics> pPhysicAttackerComponent = getComponent<Physics>(*it);
			btCompoundShape* compound_shape = (btCompoundShape*)pPhysicAttackerComponent->collision_shape;
			ComponentHandle<Handler> pHandlerAttackerComponent = getComponent<Handler>(*it);

			if (pHandlerAttackerComponent->left_arm.valid()) {
				Entity mainCarriedEntity = pHandlerAttackerComponent->left_arm;
				btTransform& rigidBodyTransform = getComponent<Physics>(mainCarriedEntity)->rigid_body->getWorldTransform();
				btTransform& oldTransform = compound_shape->getChildTransform(1);
				btTransform newTransform = oldTransform;
				newTransform.setRotation(rigidBodyTransform.getRotation());
				
				//rigidBodyTransform.setOrigin(oldTransform.getOrigin());
				compound_shape->updateChildTransform(1, newTransform);
				//Singleton<World>::getInstance().dynamic_world->updateAabbs();
			}
		}
	}

private:
	std::set<Entity> attackers;
};
