#pragma once

#include <memory>
#include "ProgramState.h"

#include "Renderable.h"
#include "Cube.h"

#include "Viewer.h"

class GameProgram;
class InputHandler;

#include <entityx/entityx.h>

class Editor : public ProgramState, public entityx::EntityX {
public:
	Editor(GameProgram& program, InputHandler& input_handler);
	virtual ~Editor();

	void run();
	void reset();

private:
	std::unique_ptr<Renderable<Plane>> m_grid;
	Viewer m_viewer;

	bool m_draw_grid;
	bool m_snap_to_grid;
};
