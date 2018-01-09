#include <entityx\Entity.h>

#include "FiniteStateMachine.h"

/// FiniteStateMachine::State definitions
FiniteStateMachine::State::State(const std::function<void(entityx::Entity, entityx::Entity)>& action_f,
	const std::function<void(entityx::Entity, entityx::Entity)>& configure_f) : m_action_f(action_f),
												m_configure_f(configure_f) {
}

void FiniteStateMachine::State::run(entityx::Entity entity, entityx::Entity player) const {
	m_action_f(entity, player);
}

void FiniteStateMachine::State::configure(entityx::Entity entity, entityx::Entity player) const {
	m_configure_f(entity, player);
}

void FiniteStateMachine::State::addTransition(const Transition& tr) {
	m_transitions.push_back(tr);
}

const std::vector<FiniteStateMachine::Transition>& FiniteStateMachine::State::getAllTransitions() const {
	return m_transitions;
}

/// FiniteStateMachine::Transition definitions
FiniteStateMachine::Transition::Transition(const State* next, const std::function<bool(entityx::Entity, entityx::Entity)>& func) : m_next(next), m_func(func) {
}
FiniteStateMachine::Transition::~Transition() {
}

/// FiniteStateMachine definitions
FiniteStateMachine::FiniteStateMachine(const State* root) : m_root(root), m_current(m_root) {
}

FiniteStateMachine::~FiniteStateMachine() {
}

bool FiniteStateMachine::run(entityx::Entity entity, entityx::Entity player) {
	m_current->run(entity, player);

	for (auto& tr : m_current->getAllTransitions()) {
		if (tr.m_func(entity, player)) {
			m_current = tr.m_next;
			m_current->configure(entity, player);
			break;
		}
	}

	return m_current->getAllTransitions().empty();
}