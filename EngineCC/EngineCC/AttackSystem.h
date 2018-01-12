#pragma once

#include <entityx/entityx.h>
#include "Components.h"
#include "Viewer.h"

struct AttackEvent {
	entityx::Entity attacker;
};

/// AttackSystem definifion
class AttackSystem : public entityx::System<AttackSystem>, public entityx::Receiver<AttackSystem> {
public:
	AttackSystem() {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<AttackEvent>(*this);
	}

	void receive(const AttackEvent &event) {
		assert(event.attacker.valid());
		entityx::Entity attacker = event.attacker;
		if (has_component<Handler>(attacker)) {
			Component<Handler> handler = getComponent<Handler>(attacker);

			// Check if a left arm entity is currently equipped
			if (handler->left_arm.valid()) {
				// The left arm is supposed to have a Physics component because it
				// has been picked up 
				Component<Physics> left_physics = getComponent<Physics>(handler->left_arm);
				btScalar angle = 0;
				/*std::cout << "aaa" << std::endl;
				PhysicHingeConstraint::add(handler->left_arm, left_physics.get(), btVector3(1, 0, 0), btVector3(0, 0, 0), btVector3(100, 0, 0), angle - M_PI * 0.5, angle + M_PI * 0.5);
				std::cout << "bbb" << std::endl;

				PhysicHingeConstraint* constraint = (PhysicHingeConstraint*)left_physics->constraints["pivot"];
				std::cout << "ccc" << std::endl;
				constraint->start(constraint->getLowerLimitAngle(), constraint->getUpperLimitAngle());
				std::cout << "ddd" << std::endl;*/
			}
		}

		attackers.insert(event.attacker);
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
	}

	/*void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		es.each<Handler>([this](entityx::Entity entity, Handler& handler) {
			
		});
	}*/
private:
	std::set<entityx::Entity> attackers;
};
