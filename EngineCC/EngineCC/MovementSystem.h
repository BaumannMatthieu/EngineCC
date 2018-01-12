#pragma once

/// Event Definitions
struct DisplacementEvent {
	DisplacementEvent(entityx::Entity entity, const glm::vec3& direction) : entity(entity),
		direction(direction) {
	}
	// The entity to move
	entityx::Entity entity;
	glm::vec3 direction;
};

struct StopDisplacementEvent {
	StopDisplacementEvent(entityx::Entity entity) : entity(entity) {
	}

	entityx::Entity entity;
};

/// System Definitions
class MovementSystem : public entityx::System<MovementSystem>, public entityx::Receiver<MovementSystem> {
public:
	MovementSystem() {
	}
	~MovementSystem() {
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		es.each<Physics, Movable>([this, &dt](entityx::Entity entity, Physics& physic, Movable& movable) {
			if (m_movements.find(entity) != m_movements.end()) {
				// If a MovementEvent has been retrieved we set the new linear velocity towards the direction 
				// defined in the Event and at the speed defined in the MovableComponent
				btVector3 dv(m_movements[entity].direction * movable.speed);

				btTransform new_tr = physic.rigid_body->getWorldTransform();
				new_tr.setOrigin(new_tr.getOrigin() + dv * dt);
				physic.rigid_body->setWorldTransform(new_tr);
			}
		});
	}

	void configure(entityx::EventManager &events) override {
		events.subscribe<DisplacementEvent>(*this);
		events.subscribe<StopDisplacementEvent>(*this);
	}

	void receive(const DisplacementEvent& move) {
		Movement movement = { btVector3(move.direction.x, move.direction.y, move.direction.z) };
		movement.direction.normalize();
		m_movements[move.entity] = movement;
	}

	void receive(const StopDisplacementEvent& stop_move) {
		m_movements.erase(stop_move.entity);
	}
private:
	struct Movement {
		btVector3 direction;
	};

	std::map<entityx::Entity, Movement> m_movements;
};
