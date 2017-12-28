#pragma once

#include "ProgramState.h"

class Game : public ProgramState
{
public:
	Game(GameProgram& program, const InputHandler& input_handler);
	virtual ~Game();

	void run();

private:
	float m_theta;
	float m_alpha;
};

