#include "PhysicConstraint.h"
#include "Components.h"

PhysicHingeConstraint::PhysicHingeConstraint(entityx::Entity entity, btVector3& axis,
	const btVector3& pivot,
	const btVector3& torque,
	btScalar lower_limit,
	btScalar upper_limit) : PhysicConstraint(entity), m_torque(torque) {
	btRigidBody* body = entity.component<Physics>()->rigid_body;

	m_constraint = new btHingeConstraint(*body, pivot, axis);
	m_constraint->setLimit(lower_limit, upper_limit);
	// For debug purposes
	m_constraint->setDbgDrawSize(btScalar(5.f));
}

void PhysicHingeConstraint::start(btScalar from_angle, btScalar to_angle) {
	m_from_angle = from_angle;
	m_to_angle = to_angle;
	assert(from_angle >= m_constraint->getLowerLimit() && to_angle <= m_constraint->getUpperLimit());

	btRigidBody* body = m_entity.component<Physics>()->rigid_body;
	const btVector3& torque = (m_to_angle >= m_from_angle) ? m_torque : -m_torque;
	body->applyTorque(torque);
	m_finished = false;
}

void PhysicHingeConstraint::restart() {
	start(m_from_angle, m_to_angle);
}

void PhysicHingeConstraint::update() {
	m_finished = (abs(m_constraint->getHingeAngle() - m_to_angle) < 0.05f);
	if (m_finished) {
		btRigidBody* body = m_entity.component<Physics>()->rigid_body;
		body->applyTorque(btVector3(0, 0, 0));
		body->setAngularVelocity(btVector3(0, 0, 0));
	}
}