#pragma once

#include <vector>
#include <unordered_set>

#include <entityx/entityx.h>

#include "btBulletDynamicsCommon.h"

#include "GameProgram.h"

#include "PhysicConstraint.h"
#include "Singleton.h"
#include "Manager.h"
#include "Components.h"
#include "Shader.h"
#include "Cube.h"

using namespace std;

/// Physic System definition
class PhysicSystem : public entityx::System<PhysicSystem> {
public:
	/// DebugDrawer Bullet
	//// Helper class; draws the world as seen by Bullet.
	//// This is very handy to see it Bullet's world matches yours.
	//// This example uses the old OpenGL API for simplicity, 
	//// so you'll have to remplace GLFW_OPENGL_CORE_PROFILE by
	//// GLFW_OPENGL_COMPAT_PROFILE in glfwWindowHint()
	//// How to use this class :
	//// Declare an instance of the class :
	//// BulletDebugDrawer_DeprecatedOpenGL mydebugdrawer;
	//// dynamicsWorld->setDebugDrawer(&mydebugdrawer);
	//// Each frame, call it :
	//// mydebugdrawer.SetMatrices(ViewMatrix, ProjectionMatrix);
	//// dynamicsWorld->debugDrawWorld();
	class BulletDebugDrawer : public btIDebugDraw {
	public:

		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
			glm::vec3 from_glm(from.x(), from.y(), from.z());
			glm::vec3 to_glm(to.x(), to.y(), to.z());
			glm::vec4 color_glm(color.x(), color.y(), color.z(), 1.0);

			m_points.push_back(from_glm);
			m_points.push_back(to_glm);

			m_colors.push_back(color_glm);
			m_colors.push_back(color_glm);
		}

		void draw() {
			Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
			Renderable<LinePrimitive> lines(shaders.get("debug_bullet"), m_points, m_colors);

			const Viewer* viewer = GameProgram::m_current_viewer;
			if (viewer != nullptr)
				lines.draw(*viewer);

			m_points.clear();
			m_colors.clear();
		}

		virtual void drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) {}
		virtual void reportErrorWarning(const char *) {}
		virtual void draw3dText(const btVector3 &, const char *) {}
		virtual void setDebugMode(int p) {
			m = p;
		}
		int getDebugMode(void) const { return 3; }
		int m;

	public:
		std::vector<glm::vec3> m_points;
		std::vector<glm::vec4> m_colors;
	};
public:
	struct YourOwnFilterCallback : public btOverlapFilterCallback
	{
		// return true when pairs need collision
		virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
		{
			bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
			collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

			//add some additional logic here that modified 'collides'
			return collides;
		}
	};

	// Receive entities so that we add them to the dynamic world
	// entities that will be instanciated during the game will be send
	// to the PhysicSystem by a special event that will add them too.
	PhysicSystem(entityx::EntityManager &es, btDiscreteDynamicsWorld& dynamic_world) : m_entities(es), m_dynamic_world(dynamic_world) {
		// Setting of the debug drawer to the dynamic world
		BulletDebugDrawer& debug_drawer = Singleton<BulletDebugDrawer>::getInstance();
		m_dynamic_world.setDebugDrawer(&debug_drawer);

		m_dynamic_world.setGravity(btVector3(0, -10.0, 0));
	}

	~PhysicSystem() {
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		EntityHierarchyManager& entityHierarchyManager = EntityHierarchyManager::getInstance();
		map<entityx::Entity, EntityHierarchyPtr>& entityHierarchies = entityHierarchyManager.getRessources();
		for (map<entityx::Entity, EntityHierarchyPtr>::iterator it = entityHierarchies.begin(); it != entityHierarchies.end(); ++it) {
			EntityHierarchyPtr& entityHierarchy = it->second;
			entityx::Entity rootEntity = it->first;

			Component<Physics> pPhysicsRootEntityComponent = getComponent<Physics>(rootEntity);
			btTransform& rootEntityTransform = pPhysicsRootEntityComponent->rigid_body->getWorldTransform();
			entityHierarchy->setLocalTransform(rootEntityTransform);

			entityHierarchy->computeTransformHierarchy();
		}

		m_dynamic_world.stepSimulation(dt);

		//Draw the debugging bullet world
		m_dynamic_world.debugDrawWorld();
		BulletDebugDrawer& debug_drawer = Singleton<BulletDebugDrawer>::getInstance();
		debug_drawer.draw();
	}

private:
	btDiscreteDynamicsWorld& m_dynamic_world;

	// Reference to the entity manager for deallocating all Physics components
	entityx::EntityManager& m_entities;
};