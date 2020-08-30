#pragma once

#include <functional>
#include <memory>
#include <vector>

class Action {
  public:
	Action(const std::function<std::vector<Action>()>& fn);
	virtual ~Action() = default;

	std::vector<Action> operator()() const;

  private:
	std::function<std::vector<Action>()> m_fn;
};
