#pragma once

#include <memory>
#include <unordered_set>
#include <entityx/entityx.h>

#include "ProgramState.h"
#include "Viewer.h"

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
	void createPlayerEntity(entityx::EntityManager &es);
	void createArrowEntity(entityx::EntityManager &es, entityx::EventManager &events);

	void addEntity(const std::string& name, entityx::Entity entity);

private:
	Viewer m_viewer;

	/// Player 
	std::unordered_set<std::string> m_game_entity_names;
	glm::vec3 m_player_direction;

	float m_theta;
	float m_alpha;
};

