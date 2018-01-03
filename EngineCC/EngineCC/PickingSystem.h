#pragma once
#include <memory>
#include <entityx/entityx.h>

#include "InputHandler.h"
/*
class PickingSystem {
public:
	static const std::unique_ptr<Entity>& getPickedEntity();
	static int getSelectedId();
	static void update(const InputHandler& input, const Viewer& viewer, bool snap_to_grid);
	static void setSelectedId(int selected_id);

private:
	PickingSystem();
	~PickingSystem();

	static PickingSystem m_entity;

private:
	static bool m_picked;
	static int m_picked_index;
	static int m_prev_picked;
};
*/
namespace PickingSystem {
	// Get the entity that has been picked by the user when hitting the
	// left button of the mouse
	// return "" if nothing has been hit
	std::string getPickedEntity(const InputHandler& input, const Viewer& viewer, bool& hit, btVector3& intersection_point);
}




