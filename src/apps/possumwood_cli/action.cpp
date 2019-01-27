#include "action.h"

Action::Action(const std::function<std::vector<Action>()>& fn) : m_fn(fn) {
}

std::vector<Action> Action::operator()() const {
	return m_fn();
}
