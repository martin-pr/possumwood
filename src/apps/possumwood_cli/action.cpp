#include "action.h"

std::vector<std::unique_ptr<Action>> Action::operator()() const {
	return exec();
}

ActionFunctor::ActionFunctor(const std::function<std::vector<std::unique_ptr<Action>>()>& fn) : m_fn(fn) {
}

std::vector<std::unique_ptr<Action>> ActionFunctor::exec() const {
	return m_fn();
}
