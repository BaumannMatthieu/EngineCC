#include <functional>

#include "InputHandler.h"
#include "PickingSystem.h"
#include "Singleton.h"

InputHandler::InputHandler(GameProgram& program) : m_program(program) {
	m_keydown = false;
	m_button = 0;
	m_wheel = 0;

	m_key_repeat_disabled.insert(SDLK_RETURN);
	// SDLK_e is reserved for the interaction with other entities
	m_key_repeat_disabled.insert(SDLK_e);
	m_key_repeat_disabled.insert(SDLK_r);
}

InputHandler::~InputHandler() {
}

void InputHandler::update(SDL_Event& event) {
	// Pump new events 
	// Get new keyboard events
	m_wheel = 0;
	// Delete from m_key the keys which are not repeated
	if (m_keydown) {
		for(std::set<int>::iterator it = m_key_repeat_disabled.begin(); it != m_key_repeat_disabled.end(); it++)
			m_key.erase(*it);
	}

	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);
		int key_pushed = event.key.keysym.sym;
	
		switch (event.type) {
		case SDL_KEYDOWN:
			m_key_repeat = event.key.repeat;
			if (m_key_repeat_disabled.find(key_pushed) == m_key_repeat_disabled.end()) {
				m_key.insert(key_pushed);
			}
			else {
				if (!m_keydown) {
					m_key.insert(key_pushed);
				}
			}
			m_keydown = true;
			break;
		case SDL_KEYUP:
			m_key.erase(event.key.keysym.sym);
			if (m_key.empty()) {
				m_keydown = false;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_button = event.button.button;
			break;
		case SDL_MOUSEBUTTONUP:
			m_button = 0;
			break;
		case SDL_MOUSEMOTION:
			m_mouse_X = event.motion.x;
			m_mouse_Y = event.motion.y;
		case SDL_MOUSEWHEEL:
			m_wheel = event.wheel.y;
		default:
			break;
		}
	}
}




