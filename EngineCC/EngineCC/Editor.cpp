#include <iostream>

#include "Editor.h"

#include "GameProgram.h"
#include "InputHandler.h"

#include "RenderSystem.h"

Editor::Editor(GameProgram& program, InputHandler& input_handler) : ProgramState(program, input_handler),
																			m_snap_to_grid(true),
																			m_draw_grid(true),
																			m_viewer(glm::vec3(10, 10, 10), glm::vec3(0,0,0)) {
	float speed_viewer = 0.5f;

	Viewer& viewer = m_viewer;
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_z, [&viewer]() {
		glm::vec3 direction = glm::normalize(glm::vec3(viewer.getDirection().x, 0.f, viewer.getDirection().z));
		viewer.translate(direction);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_s, [&viewer]() {
		glm::vec3 direction = glm::normalize(glm::vec3(-viewer.getDirection().x, 0.f, -viewer.getDirection().z));
		viewer.translate(direction);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_q, [&viewer]() {
		glm::vec3 direction = glm::normalize(glm::vec3(viewer.getDirection().z, 0.f, -viewer.getDirection().x));
		viewer.translate(direction);
	}));
	m_commands.insert(std::pair<int, std::function<void()> >(SDLK_d, [&viewer]() {
		glm::vec3 direction = glm::normalize(glm::vec3(-viewer.getDirection().z, 0.f, viewer.getDirection().x));
		viewer.translate(direction);
	}));

	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
	m_grid = std::make_unique<Renderable<Plane>>(shaders.get("grid"));

	LocalTransform t;
	t.scale(glm::vec3(1000, 0, 1000));
	m_grid->setLocalTransform(t);

	systems.add<RenderSystem>(m_viewer);
	systems.configure();
}

Editor::~Editor() {
}

void Editor::run() {
	GameProgram::m_current_viewer = &m_viewer;

	this->callbacks();

	if (m_input_handler.m_wheel == 1) {	
		m_viewer.setPosition(m_viewer.getPosition() + m_viewer.getDirection());
	}
	else if (m_input_handler.m_wheel == -1) {
		m_viewer.setPosition(m_viewer.getPosition() - m_viewer.getDirection());
	}

	EditionWindow& entity_panel = Singleton<EditionWindow>::getInstance();
	entity_panel.render(m_input_handler);
	EntityCreationPanel& creation_panel = Singleton<EntityCreationPanel>::getInstance();
	creation_panel.render(entities);
	creation_panel.loadSaveSceneWindow(entities);

	if (m_draw_grid) {
		m_grid->draw(m_viewer);
	}
	
	systems.update_all(1.f / 60.f);
}

void Editor::reset() {
	EditionWindow& entity_panel = Singleton<EditionWindow>::getInstance();
	entity_panel.reset();
}