#pragma once

#include "btBulletDynamicsCommon.h"
#include <entityx/entityx.h>

class PhysicConstraint {
public:
	PhysicConstraint(btRigidBody* body) : m_body(body),
										  m_finished(false) {
	}

	virtual ~PhysicConstraint() {
	}

	virtual void update() = 0;
	virtual void restart() = 0;
	virtual btTypedConstraint* getTypedConstraint() = 0;

	bool isFinished() const {
		return m_finished;
	}

protected:
	bool m_finished;
	btRigidBody* m_body;
};

struct Physics;

class PhysicHingeConstraint : public PhysicConstraint {
public:
	PhysicHingeConstraint(btRigidBody* body,
		const btVector3& axis,
		const btVector3& pivot,
		btScalar lower_limit,
		btScalar upper_limit);

	virtual ~PhysicHingeConstraint() {
		std::cout << "destructed" << std::endl;
		delete m_constraint;
	}

	btTypedConstraint* getTypedConstraint() {
		return m_constraint;
	}

	btScalar getUpperLimitAngle() const {
		return m_constraint->getUpperLimit();
	}

	btScalar getLowerLimitAngle() const {
		return m_constraint->getLowerLimit();
	}

	btScalar getStartAngle() const {
		return m_start_angle;
	}

	void startImpulse(btScalar offset_angle, const btVector3& torque_abs);
	void restart();
	void update();

private:
	btVector3 m_torque;
	btScalar m_from_angle;
	btScalar m_to_angle;
	btScalar m_start_angle;

	btHingeConstraint* m_constraint;
};
