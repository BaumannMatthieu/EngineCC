#include <iostream>
#include <string>
#include <set>
#include <functional>

#include "Dependencies\glew\glew.h"

#include "GameProgram.h"
#include "Entity.h"
#include "InputHandler.h"
#include "FiniteStateMachine.h"

#include "Manager.h"

#include <entityx/entityx.h>

uint16_t GameProgram::width = 1366;
uint16_t GameProgram::height = 1024;

std::unique_ptr<Editor> GameProgram::editor = nullptr;
std::unique_ptr<Game> GameProgram::game = nullptr;
Viewer* GameProgram::m_current_viewer = nullptr;

void SetSDLFlags() {
	// Request opengl 4.5 context
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	// Turn on double buffering with a 24bit Z buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
}

void SetOpenglFlags() {
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void InitGLEW() {
	glewExperimental = GL_TRUE;
	GLenum rev = glewInit();

	if (GLEW_OK != rev) {
		std::cout << "Error: " << glewGetErrorString(rev) << std::endl;
		exit(1);
	}
	else {
		std::cout << "GLEW Init: Success!" << std::endl;
	}
}

void GameProgram::setFullscreen(bool active) {
	if (active) {
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		width = DM.w;
		height = DM.h;

		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else {
		SDL_SetWindowFullscreen(m_window, 0);
	}
}

GameProgram::GameProgram() : m_font_color(glm::vec4(0.5, 0.5, 0.8, 1.0)),
							 m_run(true) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) { /* Initialize SDL's Video subsystem */
		std::cout << "Unable to initialize SDL";
		exit(1);
	}

	IMG_Init(IMG_INIT_JPG);

	SetSDLFlags();
	// Create our window centered at win_width x win_height  resolution
	m_window = SDL_CreateWindow("EngineCC",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	// Die if creation failed
	if (!m_window) {
		std::cout << "SDL Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}

	/* Create our opengl context and attach it to our window */
	m_context = SDL_GL_CreateContext(m_window);
	InitGLEW();

	SetOpenglFlags();

	SDL_GL_SetSwapInterval(1);
	// Setup ImGui binding
	ImGui_ImplSdlGL3_Init(m_window);

	// Definition of the shaders
	initShaders();

	SDL_Event event;
	InputHandler inputHandler(*this);
	editor = std::make_unique<Editor>(*this, inputHandler);
	game = std::make_unique<Game>(*this, inputHandler);

	FiniteStateMachine::State* editorState = new FiniteStateMachine::State(
	[]() {
		GameProgram::editor->run();
	},
	[]() {
		GameProgram::game->clear();
		GameProgram::editor->reset();
	});

	FiniteStateMachine::State* gameState = new FiniteStateMachine::State(
	[]() {
		GameProgram::game->run();
	},
		// When the game is launched, we init it by copying all entities from the editor to the game
	[]() {
		GameProgram::game->init(GameProgram::editor->entities);
	});

	editorState->addTransition(FiniteStateMachine::Transition(gameState, [&inputHandler, this] {
		if (inputHandler.m_keydown) {
			if (inputHandler.m_key.find(SDLK_RETURN) != inputHandler.m_key.end()) {
				return true;
			}
		}
		return false;
	}));

	gameState->addTransition(FiniteStateMachine::Transition(editorState, [&inputHandler, this] {
		if (inputHandler.m_keydown) {
			if (inputHandler.m_key.find(SDLK_RETURN) != inputHandler.m_key.end()) {
				return true;
			}
		}
		return false;
	}));

	std::unique_ptr<FiniteStateMachine> programStateMachine = std::make_unique<FiniteStateMachine>(gameState);

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (m_run) {
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(m_font_color.x,
					 m_font_color.y,
					 m_font_color.z,
					 m_font_color.w);

		ImGui_ImplSdlGL3_NewFrame(m_window);
		inputHandler.update(event);

		programStateMachine->run();

		ImGui::Render();

		SDL_GL_SwapWindow(m_window);
	}
}

void GameProgram::initShaders() const {
	// Simple shader loading handling vertices and colors
	std::shared_ptr<Shader> textured_shader = std::make_shared<Shader>("vertex_shader.glsl", "fragment_shader.glsl");
	std::shared_ptr<Shader> textured_cubemap_shader = std::make_shared<Shader>("vertex_cubemap.glsl", "fragment_cubemap.glsl");
	std::shared_ptr<Shader> simple_shader = std::make_shared<Shader>("vertex_color_shader.glsl", "fragment_color_shader.glsl");
	std::shared_ptr<Shader> grid_shader = std::make_shared<Shader>("vertex_color_shader.glsl", "fragment_grid.glsl");
	// Add a new shader for drawing the lines of the bullet debug drawer
	std::shared_ptr<Shader> debug_bullet_shader = std::make_shared<Shader>("vertex_debug_bullet_shader.glsl", "fragment_color_shader.glsl");

	Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();

	shaders.insert("simple", simple_shader);
	shaders.insert("grid", grid_shader);
	shaders.insert("texture", textured_shader);
	shaders.insert("textured_cubemap", textured_cubemap_shader);
	shaders.insert("debug_bullet", debug_bullet_shader);
}

GameProgram::~GameProgram()
{
	ImGui_ImplSdlGL3_Shutdown();
	// Delete our opengl context, destroy our window, and shutdown SDL
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);

	IMG_Quit();
	SDL_Quit();
}

void GameProgram::setFontColor(const glm::vec4& color) {
	m_font_color = color;
}

void GameProgram::close() {
	m_run = false;
}
