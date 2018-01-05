#pragma once

#include <iostream>
#include <functional>
#include <map>

class GameProgram;
class InputHandler;

class ProgramState {
public:
	ProgramState(GameProgram& program, InputHandler& input_handler);
	virtual ~ProgramState();

	virtual void run() = 0;

protected:
	void callbacks();

protected:
	InputHandler& m_input_handler;
	std::map<int, std::function<void()>> m_commands;
};
