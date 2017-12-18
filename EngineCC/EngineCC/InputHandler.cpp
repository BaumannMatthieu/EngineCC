#include <functional>

#include "InputHandler.h"
#include "PickingSystem.h"

InputHandler::InputHandler(GameProgram& program) : m_program(program) {
	this->defineViewerActions(GameProgram::m_viewer);
	this->defineProgramActions(program);
	m_keydown = false;
	m_button = 0;
}

InputHandler::~InputHandler() {
}

void InputHandler::update(SDL_Event& event) {
	// Pump new events 
	// Get new keyboard events
	//const Uint8 *keystate = SDL_GetKeyboardState(NULL);
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

	ImGui_ImplSdlGL3_NewFrame(m_program.m_window);

	// Keyboard input handling
	Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

	const std::map<int, std::function<void()>>& ressources = inputs.getRessources();
	if (m_keydown) {
		for (std::map<int, std::function<void()>>::const_iterator it = ressources.begin(); it != ressources.end(); ++it) {
			// If the action has been retrieved through keystate
			// we launch its corresponding function
			int key_code = it->first;
			if (std::find(m_key.begin(), m_key.end(), key_code) != m_key.end())
				it->second();
		}
	}

	// Mouse input handling
	{
		m_mouse_X = event.motion.x;
		m_mouse_Y = event.motion.y;

		PickingSystem::update(this);
	}

	// Launch editing windows
	{
		m_entity_panel.render();
	}
}


void InputHandler::defineViewerActions(Viewer& viewer) {
	Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

	float speed_viewer = 0.1f;
	inputs.insert(SDLK_w, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(0, 0, -speed_viewer));
	});
	inputs.insert(SDLK_s, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(0, 0, speed_viewer));
	});
	inputs.insert(SDLK_a, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(-speed_viewer, 0, 0));
	});
	inputs.insert(SDLK_d, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(speed_viewer, 0, 0));
	});
}

void InputHandler::defineProgramActions(GameProgram& program) {
	Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

	inputs.insert(SDLK_ESCAPE, [&program]() {
		std::cout << "EngineCC closed" << std::endl;
		program.close();
	});
}
