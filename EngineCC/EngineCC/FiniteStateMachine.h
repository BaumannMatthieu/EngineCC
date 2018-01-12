#pragma once

#include <functional>
#include <vector>

class FiniteStateMachine {
public:
	struct Transition;
	struct State {
		State(const std::function<void(entityx::Entity, entityx::Entity)>& action_f, const std::function<void(entityx::Entity, entityx::Entity)>& configure_f = [](entityx::Entity entity, entityx::Entity player) {});
		void run(entityx::Entity entity, entityx::Entity player) const;
		void configure(entityx::Entity entity, entityx::Entity player) const;

		void addTransition(const Transition& tr);
		const std::vector<Transition>& getAllTransitions() const;

	private:
		std::function<void(entityx::Entity, entityx::Entity)> m_action_f;
		std::function<void(entityx::Entity, entityx::Entity)> m_configure_f;
		std::vector<Transition> m_transitions;
	};

	struct Transition {
		Transition(const State* next, const std::function<bool(entityx::Entity, entityx::Entity)>& func);
		~Transition();

		std::function<bool(entityx::Entity, entityx::Entity)> m_func;
		const State* m_next;
	};

	// Returns true if the FiniteStateMachine has ended
	bool run(entityx::Entity entity, entityx::Entity player);

	FiniteStateMachine(const State* root);
	~FiniteStateMachine();
private:
	const State* m_root;
	const State* m_current;
};

using FiniteStateMachinePtr = std::shared_ptr<FiniteStateMachine>;