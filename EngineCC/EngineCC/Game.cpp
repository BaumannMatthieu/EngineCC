#include <iostream>

#include "Game.h"
#include "Viewer.h"
#include "GameProgram.h"
#include "InputHandler.h"

Game::Game(GameProgram& program, const InputHandler& input_handler) : ProgramState(program, input_handler),
																	  m_theta(0.f),
																	  m_alpha(0.f) {
	Viewer& viewer = GameProgram::m_viewer;
}

Game::~Game() {
}

void Game::run() {
	this->callbacks();

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

			GameProgram::m_viewer.setDirection(glm::vec3(glm::sin(m_alpha)*glm::cos(m_theta),
				glm::sin(m_theta),
				-glm::cos(m_alpha)*glm::cos(m_theta)));
		}
	}
}