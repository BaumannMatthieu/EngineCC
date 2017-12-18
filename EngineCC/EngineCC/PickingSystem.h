#pragma once
#include <memory>

#include "Entity.h"

class InputHandler;

class PickingSystem {
public:
	static const std::unique_ptr<Entity>& getPickedEntity();
	static int getSelectedId();
	static void update(InputHandler* input);

private:
	PickingSystem();
	~PickingSystem();

	static PickingSystem m_entity;

private:
	static bool m_picked;
	static int m_picked_index;
	static int m_prev_picked;
};



