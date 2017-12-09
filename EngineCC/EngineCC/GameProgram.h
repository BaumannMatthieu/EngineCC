#pragma once

#include <SDL.h>

#include "Viewer.h"

class GameProgram
{
public:
	GameProgram();
	~GameProgram();

	void setFullscreen(bool active = true);
	void setFontColor(const glm::vec4& color);

private:
	Viewer m_viewer;

	glm::vec4 m_font_color;

	SDL_Window *m_window;
	SDL_GLContext m_context;

public:
	static uint16_t win_width;
	static uint16_t win_height;
};



