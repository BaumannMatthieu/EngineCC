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
	Scene();
	~Scene();

	void init();
	void update();
	void draw(const Viewer& viewer) const;
	void addEntity(std::unique_ptr<Entity> entity);

	std::vector<std::unique_ptr<Entity>>& getEntities();
	std::unique_ptr<Entity>& getEntity(const std::string& name);

private:
	std::vector<std::unique_ptr<Entity>> m_entities;
};

