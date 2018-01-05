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
	int m_key_repeat;
	std::set<int> m_key;
	std::set<int> m_key_repeat_disabled;

	// Mouse event
	// - Position
	int m_mouse_X;
	int m_mouse_Y;
	// - Button triggered identity
	Uint8 m_button;
	int m_wheel;

private:
	GameProgram& m_program;
};





