#pragma once

#include <entityx/entityx.h>
#include "Components.h"
#include "Viewer.h"

/// RenderSystem definifion
class RenderSystem : public entityx::System<RenderSystem> {
public:
	RenderSystem(const Viewer& viewer) : m_viewer(viewer) {
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		// Update renderable position following the computation of the motion state by bullet
		es.each<Physics, Render>([this](entityx::Entity entity, Physics& physic, Render& render) {
			const btTransform& tr = physic.rigid_body->getWorldTransform();
			const btVector3& scale = physic.collision_shape->getLocalScaling();
			LocalTransform render_tr = render->getLocalTransform();
			render_tr.setTranslation(glm::vec3(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z()));
			render_tr.setRotation(glm::quat(tr.getRotation().getW(), tr.getRotation().getX(), tr.getRotation().getY(), tr.getRotation().getZ()));
			render_tr.setScale(glm::vec3(scale.x(), scale.y(), scale.z()));
			render->setLocalTransform(render_tr);
		});

		es.each<Render>([this](entityx::Entity entity, Render& render) {
			render->draw(m_viewer);
		});
	}
private:
	const Viewer& m_viewer;
};