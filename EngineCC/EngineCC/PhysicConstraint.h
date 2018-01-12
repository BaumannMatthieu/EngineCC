#pragma once

#include "btBulletDynamicsCommon.h"
#include <entityx/entityx.h>

class PhysicConstraint {
public:
	PhysicConstraint(entityx::Entity entity) : m_entity(entity), m_finished(false) {
		assert(entity.valid());
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
	entityx::Entity m_entity;
};

class PhysicHingeConstraint : public PhysicConstraint {
public:
	PhysicHingeConstraint(entityx::Entity entity, btVector3& axis,
		const btVector3& pivot,
		const btVector3& torque,
		btScalar lower_limit,
		btScalar upper_limit);

	~PhysicHingeConstraint() {
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

	void start(btScalar from_angle, btScalar to_angle);
	void restart();
	void update();

private:
	btVector3 m_torque;
	btScalar m_from_angle;
	btScalar m_to_angle;

	btHingeConstraint* m_constraint;
};
