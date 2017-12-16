#pragma once

#include "Entity.h"
#include "Cube.h"
#include "Model.h"

#include "Viewer.h"
#include "Shader.h"

#include "Manager.h"

#include<map>

class Scene
{
public:
	Scene(const Viewer& viewer) : m_viewer(viewer),
								  m_enable_bb(true) {
		// Simple shader loading handling vertices and colors
		std::shared_ptr<Shader> textured_shader = std::make_shared<Shader>("vertex_shader.glsl", "fragment_shader.glsl");
		std::shared_ptr<Shader> simple_shader = std::make_shared<Shader>("vertex_color_shader.glsl", "fragment_color_shader.glsl");

		Manager<std::string, std::shared_ptr<Shader>>& shaders = Manager<std::string, std::shared_ptr<Shader>>::getInstance();
		shaders.insert("simple", simple_shader);
		shaders.insert("texture", textured_shader);

		std::unique_ptr<Renderable<Cube>> cube_render = std::make_unique<Renderable<Cube>>(shaders.get("simple"));
		LocalTransform local_tr;
		local_tr.setRotation(glm::vec3(1, 0, 1), -3.14f / 2.f);
		cube_render->setLocalTransform(local_tr);

		std::unique_ptr<Renderable<Model>> bob_render = std::make_unique<Renderable<Model>>(shaders.get("texture"), "boblampclean.md5mesh");
		LocalTransform local_tr2;
		local_tr2.setScale(glm::vec3(0.1f));
		//local_tr2.setRotation(glm::vec3(1, 0, 0), -3.14f / 2.f);
		//local_tr2.setTranslation(glm::vec3(0, -7, -12));

		bob_render->setLocalTransform(local_tr2);

		std::unique_ptr<Entity> cube = std::make_unique<Entity>(std::move(cube_render));
		std::unique_ptr<Entity> bob = std::make_unique<Entity>(std::move(bob_render));

		this->addEntity(std::move(cube));
		this->addEntity(std::move(bob));
	}
	~Scene() {
	}

	void draw() const {
		for (unsigned int i = 0; i < m_entities.size(); ++i) {
			const std::unique_ptr<RenderObject>& renderable = m_entities[i]->getRenderObject();
			const std::vector<std::unique_ptr<Renderable<Cube>>>& boxes = m_entities[i]->getBoxesObject();
			const std::unique_ptr<Renderable<Cube>>& selection_box = m_entities[i]->getSelectionBoxObject();

			renderable->draw(m_viewer);
			selection_box->draw(m_viewer);
			for(unsigned int i = 0; i < boxes.size(); ++i) 
				boxes[i]->draw(m_viewer);
		}
	}

	void addEntity(std::unique_ptr<Entity> entity) {
		m_entities.push_back(std::move(entity));
	}

	std::vector<std::unique_ptr<Entity>>& getEntities() {
		return m_entities;
	}
	
	void getPickedRenderable(const glm::vec3& I) {
		/*for (std::vector<std::unique_ptr<RenderObject>>::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it) {
			return;
		}*/
	}

private:
	std::vector<std::unique_ptr<Entity>> m_entities;
	bool m_enable_bb;

	const Viewer& m_viewer;
};

