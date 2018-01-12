#include "PhysicConstraint.h"
#include "Components.h"

PhysicHingeConstraint::PhysicHingeConstraint(btRigidBody* body, const btVector3& axis,
	const btVector3& pivot,
	const btVector3& torque,
	btScalar lower_limit,
	btScalar upper_limit) : PhysicConstraint(body), m_torque(torque) {
	m_constraint = new btHingeConstraint(*body, pivot, axis);
	m_start_angle = m_constraint->getHingeAngle();
	m_constraint->setLimit(lower_limit, upper_limit);
	// For debug purposes
	m_constraint->setDbgDrawSize(btScalar(5.f));
}

void PhysicHingeConstraint::startImpulse(btScalar to_angle) {
	m_from_angle = m_constraint->getHingeAngle();
	
	m_to_angle = to_angle;
	assert(m_from_angle >= m_constraint->getLowerLimit() && to_angle <= m_constraint->getUpperLimit());

	const btVector3& torque = (m_to_angle >= m_from_angle) ? m_torque : -m_torque;
	m_body->applyTorque(torque);
	m_finished = false;
}

void PhysicHingeConstraint::restart() {
	startImpulse(m_to_angle);
}

void PhysicHingeConstraint::update() {
	m_finished = (abs(m_constraint->getHingeAngle() - m_to_angle) < 0.05f);
	if (m_finished) {
		m_body->applyTorque(btVector3(0, 0, 0));
		m_body->setAngularVelocity(btVector3(0, 0, 0));
	}
}
