#pragma once
#include "GameProgram.h"
#include "EntityEditionPanel.h"
#include "Singleton.h"
#include <functional>
#include <set>

class InputHandler {
public:
	InputHandler(GameProgram& program);
	~InputHandler();

	void update(SDL_Event& event);

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
};





