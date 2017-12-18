#pragma once
#include "GameProgram.h"
#include "EntityEditionPanel.h"
#include <set>

class InputHandler {
public:
	InputHandler(GameProgram& program);
	~InputHandler();

	void update(SDL_Event& event);

private:

	void defineViewerActions(Viewer& viewer);

	void defineProgramActions(GameProgram& program);

public:
	bool m_keydown;
	std::set<Uint8> m_key;

	// Mouse event
	// - Position
	int m_mouse_X;
	int m_mouse_Y;
	// - Button triggered identity
	Uint8 m_button;

private:
	GameProgram& m_program;

	EntityEditionPanel m_entity_panel;
};

