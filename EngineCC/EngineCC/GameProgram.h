#pragma once

#include <SDL.h>

#include "Scene.h"
#include "Viewer.h"

class GameProgram
{
public:
	GameProgram();
	~GameProgram();

	void setFullscreen(bool active = true);
	void setFontColor(const glm::vec4& color);
	void close();

private:
	std::unique_ptr<Scene> m_scene;
	Viewer m_viewer;
	glm::vec4 m_font_color;

	SDL_Window *m_window;
	SDL_GLContext m_context;

	bool m_run;

public:
	static uint16_t width;
	static uint16_t height;
};



