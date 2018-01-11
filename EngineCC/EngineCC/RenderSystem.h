#pragma once

#include <entityx/entityx.h>
#include "Components.h"
#include "Viewer.h"



/// RenderSystem definifion
class RenderSystem : public entityx::System<RenderSystem> {
public:
	RenderSystem(const Viewer& viewer) : m_viewer(viewer) {
	}

	LocalTransform btTransformToLocalTransform(const btTransform& bt_tr, const btCollisionShape* collision_shape) const {
		LocalTransform local_tr;
		const btVector3& scale = collision_shape->getLocalScaling();
		local_tr.setTranslation(glm::vec3(bt_tr.getOrigin().x(), bt_tr.getOrigin().y(), bt_tr.getOrigin().z()));
		local_tr.setRotation(glm::quat(bt_tr.getRotation().getW(), bt_tr.getRotation().getX(), bt_tr.getRotation().getY(), bt_tr.getRotation().getZ()));
		local_tr.setScale(glm::vec3(scale.x(), scale.y(), scale.z()));

		return local_tr;
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		// Update renderable position following the computation of the motion state by bullet
		es.each<Physics, Render>([this](entityx::Entity entity, Physics& physic, Render& render) {
			btCollisionShape* collision_shape = physic.collision_shape;
			
			if (collision_shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
				// The collision is a compound of other collision shapes
				btCompoundShape* compound_shape = (btCompoundShape*)collision_shape;
				entityx::ComponentHandle<Handler> handler = entity.component<Handler>();
				if (handler && handler->left_arm.valid()) {
					const btTransform& tr = compound_shape->getChildTransform(1);
					
					entityx::ComponentHandle<Render> render_left = handler->left_arm.component<Render>();
					(*render_left)->setLocalTransform(btTransformToLocalTransform(physic.rigid_body->getWorldTransform() * tr, collision_shape));
				}
			}
			else {
				btTransform tr;
				physic.motion_state->getWorldTransform(tr);
				render->setLocalTransform(btTransformToLocalTransform(tr, collision_shape));
			}
		});

		es.each<Render>([this](entityx::Entity entity, Render& render) {
			render->draw(m_viewer);
		});
	}
private:
	const Viewer& m_viewer;
};