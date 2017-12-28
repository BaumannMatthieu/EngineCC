#include <iostream>

#include "Editor.h"
#include "Viewer.h"
#include "GameProgram.h"
#include "InputHandler.h"

Editor::Editor(GameProgram& program, const InputHandler& input_handler) : ProgramState(program, input_handler),
																			m_snap_to_grid(true),
																			m_draw_grid(true) {
	float speed_viewer = 0.2f;
	Viewer& viewer = GameProgram::m_viewer;
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_w, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(0, 0, -speed_viewer));
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_s, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(0, 0, speed_viewer));
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_a, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(-speed_viewer, 0, 0));
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_d, [&viewer, speed_viewer]() {
		viewer.translate(glm::vec3(speed_viewer, 0, 0));
	}));

	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	m_grid = std::make_unique<Renderable<Plane>>(shaders.get("grid"));

	LocalTransform t;
	t.scale(glm::vec3(1000, 0, 1000));
	m_grid->setLocalTransform(t);
}

Editor::~Editor() {
}

void Editor::run() {
	this->callbacks();

	// Mouse input handling
	PickingSystem::update(m_input_handler, m_snap_to_grid);

	EntityEditionPanel& entity_panel = Singleton<EntityEditionPanel>::getInstance();
	entity_panel.render();
	EntityCreationPanel& creation_panel = Singleton<EntityCreationPanel>::getInstance();
	creation_panel.render();

	if(m_draw_grid)
		m_grid->draw(GameProgram::m_viewer);
}