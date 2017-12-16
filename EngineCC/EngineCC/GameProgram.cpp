#include <iostream>
#include <string>
#include <set>

#include <SDL_image.h>

#include "Dependencies\glew\glew.h"

#include "GameProgram.h"
#include "Renderable.h"

// Include Imgui
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

#include "Cube.h"
#include "Model.h"

#include "Manager.h"

#include <functional>

uint16_t GameProgram::width = 1366;
uint16_t GameProgram::height = 1024;

class InputHandler {
public:
	InputHandler(GameProgram& program, Viewer& viewer, const std::unique_ptr<Scene>& scene) : m_program(program),
																							  m_viewer(viewer),
																							  m_scene(scene),
																							  m_picked_index(-1) {
		this->defineViewerActions(viewer);
		this->defineProgramActions(program);
		m_keydown = false;
		m_button = 0;
	}

	~InputHandler() {
	}

	void update(SDL_Event& event) {
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
		bool show_test_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			if (ImGui::Button("Test Window")) show_test_window ^= 1;
			if (ImGui::Button("Another Window")) show_another_window ^= 1;
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

		const std::map<int, std::function<void()>>& ressources = inputs.getRessources();
		if (m_keydown) {
			//std::cout << event.key.repeat << std::endl;
			for (std::map<int, std::function<void()>>::const_iterator it = ressources.begin(); it != ressources.end(); ++it) {
				// If the action has been retrieved through keystate
				// we launch its corresponding function
				int key_code = it->first;
				if (std::find(m_key.begin(), m_key.end(), key_code) != m_key.end())
					it->second();
			}
		}

		// Update for picking
		this->pickingUpdate(event);
	}

private:
	void pickingUpdate(const SDL_Event& event) {
		std::vector<std::unique_ptr<Entity>>& entities = m_scene->getEntities();
		if (m_button == SDL_BUTTON_LEFT) {
			// Viewport coords
			m_mouse_X = event.motion.x;
			m_mouse_Y = event.motion.y;

			// Normalized Device coords
			glm::vec2 mouse_normalized_device(2.f * m_mouse_X / GameProgram::width - 1, 1.f - 2.f * m_mouse_Y / GameProgram::height);
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

			if (m_picked_index == -1) {
				for (unsigned int i = 0; i < entities.size(); ++i) {
					if (entities[i]->intersect(I)) {
						m_picked_index = i;
						const std::unique_ptr<Renderable<Cube>>& selection_box = entities[m_picked_index]->getSelectionBoxObject();
						selection_box->setColor(glm::vec4(0, 1, 0, 1));
						break;
					}
				}
			}

			if (m_picked_index != -1) {
				LocalTransform transform = entities[m_picked_index]->getLocalTransform();
				transform.setTranslation(I);
				entities[m_picked_index]->setLocalTransform(transform);
			}
		}
		else {
			if (m_picked_index != -1) {
				const std::unique_ptr<Renderable<Cube>>& selection_box = entities[m_picked_index]->getSelectionBoxObject();
				selection_box->setColor(glm::vec4(0, 0, 0, 1));
			}
			m_picked_index = -1;
		}
	}

	void defineViewerActions(Viewer& viewer) {
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

	void defineProgramActions(GameProgram& program) {
		Manager< int, std::function<void()> >& inputs = Manager < int, std::function<void()> >::getInstance();

		inputs.insert(SDLK_ESCAPE, [&program]() {
			std::cout << "EngineCC closed" << std::endl;
			program.close();
		});
	}

private:
	const Viewer& m_viewer;

	const std::unique_ptr<Scene>& m_scene;
	
	unsigned int m_picked_index;


	bool m_keydown;
	std::set<Uint8> m_key;
	Uint8 m_button;
	int m_mouse_X, m_mouse_Y;
	GameProgram& m_program;
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
	// Setup ImGui binding
	ImGui_ImplSdlGL3_Init(m_window);
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	m_scene = std::make_unique<Scene>(m_viewer);

	InputHandler inputHandler(*this, m_viewer, m_scene);

	SDL_Event event;
	while (m_run) {
		inputHandler.update(event);
		
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(m_font_color.x,
					 m_font_color.y,
					 m_font_color.z,
					 m_font_color.w);

		m_scene->draw();
		ImGui::Render();


		SDL_GL_SwapWindow(m_window);
	}
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
