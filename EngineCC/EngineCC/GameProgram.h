#pragma once

#include <SDL.h>

#include "Scene.h"
#include "Viewer.h"
#include "Editor.h"
#include "Game.h"

class GameProgram
{
public:
	GameProgram();
	~GameProgram();

	void setFullscreen(bool active = true);
	void setFontColor(const glm::vec4& color);
	void close();

private:

	glm::vec4 m_font_color;
	SDL_GLContext m_context;
	bool m_run;

public:
	static std::unique_ptr<Scene> m_scene;
	static Viewer m_viewer;
	static std::unique_ptr<Editor> editor;
	static std::unique_ptr<Game> game;

	SDL_Window *m_window;

	static uint16_t width;
	static uint16_t height;
};



