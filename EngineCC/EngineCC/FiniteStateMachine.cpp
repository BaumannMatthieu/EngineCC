#include "FiniteStateMachine.h"

/// FiniteStateMachine::State definitions
FiniteStateMachine::State::State(const std::function<void()>& action_f,
	const std::function<void()>& configure_f) : m_action_f(action_f),
												m_configure_f(configure_f) {
}

void FiniteStateMachine::State::run() const {
	m_action_f();
}

void FiniteStateMachine::State::configure() const {
	m_configure_f();
}

void FiniteStateMachine::State::addTransition(const Transition& tr) {
	m_transitions.push_back(tr);
}

const std::vector<FiniteStateMachine::Transition>& FiniteStateMachine::State::getAllTransitions() const {
	return m_transitions;
}

/// FiniteStateMachine::Transition definitions
FiniteStateMachine::Transition::Transition(const State* next, const std::function<bool()>& func) : m_next(next), m_func(func) {
}
FiniteStateMachine::Transition::~Transition() {
}

/// FiniteStateMachine definitions
FiniteStateMachine::FiniteStateMachine(const State* root) : m_root(root), m_current(m_root) {
}

FiniteStateMachine::~FiniteStateMachine() {
}

bool FiniteStateMachine::run() {
	m_current->run();

	for (auto& tr : m_current->getAllTransitions()) {
		if (tr.m_func()) {
			m_current = tr.m_next;
			m_current->configure();
			break;
		}
	}

	return m_current->getAllTransitions().empty();
}