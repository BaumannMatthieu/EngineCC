#pragma once

#include <functional>
#include <vector>

class FiniteStateMachine {
public:
	struct Transition;
	struct State {
		State(const std::function<void()>& action_f, const std::function<void()>& configure_f = []() {});
		void run() const;
		void configure() const;

		void addTransition(const Transition& tr);
		const std::vector<Transition>& getAllTransitions() const;

	private:
		std::function<void()> m_action_f;
		std::function<void()> m_configure_f;
		std::vector<Transition> m_transitions;
	};

	struct Transition {
		Transition(const State* next, const std::function<bool()>& func);
		~Transition();

		std::function<bool()> m_func;
		const State* m_next;
	};

	// Returns true if the FiniteStateMachine has ended
	bool run();

	FiniteStateMachine(const State* root);
	~FiniteStateMachine();
private:
	const State* m_root;
	const State* m_current;
};