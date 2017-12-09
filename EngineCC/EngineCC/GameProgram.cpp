#include <iostream>
#include <string>

#include <SDL_image.h>

#include "Dependencies\glew\glew.h"

#include "GameProgram.h"
#include "Renderable.h"

uint16_t GameProgram::win_width = 1366;
uint16_t GameProgram::win_height = 1024;

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
		win_width = DM.w;
		win_height = DM.h;

		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else {
		SDL_SetWindowFullscreen(m_window, 0);
	}
}

GameProgram::GameProgram() : m_font_color(glm::vec4(0.5, 0.5, 0.8, 1.0)) {

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
		win_width,
		win_height,
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

	SDL_Event event;
	bool quit = false;

	Renderable<Cube> cube;

	while (!quit) {
		SDL_PumpEvents();
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);

		if (keystate[SDL_SCANCODE_ESCAPE]) {
			std::cout << "Return Key Pressed." << std::endl;
			quit = true;
		}

		//cameraEvent(keystate);

		//renderScene();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(m_font_color.x,
					 m_font_color.y,
					 m_font_color.z,
					 m_font_color.w);

		cube.draw(m_viewer);
		SDL_GL_SwapWindow(m_window);
	}
}

GameProgram::~GameProgram()
{
	// Delete our opengl context, destroy our window, and shutdown SDL
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);

	IMG_Quit();
	SDL_Quit();
}

void GameProgram::setFontColor(const glm::vec4& color) {
	m_font_color = color;
}
