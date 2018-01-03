#include "Scene.h"

#include "EntityEditionPanel.h"
#include "Singleton.h"

Scene::Scene() {
}

Scene::~Scene() {
}

void Scene::init() {


	/*std::unique_ptr<Renderable<Cube>> cube_render = std::make_unique<Renderable<Cube>>(shaders.get("simple"));
	LocalTransform local_tr;
	local_tr.setRotation(glm::vec3(1, 0, 1), -3.14f / 2.f);
	cube_render->setLocalTransform(local_tr);

	std::unique_ptr<Renderable<Model>> jeep_render = std::make_unique<Renderable<Model>>(shaders.get("texture"), "Content/jeep.obj");
	LocalTransform local_tr3;
	local_tr3.setScale(glm::vec3(0.01f));
	jeep_render->setLocalTransform(local_tr3);

	std::unique_ptr<Renderable<Model>> monkey_render = std::make_unique<Renderable<Model>>(shaders.get("texture"), "Content/spider.obj");
	LocalTransform local_tr4;
	local_tr4.setScale(glm::vec3(0.1f));
	monkey_render->setLocalTransform(local_tr4);

	std::unique_ptr<Renderable<Model>> bob_render = std::make_unique<Renderable<Model>>(shaders.get("texture"), "Content/boblampclean.md5mesh");
	LocalTransform local_tr2;
	local_tr2.setScale(glm::vec3(0.1f));
	//local_tr2.setRotation(glm::vec3(1, 0, 0), -3.14f / 2.f);
	local_tr2.setTranslation(glm::vec3(0, 0, 10));
	bob_render->setLocalTransform(local_tr2);

	std::unique_ptr<Entity> cube = std::make_unique<Entity>("Cube1", std::move(cube_render));
	std::unique_ptr<Entity> bob = std::make_unique<Entity>("Bob", std::move(bob_render));
	std::unique_ptr<Entity> jeep = std::make_unique<Entity>("Jeep", std::move(jeep_render));
	std::unique_ptr<Entity> suzanne = std::make_unique<Entity>("Suzanne", std::move(monkey_render));

	this->addEntity(std::move(cube));
	this->addEntity(std::move(bob));
	this->addEntity(std::move(jeep));
	this->addEntity(std::move(suzanne));*/
}

void Scene::update() {
	/*for (unsigned int i = 0; i < m_entities.size(); ++i) {
	m_entities[i]->update();
	}*/
}

void Scene::draw(const Viewer& viewer) const {
	for (unsigned int i = 0; i < m_entities.size(); ++i) {
		const std::unique_ptr<RenderObject>& renderable = m_entities[i]->getRenderObject();
		const std::vector<std::unique_ptr<Renderable<Cube>>>& boxes = m_entities[i]->getBoxesObject();
		const std::unique_ptr<Renderable<Cube>>& selection_box = m_entities[i]->getSelectionBoxObject();

		renderable->draw(viewer);
		selection_box->draw(viewer);
		for (unsigned int i = 0; i < boxes.size(); ++i)
			boxes[i]->draw(viewer);
	}
}

void Scene::addEntity(std::unique_ptr<Entity> entity) {
	m_entities.push_back(std::move(entity));
}

std::vector<std::unique_ptr<Entity>>& Scene::getEntities() {
	return m_entities;
}

std::unique_ptr<Entity>& Scene::getEntity(const std::string& name) {
	for (int i = 0; i < m_entities.size(); ++i) {
		if (m_entities[i]->getName() == name) {
			return m_entities[i];
		}
	}
}
