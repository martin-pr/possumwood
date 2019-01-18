#include "stack.h"

void Stack::add(std::unique_ptr<Action>&& action) {
	m_actions.push_back(std::move(action));
}

void Stack::step() {
	if(!m_actions.empty()) {
		std::unique_ptr<Action> current = std::move(*m_actions.begin());
		m_actions.erase(m_actions.begin());

		std::vector<std::unique_ptr<Action>> newActions = (*current)();
		m_actions.insert(m_actions.begin(), std::make_move_iterator(newActions.begin()), std::make_move_iterator(newActions.end()));
	}
}

bool Stack::isFinished() const {
	return m_actions.empty();
}
