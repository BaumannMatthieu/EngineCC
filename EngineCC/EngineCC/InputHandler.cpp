#include <functional>

#include "InputHandler.h"
#include "PickingSystem.h"
#include "Singleton.h"

InputHandler::InputHandler(GameProgram& program) : m_program(program) {
	m_keydown = false;
	m_button = 0;
}

InputHandler::~InputHandler() {
}

void InputHandler::update(SDL_Event& event) {
	// Pump new events 
	// Get new keyboard events
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);
		switch (event.type) {
		case SDL_KEYDOWN:
			m_keydown = true;
			m_key.insert(event.key.keysym.sym);
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
		default:
			break;
		}
	}
	
	m_mouse_X = event.motion.x;
	m_mouse_Y = event.motion.y;
}




