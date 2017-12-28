#include "ProgramState.h"

#include "InputHandler.h"
#include "GameProgram.h"

ProgramState::ProgramState(GameProgram& program, const InputHandler& input_handler) : m_input_handler(input_handler) {
	m_commands.insert(std::pair<int, std::function<void()>>(SDLK_ESCAPE, [&program]() {
		std::cout << "EngineCC closed" << std::endl;
		program.close();
	}));
}

ProgramState::~ProgramState() {
}


void ProgramState::callbacks() const {
	// Keyboard input handling
	if (m_input_handler.m_keydown) {
		for (std::map<int, std::function<void()>>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it) {
			// If the action has been retrieved through keystate
			// we launch its corresponding function
			int key_code = it->first;
			if (std::find(m_input_handler.m_key.begin(), m_input_handler.m_key.end(), key_code) != m_input_handler.m_key.end())
				it->second();
		}
	}
}