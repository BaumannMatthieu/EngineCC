#pragma once

#include <unordered_set>
#include <map>
#include <entityx/entityx.h>
#include "Components.h"


/// Launch script event
struct LaunchEvent {
	entityx::Entity entity;
};

/// ScriptSystem definifion
class ScriptSystem : public entityx::System<ScriptSystem>, public entityx::Receiver<ScriptSystem> {
public:
	ScriptSystem() {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<LaunchEvent>(*this);
	}

	void receive(const LaunchEvent &script_event) {
		// Entities which do not have any Script component will be ignored
		if(script_event.entity.has_component<Script>())
			m_scripts.insert(script_event.entity);
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		for (std::set<entityx::Entity>::iterator it = m_scripts.begin(); it != m_scripts.end(); ++it) {
			entityx::Entity entity = *it;
			// Precondition : receive method assert that all entities in m_scripts have a script component
			entityx::ComponentHandle<Script> component = entity.component<Script>();
			if (auto script = component->m_scripts[Script::INTERACTION].lock()) {
				if (script->run()) {
					it = m_scripts.erase(it);
				} 
			}
			if (m_scripts.empty())
				break;
		}
	}
private:
	std::set<entityx::Entity> m_scripts;
};