#include "EntityEditionPanel.h"

#include "PickingSystem.h"
#include "InputHandler.h"

HierarchyEntity HierarchyEntity::hierarchy;
int EntityCreationPanel::id = 0;

void EditionWindow::addEntity(const std::string& name, const Transform& tr, entityx::Entity entity) {
	assert(entity.valid());
	/// Add into the dynamic world for bullet calculations
	World& world = Singleton<World>::getInstance();
	world.addEntity(name, entity);

	m_transforms.insert(std::pair<std::string, Transform>(name, tr));

	updateEntity(name);

	std::size_t pos = name.find("_");
	const std::string& entity_filename = name.substr(0, pos);

	if (m_ids.find(entity_filename) == m_ids.end()) {
		m_ids[entity_filename] = 2;
	}
	else {
		m_ids[entity_filename] = m_ids[entity_filename] + 1;
	}

	//HierarchyEntity& hierarchy = HierarchyEntity::getHierarchyEntity();
	//hierarchy.m_root->addEntity();
}

void EditionWindow::duplicateEntity(entityx::EntityManager& es) {
	const std::string& entity_to_duplicate_name = m_name_entity_picked;
	std::size_t pos = entity_to_duplicate_name.find("_");
	
	// reconstruct a new name
	const std::string& entity_filename = entity_to_duplicate_name.substr(0, pos);
	std::string entity_name = entity_filename;
	entity_name += "_";
	entity_name += std::to_string(m_ids[entity_filename]);

	EntityCreationPanel& creationWindow = Singleton<EntityCreationPanel>::getInstance();
	// create a new entity with the same properties as the one selected
	// loadEntity will first read the xml file of the entity and, parse it and create it through the
	// EntityCreationPanel::creationEntity method
	creationWindow.loadEntity(es, entity_name, entity_filename, m_transforms[m_name_entity_picked]);
}

void EditionWindow::deleteEntity(const std::string& name) {
	m_transforms.erase(name);

	World& world = Singleton<World>::getInstance();
	world.deleteEntity(name);

	// delete the picked entity so that the panel disappear until 
	// the user pick another entity
	m_name_entity_picked.clear();
}

void EditionWindow::clear() {
	m_transforms.clear();
	m_name_entity_picked.clear();

	World& world = Singleton<World>::getInstance();
	world.free();
}

void EditionWindow::render(const InputHandler& input, entityx::EntityManager& es) {
	// Mouse input handling
	bool hit = false;
	btVector3 intersection_point(0, 0, 0);

	if (input.m_button == SDL_BUTTON_LEFT) {
		 const std::string& name = PickingSystem::getPickedEntity(input, *GameProgram::m_current_viewer, hit, intersection_point);
		 if (hit) {
			 //std::cout << "Entity picked" << std::endl;
			 m_name_entity_picked = name;
		 }
	}

	if (!m_name_entity_picked.empty()) {
		Transform& tr = m_transforms[m_name_entity_picked];
		bool open = true;
		static bool snap_to_grid = false;
		ImVec2 window_pos = ImVec2(10.f, 10.f);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

		ImGui::Begin("Renderable edition", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Checkbox("Snap to grid", &snap_to_grid);

		ImGui::Separator();

		ImGui::Text(("Entity name selected : " + m_name_entity_picked).c_str());
		if (hit) {
			tr.tr = glm::vec3(intersection_point.x(), intersection_point.y(), intersection_point.z());
			if (snap_to_grid) {
				tr.tr = glm::round(tr.tr);
			}
		}

		if (ImGui::TreeNode("Rotation"))
		{
			ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&tr.rot));
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNode("Scale"))
		{
			ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&tr.scale));
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNode("Translation"))
		{
			ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&tr.tr));
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::Text("Texcoords factor :");
		ImGui::InputFloat3(": X, Y, Z", reinterpret_cast<float *>(&tr.tex_factor));

		//HierarchyEntity& hierarchy = HierarchyEntity::getHierarchyEntity();
		//hierarchy.drawImGUI();

		updateEntity(m_name_entity_picked);

		if (ImGui::Button("Delete")) {
			//hierarchy.m_root->deleteEntity(selected->getName());
			deleteEntity(m_name_entity_picked);
		}

		if (ImGui::Button("Duplicate")) {
			duplicateEntity(es);
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

void EditionWindow::reset() {
	for (std::map<std::string, Transform>::const_iterator it = m_transforms.cbegin(); it != m_transforms.cend(); it++) {
		const std::string& name = it->first;
		updateEntity(name);
	}
}

void EditionWindow::updateEntity(const std::string& name) {
	World& world = Singleton<World>::getInstance();
	entityx::Entity entity = world.get(name);
	entityx::ComponentHandle<Physics> physic = entity.component<Physics>();
	entityx::ComponentHandle<Render> render = entity.component<Render>();

	const Transform& tr = m_transforms[name];

	btTransform new_transform;
	new_transform.setIdentity();
	new_transform.setOrigin(btVector3(tr.tr.x, tr.tr.y, tr.tr.z));

	btScalar theta_X = tr.rot.x * (2 * M_PI) / 360;
	btScalar theta_Y = tr.rot.y * (2 * M_PI) / 360;
	btScalar theta_Z = tr.rot.z * (2 * M_PI) / 360;
	new_transform.setRotation(btQuaternion(theta_Y, theta_X, theta_Z));

	//physic.motion_state->setWorldTransform(new_transform);
	physic->rigid_body->setWorldTransform(new_transform);

	// The scaling is applied to the collision shape and to the renderable
	physic->collision_shape->setLocalScaling(btVector3(tr.scale.x, tr.scale.y, tr.scale.z));
	LocalTransform render_tr = (*render)->getLocalTransform();
	render_tr.setScale(tr.scale);
	(*render)->setLocalTransform(render_tr);
}

std::ostream& operator<<(std::ostream& stream_out, const EditionWindow::Transform& transform) {
	stream_out << "Translation : " << std::endl;
	stream_out << transform.tr.x << " " << transform.tr.y << " " << transform.tr.z << std::endl;
	stream_out << "Rotation : " << std::endl;
	stream_out << transform.rot.x << " " << transform.rot.y << " " << transform.rot.z << std::endl;
	stream_out << "Scale : " << std::endl;
	stream_out << transform.scale.x << " " << transform.scale.y << " " << transform.scale.z << std::endl;

	return stream_out;
}