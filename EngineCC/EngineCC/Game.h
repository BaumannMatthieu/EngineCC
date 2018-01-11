#pragma once

#include <memory>
#include <unordered_set>
#include <entityx/entityx.h>

#include "ProgramState.h"
#include "Viewer.h"
#include "World.h"

class Game : public ProgramState, public entityx::EntityX
{
public:
	Game(GameProgram& program, InputHandler& input_handler);
	virtual ~Game();
	void run();

	/// Call this function before diving into the game
	// Precondition : the game entity manager is empty
	// Postcondition :
	// - copy all entities from the editor entity manager
	// - creates a player entity
	// - reset the viewer position
	void init(entityx::EntityManager& es_editor);

	void clear();

private:
	void createGroundEntity(entityx::EntityManager &es);
	void createDoorEntity(entityx::EntityManager &es, World& world);
	void createPlayerEntity(entityx::EntityManager &es, World& world);
	void createArrowEntity(entityx::EntityManager &es, entityx::EventManager &events);
	void createSwordEntity(const std::string& name, entityx::EntityManager &es, World& world);

	void addEntity(const std::string& name, entityx::Entity entity);

	void initScripts() const;

private:
	Viewer m_viewer;

	/// Player 
	std::unordered_set<std::string> m_game_entity_names;
	glm::vec3 m_player_direction;

	float m_theta;
	float m_alpha;
};

