#include <iostream>
#include <string>

#include <SDL_image.h>

#include "Dependencies\glew\glew.h"

#include "GameProgram.h"
#include "Renderable.h"

#include "Cube.h"
#include "Model.h"

#include "Manager.h"

#include <functional>

uint16_t GameProgram::width = 1366;
uint16_t GameProgram::height = 1024;

class InputHandler {
public:
	InputHandler(GameProgram* program, Viewer& viewer, const std::unique_ptr<Scene>& scene) : m_viewer(viewer),
																							  m_scene(scene),
																							  m_picked_index(-1) {
		this->defineViewerActions(viewer);
		this->defineProgramActions(program);
	}

	~InputHandler() {
	}

	void update() {
		// Pump new events 
		SDL_PumpEvents();
		// Get new keyboard events
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);

		Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

		const std::map<int, std::function<void()>>& ressources = inputs.getRessources();
		for (std::map<int, std::function<void()>>::const_iterator it = ressources.begin(); it != ressources.end(); ++it) {
			// If the action has been retrieved through keystate
			// we launch its corresponding function
			int index = it->first;
			if(keystate[index])
				it->second();
		}

		// Get new mouse events
		int mouse_X, mouse_Y;
		// Viewport coords
		Uint32 mask_mouse = SDL_GetMouseState(&mouse_X, &mouse_Y);
		if (mask_mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			// Normalized Device coords
			glm::vec2 mouse_normalized_device(2.f * mouse_X / GameProgram::width - 1, 1.f - 2.f * mouse_Y / GameProgram::height);
			// Homogeneous Clip coords
			// The view direction points towards the negative z.
			// w = -z
			glm::vec4 mouse_ray_clip(mouse_normalized_device.x, mouse_normalized_device.y, -1, 1);
			// Eye coords
			glm::vec4 mouse_point_eye = glm::inverse(Viewer::getProjectionMatrix()) * mouse_ray_clip;
			// Set the w to 0 because we need a ray
			// mouse_point_eye.z = -1 => the vector is aligned with the forward direction of the viewer
			glm::vec4 mouse_ray_eye(mouse_point_eye.x, mouse_point_eye.y, mouse_point_eye.z, 0);

			glm::vec4 mouse_ray_model = glm::inverse(m_viewer.getViewMatrix()) * mouse_ray_eye;
			glm::vec3 t = glm::normalize(glm::vec3(mouse_ray_model.x, mouse_ray_model.y, mouse_ray_model.z));

			glm::vec3 n(0, 1, 0);
			glm::vec3 originToViewer(m_viewer.getPosition());

			float lambda = -glm::dot(n, originToViewer) / glm::dot(n, t);

			// Compute intersection point
			glm::vec3 I = m_viewer.getPosition() + lambda * t;
			if (lambda > 0)
				std::cout << I.x << " " << I.y << " " << I.z << std::endl;

			std::vector<std::unique_ptr<Entity>>& entities = m_scene->getEntities();

			if (m_picked_index == -1) {
				for (unsigned int i = 0; i < entities.size(); ++i) {
					if (entities[i]->intersect(I)) {
						m_picked_index = i;
						break;
					}
				}
			}

			if (m_picked_index != -1) {
				LocalTransform transform = entities[m_picked_index]->getLocalTransform();
				transform.setTranslation(I);
				entities[m_picked_index]->setLocalTransform(transform);
			}
		} else {
			m_picked_index = -1;
		}
	}

private:
	void defineViewerActions(Viewer& viewer) {
		Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

		float speed_viewer = 0.1f;
		inputs.insert(SDL_SCANCODE_W, [&viewer, speed_viewer]() {
			viewer.translate(glm::vec3(0, 0, -speed_viewer));
		});
		inputs.insert(SDL_SCANCODE_S, [&viewer, speed_viewer]() {
			viewer.translate(glm::vec3(0, 0, speed_viewer));
		});
		inputs.insert(SDL_SCANCODE_A, [&viewer, speed_viewer]() {
			viewer.translate(glm::vec3(-speed_viewer, 0, 0));
		});
		inputs.insert(SDL_SCANCODE_D, [&viewer, speed_viewer]() {
			viewer.translate(glm::vec3(speed_viewer, 0, 0));
		});
	}

	void defineProgramActions(GameProgram* program) {
		Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

		inputs.insert(SDL_SCANCODE_ESCAPE, [program]() {
			std::cout << "EngineCC closed" << std::endl;
			program->close();
		});
	}

private:
	const Viewer& m_viewer;

	const std::unique_ptr<Scene>& m_scene;
	unsigned int m_picked_index;
};

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

	m_scene = std::make_unique<Scene>(m_viewer);

	InputHandler inputHandler(this, m_viewer, m_scene);

	while (m_run) {
		inputHandler.update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(m_font_color.x,
					 m_font_color.y,
					 m_font_color.z,
					 m_font_color.w);

		m_scene->draw();

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

void GameProgram::close() {
	m_run = false;
}
