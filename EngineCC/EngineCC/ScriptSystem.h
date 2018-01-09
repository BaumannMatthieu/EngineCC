#pragma once

#include <unordered_set>
#include <map>
#include <entityx/entityx.h>
#include "Components.h"


/// Launch script event
struct LaunchEvent {
	entityx::Entity entity;
	Script::Activation type;
};

/// ScriptSystem definifion
class ScriptSystem : public entityx::System<ScriptSystem>, public entityx::Receiver<ScriptSystem> {
public:
	ScriptSystem(entityx::Entity player) : player(player) {
	}

	void configure(entityx::EventManager &event_manager) {
		event_manager.subscribe<LaunchEvent>(*this);
	}

	void receive(const LaunchEvent &script_event) {
		// Entities which do not have any Script component will be ignored
		if (script_event.entity.has_component<Script>()) {
			// among them we determine if the entity has already a script that is currently executed
			if (m_scripts.find(script_event.entity) == m_scripts.end()) {
				// if not, the script is added in the map and will be launched at the next update of the ScriptSystem
				m_scripts[script_event.entity] = script_event.type;
			}
		}
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
		for (std::map<entityx::Entity, Script::Activation>::iterator it = m_scripts.begin(); it != m_scripts.end(); ++it) {
			entityx::Entity entity = it->first;
			Script::Activation type = it->second;
			// Precondition : receive method assert that all entities in m_scripts have a script component
			entityx::ComponentHandle<Script> component = entity.component<Script>();
			if (auto script = component->m_scripts[type].lock()) {
				if (script->run(entity, player)) {
					it = m_scripts.erase(it);
				}
			}
			if (m_scripts.empty())
				break;
		}
	}

	bool isRunningScriptFrom(entityx::Entity entity) {
		return (m_scripts.find(entity) != m_scripts.end());
	}
private:
	// All entities whose script are currently executing
	std::map<entityx::Entity, Script::Activation> m_scripts;
	entityx::Entity player;
};