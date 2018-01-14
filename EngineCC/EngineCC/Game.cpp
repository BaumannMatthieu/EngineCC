#include <iostream>
#include <vector>
#include <memory>

#include "Game.h"
#include "World.h"
#include "GameProgram.h"
#include "InputHandler.h"
#include "Manager.h"

#include "Components.h"

#include "RenderSystem.h"
#include "PhysicSystem.h"
#include "ScriptSystem.h"
#include "PickingSystem.h"
#include "AttackSystem.h"
#include "PhysicConstraintSystem.h"
#include "MovementSystem.h"

#include <entityx/entityx.h>
#include <glm/gtx/vector_angle.hpp>

template<typename T>
using SystemPtr = std::shared_ptr<entityx::System<T>>;

using PhysicConstraintSystemPtr = std::shared_ptr<PhysicConstraintSystem>;

void Game::createPlayerEntity(entityx::EntityManager &es, World& world) {
	entityx::Entity entity = es.create();

	// Render Component
	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	std::shared_ptr<Renderable<Cube>> ground_render = std::make_shared<Renderable<Cube>>(shaders.get("simple"));

	LocalTransform tr;
	ground_render->setLocalTransform(tr);
	ground_render->setInvisible();
	entity.assign<Render>(ground_render);

	// Physics Component
	glm::vec3 size_box(1.5f, 2.5f, 1.5f);
	//btCollisionShape* entity_shape = new btBoxShape(btVector3(size_box.x / 2.f, size_box.y / 2.f, size_box.z / 2.f));
	btCollisionShape* entity_shape = new btCapsuleShape(size_box.x/2, size_box.y);
	btTransform entity_tr;
	entity_tr.setIdentity();
	entity_tr.setOrigin(btVector3(m_viewer.getPosition().x, m_viewer.getPosition().y, m_viewer.getPosition().z));
	btScalar mass(1.f);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static

	btCompoundShape* compound = new btCompoundShape();
	compound->addChildShape(btTransform::getIdentity(), entity_shape);

	bool is_dynamic = (mass != 0.f);
	btVector3 local_inertia(0, 0, 0);
	if (is_dynamic)
		compound->calculateLocalInertia(mass, local_inertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(entity_tr);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, compound, local_inertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	// Active rigid body
	body->setActivationState(DISABLE_DEACTIVATION);

	// Disable angular rotation when hitting another body
	body->setAngularFactor(0);

	Physics physics = { compound, motion_state, body, mass, local_inertia };
	entity.assign<Physics>(physics);

	// Movable Component
	float player_speed = 8.f;
	Movable movable(player_speed);
	entity.assign<Movable>(movable);

	// The player can carry two things at the same time, one entity for its left hand and a second for its right hand
	Handler handler;
	entity.assign<Handler>(handler);

	addEntity("player", entity);
}

// Creation of all the in-game entities in Game constructor's class
void Game::createGroundEntity(entityx::EntityManager &es) {
	entityx::Entity entity = es.create();

	// Render Component
	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	std::shared_ptr<Renderable<Plane>> ground_render = std::make_shared<Renderable<Plane>>(shaders.get("texture"));
	ground_render->setTexture("C:\\Users\\Matthieu\\Source\\Repos\\EngineCC\\EngineCC\\EngineCC\\Content\\wall3.jpg");
#define STD_UNIT_PER_TILE 4

	LocalTransform tr;
	tr.setTranslation(glm::vec3(0, 0, 0));
	//tr.setScale(glm::vec3(1, 0, 1));
	ground_render->setLocalTransform(tr);
	entity.assign<Render>(ground_render);

	// Physics Component
	btCollisionShape* ground_shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
	ground_shape->setLocalScaling(btVector3(100, 0, 100));
	ground_render->setTexcoordsFactor(glm::vec3(100 / STD_UNIT_PER_TILE, 100 / STD_UNIT_PER_TILE, 0));
	btTransform ground_tr;
	ground_tr.setIdentity();
	ground_tr.setOrigin(btVector3(0, 0, 0));
	btScalar mass(0.);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool is_dynamic = (mass != 0.f);
	btVector3 local_inertia(0, 0, 0);
	if (is_dynamic)
		ground_shape->calculateLocalInertia(mass, local_inertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tr);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, ground_shape, local_inertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	
	body->setFriction(2.f);
	Physics physics = { ground_shape, motion_state, body, mass, local_inertia };
	entity.assign<Physics>(physics);

	// Add an open script to the door
	addEntity("ground", entity);
}

// Creation of all the in-game entities in Game constructor's class
void Game::createDoorEntity(entityx::EntityManager &es, World& world) {
	entityx::Entity entity = es.create();

	// Render Component
	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	std::shared_ptr<Renderable<Cube>> render = std::make_shared<Renderable<Cube>>(shaders.get("simple"));
	LocalTransform tr;

	float length = 4.f;
	float height = 6.f;
	float angle = M_PI / 4.f;

	//tr.setTranslation(glm::vec3(5, 0, 0));
	tr.setScale(glm::vec3(length, height, 0.4));
	//tr.setRotation(glm::vec3(0, 0, 1), 90*2*M_PI/360.f);
	render->setLocalTransform(tr);
	entity.assign<Render>(render);

	// Physics Component
	std::vector<glm::vec3>& vertices = render->getPrimitive().getVertices();

	/*btConvexHullShape* entity_shape = new btConvexHullShape();
	for (int i = 0; i < vertices.size(); ++i) {
		entity_shape->addPoint(btVector3(vertices[i].x, vertices[i].y, vertices[i].z));
	}
	entity_shape->optimizeConvexHull();*/
	btCollisionShape* entity_shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));

	btTransform entity_tr;
	entity_tr.setIdentity();
	entity_tr.setOrigin(btVector3(cos(angle) * length / 2, height / 2, - sin(angle) * length / 2));
	entity_tr.setRotation(btQuaternion(angle, 0, 0));
	entity_shape->setLocalScaling(btVector3(length, height, 0.4));
	btScalar mass(1.);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool is_dynamic = (mass != 0.f);
	btVector3 local_inertia(0, 0, 0);
	if (is_dynamic)
		entity_shape->calculateLocalInertia(mass, local_inertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(entity_tr);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, entity_shape, local_inertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	body->setActivationState(DISABLE_DEACTIVATION);

	// Add hinge constraint
	btVector3 pivot(-length / 2.f, 0, 0);
	btVector3 axis(0, 1, 0);

	Physics physics = { entity_shape, motion_state, body, mass, local_inertia};
	//physics.constraints["pivot"] = new PhysicHingeConstraint(body, axis, pivot, torque, angle - M_PI * 0.5, angle + M_PI * 0.5);
	entity.assign<Physics>(physics);


	// Script Component
	ScriptManager& scripts = ScriptManager::getInstance();
	Script script;
	script.m_scripts[Script::INTERACTION] = "door_opening";
	entity.assign<Script>(script);

	addEntity("door", entity);

	// Add of a hinge constraint of axis Y
	events.emit<AddConstraint>(AddConstraint(entity,
		"pivotY",
		new PhysicHingeConstraint(body, axis, pivot, angle - M_PI * 0.5, angle + M_PI * 0.5)));
}

void Game::createSwordEntity(const std::string& name, entityx::EntityManager &es, World& world) {
	entityx::Entity entity = es.create();

	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	std::shared_ptr<Renderable<Model>> arrow_render = std::make_shared<Renderable<Model>>(shaders.get("simple"), "C:\\Users\\Matthieu\\Source\\Repos\\EngineCC\\EngineCC\\EngineCC\\Content\\sword.obj");
	entity.assign<Render>(arrow_render);

	/// Add physic component
	// Collision shape computed from the mesh of the entity
	entityx::ComponentHandle<Render> render = entity.component<Render>();
	assert(*render != nullptr);
	std::vector<glm::vec3>& vertices = (*render)->getPrimitive().getVertices();

	btConvexHullShape* entity_shape = new btConvexHullShape();
	for (int i = 0; i < vertices.size(); ++i) {
		entity_shape->addPoint(btVector3(vertices[i].x, vertices[i].y, vertices[i].z));
	}
	entity_shape->optimizeConvexHull();

	btTransform entity_transform;
	entity_transform.setIdentity();
	entity_transform.setOrigin(btVector3(2, 3, 2));
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	float mass = 1.f;
	bool is_dynamic = (mass != 0.f);
	btVector3 local_inertia(0, 0, 0);
	if (is_dynamic)
		entity_shape->calculateLocalInertia(mass, local_inertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(entity_transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, entity_shape, local_inertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	// Active rigid body
	body->setActivationState(DISABLE_DEACTIVATION);
	//if (data.disable_angular_rotation)
	//	body->setAngularFactor(0.f);

	Physics physics = { entity_shape, motion_state, body, mass, local_inertia };
	entity.assign<Physics>(physics);

	// A sword is carryable
	btTransform local_transform;
	local_transform.setIdentity();
	local_transform.setOrigin(btVector3(0.3f, -0.5f, -0.5f));
	local_transform.setRotation(btQuaternion(0, M_PI / 2.f, 0));
	Carryable carryable = { "typical iron sword", "all humans, orks and dwarfs can recognize it ! It's THE sword", true, local_transform };
	entity.assign<Carryable>(carryable);

	addEntity(name, entity);
}

// Creation of all the in-game entities in Game constructor's class
void Game::createArrowEntity(entityx::EntityManager &es, entityx::EventManager &events) {
	entityx::Entity entity = es.create();

	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	std::shared_ptr<Renderable<Cube>> arrow_render = std::make_shared<Renderable<Cube>>(shaders.get("simple"));
	LocalTransform tr;
	tr.setTranslation(glm::vec3(10, 5, 0));
	tr.setScale(glm::vec3(0.5, 0.5, 2));
	arrow_render->setLocalTransform(tr);
	entity.assign<Render>(arrow_render);

	// Physics Component
	const glm::vec3& scale = tr.getScaleVec();
	btCollisionShape* arrow_shape = new btBoxShape(btVector3(scale.x / 2.f, scale.y / 2.f, scale.z / 2.f));
	btTransform arrow_tr;
	arrow_tr.setIdentity();
	arrow_tr.setOrigin(btVector3(m_viewer.getPosition().x, m_viewer.getPosition().y, m_viewer.getPosition().z));
	btScalar mass(1.);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool is_dynamic = (mass != 0.f);
	btVector3 local_inertia(0, 0, 0);
	if (is_dynamic)
		arrow_shape->calculateLocalInertia(mass, local_inertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* motion_state = new btDefaultMotionState(arrow_tr);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, arrow_shape, local_inertia);
	btRigidBody* body = new btRigidBody(rbInfo);
	body->setLinearVelocity(btVector3(m_viewer.getDirection().x, m_viewer.getDirection().y, m_viewer.getDirection().z));

	Physics physics = { arrow_shape, motion_state, body, mass, local_inertia };
	entity.assign<Physics>(physics);
	
	addEntity("arrow", entity);
}

void Game::init(entityx::EntityManager& es_editor) {
	entities.reset();

	World& world = Singleton<World>::getInstance();

	m_viewer.setPosition(glm::vec3(0, 5, 0));
	m_viewer.setDirection(glm::vec3(1, 0, 0));
	
	createGroundEntity(entities);
	createDoorEntity(entities, world);
	createSwordEntity("sword", entities, world);
	createSwordEntity("sword2", entities, world);

	// The player is instanciated in last after all other entities/objects have been instanciated;
	createPlayerEntity(entities, world);

	for (entityx::Entity entity : es_editor.entities_with_components<Render, Physics>()) {
		entities.create_from_copy(entity);
	}
}

void Game::clear() {
	// For all the entities that have been defined in the game, we delete them from the dynamic world
	World& world = Singleton<World>::getInstance();
	for (std::unordered_set<std::string>::iterator it = m_game_entity_names.begin(); it != m_game_entity_names.end(); it++) {
		const std::string& name = *it;
		world.deleteEntity(name);
	}
	m_game_entity_names.clear();
}



Game::Game(GameProgram& program, InputHandler& input_handler) : ProgramState(program, input_handler),
																	  m_theta(0.f),
																	  m_alpha(0.f),
																	  m_player_direction(0.f) {
	World& world = Singleton<World>::getInstance();
	// Init systems
	/// Set up systems
	// The PhysicConstraintSystem is configured before so that he can accept the AddConstraint events 
	// for the entities that are just created below
	systems.add<PhysicConstraintSystem>();
	systems.configure();

	// Add game entities

	createGroundEntity(entities);
	createDoorEntity(entities, world);
	createSwordEntity("sword", entities, world);
	createSwordEntity("sword2", entities, world);

	// The player is instanciated in last after all other entities/objects have been instanciated;
	createPlayerEntity(entities, world);
	systems.add<PhysicSystem>(entities, *(world.dynamic_world));
	systems.add<MovementSystem>();
	systems.add<AttackSystem>();
	systems.add<ScriptSystem>(world.get("player"));
	systems.add<PickingSystem>(m_viewer, m_input_handler, world.get("player"));
	systems.add<RenderSystem>(m_viewer);
	systems.configure();

	// Init scripts
	initScripts();

	// Keyboard callbacks definition
	Viewer& viewer = m_viewer;
	entityx::EntityManager& es = entities;
	entityx::EventManager& ev = events;

	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_a, [&]() {
		createArrowEntity(entities, events);
	}));

	glm::vec3& direction_player = m_player_direction;

	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_z, [&viewer, &direction_player]() {
		direction_player += glm::vec3(viewer.getDirection().x, 0.f, viewer.getDirection().z);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_s, [&viewer, &direction_player]() {
		direction_player += glm::vec3(-viewer.getDirection().x, 0.f, -viewer.getDirection().z);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_q, [&viewer, &direction_player]() {
		direction_player += glm::vec3(viewer.getDirection().z, 0.f, -viewer.getDirection().x);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_d, [&viewer, &direction_player]() {
		direction_player += glm::vec3(-viewer.getDirection().z, 0.f, viewer.getDirection().x);
	}));

	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_SPACE, [&viewer, &direction_player]() {
		direction_player += glm::vec3(0, 10.f, 0);
	}));

	/// Interaction Key : e 
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_e, [&ev, &world, &viewer, &input_handler]() {
		entityx::Entity interactionWithEntity;
		// When the user press 'e' we check if the mouse cursor points to an entity called interactionWithEntity
		if (PickingSystem::isEntityPerInteraction(interactionWithEntity, input_handler, viewer)) {
			// We emit an InteractionEvent on this entity by the player entity.
			ev.emit<InteractionEvent>(InteractionEvent(world.get("player"), interactionWithEntity));
		}
	}));
	/// Interaction Key : r
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_r, [&ev, &world]() {
		ev.emit<AttackEvent>({ world.get("player") });
	}));
}

void Game::initScripts() {
	ScriptManager& scripts = ScriptManager::getInstance();
	const Viewer& viewer = m_viewer;
	const InputHandler& input_handler = m_input_handler;
	entityx::EventManager& ev = events;
	PhysicConstraintSystemPtr pPhysicConstraintSystem = (PhysicConstraintSystemPtr)systems.system<PhysicConstraintSystem>();

	/// Open door script
	FiniteStateMachine::State* door_opening_start = new FiniteStateMachine::State(
		[&viewer](entityx::Entity entity, entityx::Entity player) {
	}, [](entityx::Entity entity, entityx::Entity player) {
		std::cout << "Step 1 : Opening a door" << std::endl;
	});

	FiniteStateMachine::State* door_push_plus = new FiniteStateMachine::State(
		[](entityx::Entity entity, entityx::Entity player) {},
		[&ev](entityx::Entity entity, entityx::Entity player) {
		std::cout << "Step 2 : Push the door in + sense" << std::endl;

		ev.emit<StartImpulseHinge>(StartImpulseHinge(entity, "pivotY", M_PI / 4, btVector3(0, 80, 0)));
	});

	FiniteStateMachine::State* door_pull_plus = new FiniteStateMachine::State(
		[](entityx::Entity entity, entityx::Entity player) {},
		[&ev](entityx::Entity entity, entityx::Entity player) {
		std::cout << "Step 3 : Pull the door in + sense" << std::endl;

		ev.emit<StartImpulseHinge>(StartImpulseHinge(entity, "pivotY", -M_PI / 4, btVector3(0, 80, 0)));
	});

	FiniteStateMachine::State* door_push_minus = new FiniteStateMachine::State(
		[](entityx::Entity entity, entityx::Entity player) {},
		[&ev](entityx::Entity entity, entityx::Entity player) {
		std::cout << "Step 4 : Push the door in - sense" << std::endl;

		ev.emit<StartImpulseHinge>(StartImpulseHinge(entity, "pivotY", -M_PI / 4, btVector3(0, 80, 0)));
	});

	FiniteStateMachine::State* door_pull_minus = new FiniteStateMachine::State(
		[](entityx::Entity entity, entityx::Entity player) {},
		[&ev](entityx::Entity entity, entityx::Entity player) {
		std::cout << "Step 5 : Pull the door in - sense" << std::endl;

		ev.emit<StartImpulseHinge>(StartImpulseHinge(entity, "pivotY", M_PI / 4, btVector3(0, 80, 0)));
	});
	
	door_opening_start->addTransition(FiniteStateMachine::Transition(door_push_plus,
		[&input_handler, &viewer, pPhysicConstraintSystem](entityx::Entity entity, entityx::Entity player) {
		entityx::Entity entity_picked;
		if (!PickingSystem::isEntityPerInteraction(entity_picked, input_handler, viewer))
			return false;
		if (entity_picked != entity)
			return false;

		/*entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
		PhysicHingeConstraint* pivot = (PhysicHingeConstraint*)(physic->constraints["pivot2"]);
		std::cout << "1 -> 2" << std::endl;
		assert(pivot != nullptr);
		btScalar start_angle = (pivot->() + pivot->getLowerLimitAngle()) / 2.f;*/

		btScalar start_angle = pPhysicConstraintSystem->getStartAngle(entity, "pivotY");
		glm::vec2 door_v(glm::cos(start_angle), -glm::sin(start_angle));
		glm::vec2 pos_v(viewer.getPosition().x, viewer.getPosition().z);
		pos_v = glm::normalize(pos_v);

		float sin_theta = pos_v.x * door_v.y - pos_v.y * door_v.x;
		return (sin_theta < 0.f);
	}));

	door_opening_start->addTransition(FiniteStateMachine::Transition(door_push_minus,
		[&input_handler, &viewer, pPhysicConstraintSystem](entityx::Entity entity, entityx::Entity player) {
		entityx::Entity entity_picked;
		if (!PickingSystem::isEntityPerInteraction(entity_picked, input_handler, viewer))
			return false;
		if (entity_picked != entity)
			return false;

		/*entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
		PhysicHingeConstraint* pivot = (PhysicHingeConstraint*)(physic->constraints["pivot2"]);
		std::cout << "1 -> 4" << std::endl;
		assert(pivot != nullptr);
		btScalar start_angle = (pivot->getUpperLimitAngle() + pivot->getLowerLimitAngle()) / 2.f;*/
		btScalar start_angle = pPhysicConstraintSystem->getStartAngle(entity, "pivotY");
		glm::vec2 door_v(glm::cos(start_angle), -glm::sin(start_angle));
		glm::vec2 pos_v(viewer.getPosition().x, viewer.getPosition().z);
		pos_v = glm::normalize(pos_v);

		float sin_theta = pos_v.x * door_v.y - pos_v.y * door_v.x;
		return (sin_theta >= 0.f);
	}));

	const std::function<bool(entityx::Entity entity, entityx::Entity player)>& transition_door = 
	[&input_handler, &viewer, pPhysicConstraintSystem] (entityx::Entity entity, entityx::Entity player) {
		if (!input_handler.m_keydown)
			return false;
		if (input_handler.m_key.find(SDLK_e) == input_handler.m_key.end())
			return false;
		entityx::Entity entity_picked;
		if (!PickingSystem::isEntityPerInteraction(entity_picked, input_handler, viewer))
			return false;
		if (entity_picked != entity)
			return false;
		
		return pPhysicConstraintSystem->isFinished(entity, "pivotY");
	};

	door_push_plus->addTransition(FiniteStateMachine::Transition(door_pull_plus, transition_door));
	door_pull_plus->addTransition(FiniteStateMachine::Transition(door_opening_start, transition_door));
	door_push_minus->addTransition(FiniteStateMachine::Transition(door_pull_minus, transition_door));
	door_pull_minus->addTransition(FiniteStateMachine::Transition(door_opening_start, transition_door));

	/// Interaction script for picked objects
	FiniteStateMachine::State* picking = new FiniteStateMachine::State(
		[&viewer](entityx::Entity entity, entityx::Entity player) {
		entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
		
	});

	scripts.insert("door_opening", std::make_shared<FiniteStateMachine>(door_opening_start));
}

Game::~Game() {
	PhysicConstraintSystemPtr pPhysicConstrainedSystem = systems.system<PhysicConstraintSystem>();
	pPhysicConstrainedSystem->~PhysicConstraintSystem();

	World& world = Singleton<World>::getInstance();
	world.free();
}

void Game::addEntity(const std::string& name, entityx::Entity entity) {
	/*
	// Add it to the game's bullet dynamic world
	m_game_entities[name + std::to_string(id_entity)] = entity;

	entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
	m_dynamic_world->addRigidBody(physic->rigid_body);
	id_entity++;
	*/
	m_game_entity_names.insert(name);
	World& world = Singleton<World>::getInstance();
	world.addEntity(name, entity);
}

glm::vec3 btVector3ToGlmVec3(const btVector3& vec) {
	return glm::vec3(vec.x(), vec.y(), vec.z());
}

void Game::run() {
	// Get a reference to the world
	World& world = Singleton<World>::getInstance();
	entityx::Entity player = world.get("player");


	GameProgram::m_current_viewer = &m_viewer;

	/// Systems updates
	systems.update_all(1.f / 60.f);

	/*systems.update<PhysicSystem>(1.f / 60.f);
	systems.update<MovementSystem>(1.f / 60.f);
	systems.update<ScriptSystem>(1.f / 60.f);
	systems.update<PickingSystem>(1.f / 60.f);
	systems.update<RenderSystem>(1.f / 60.f);*/

	/// Player keyboard callbacks
	// Reset the direction vector of the player
	m_player_direction = glm::vec3(0.f);
	this->callbacks();
	
	/// Player view update
	// Viewer in game mode update
	// First Player Shooter
	if (m_input_handler.m_mouse_X >= 0 && m_input_handler.m_mouse_X <= GameProgram::width) {
		if (m_input_handler.m_mouse_Y >= 0 && m_input_handler.m_mouse_Y <= GameProgram::height) {
			float factorX = (float)m_input_handler.m_mouse_X / GameProgram::width - 0.5f;
			m_alpha += glm::abs(factorX) >= 0.1 ? factorX*0.1f : 0.f;
			float factorY = ((GameProgram::height - (float)m_input_handler.m_mouse_Y) / GameProgram::height - 0.5f);
			m_theta += glm::abs(factorY) >= 0.1f ? factorY*0.1f : 0.f;

			if (glm::abs(m_theta) >= 3.14f / 3.f) {
				m_theta = glm::sign(m_theta)*3.14f / 3.f;
			}

			m_viewer.setDirection(glm::vec3(glm::sin(m_alpha)*glm::cos(m_theta),
				glm::sin(m_theta),
				-glm::cos(m_alpha)*glm::cos(m_theta)));
			
			btTransform tr = player.component<Physics>()->rigid_body->getWorldTransform();
			//tr.setIdentity();
			tr.setRotation(btQuaternion(-m_alpha, 0, 0));
			player.component<Physics>()->rigid_body->setWorldTransform(tr);

		}
	}

	// Send an DisplacementEvent of the player entity to the MovementSystem.
	if (m_player_direction != glm::vec3(0.f)) {
		// Normalization of the direction of the player
		glm::normalize(m_player_direction);
		events.emit<DisplacementEvent>(player, m_player_direction);
	}
	else {
		events.emit<StopDisplacementEvent>(player);
	}

	/// Set viewer on the player position
	entityx::ComponentHandle<Physics> physic = player.component<Physics>();
	//btVector3 viewer_pos(m_viewer.getPosition().x, m_viewer.getPosition().y, m_viewer.getPosition().z);
	//btVector3 pos_player = viewer_pos + m_player_direction;
	//std::cout << physic->rigid_body->getLinearVelocity().x() << " " << physic->rigid_body->getLinearVelocity().y() << " " << physic->rigid_body->getLinearVelocity().z() << std::endl;
	
	btTransform player_transform;
	player_transform = player.component<Physics>()->rigid_body->getCenterOfMassTransform();

	m_viewer.setPosition(btVector3ToGlmVec3(player_transform.getOrigin()));
}