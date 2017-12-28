#pragma once

#include <memory>
#include "ProgramState.h"

#include "Renderable.h"
#include "Cube.h"

class GameProgram;
class InputHandler;

class Editor : public ProgramState {
public:
	Editor(GameProgram& program, const InputHandler& input_handler);
	virtual ~Editor();

	void run();

private:
	std::unique_ptr<Renderable<Plane>> m_grid;

	bool m_draw_grid;
	bool m_snap_to_grid;
};
