#pragma once

#include <vector>
#include <functional>
#include <memory>

class Action {
	public:
		Action() = default;
		virtual ~Action() = default;

		std::vector<std::unique_ptr<Action>> operator()() const;

	protected:
		virtual std::vector<std::unique_ptr<Action>> exec() const = 0;

	private:
		Action(const Action&) = delete;
		Action& operator = (const Action&) = delete;
};

class ActionFunctor final : public Action {
	public:
		ActionFunctor(const std::function<std::vector<std::unique_ptr<Action>>()>& fn);

	protected:
		virtual std::vector<std::unique_ptr<Action>> exec() const override;

	private:
		std::function<std::vector<std::unique_ptr<Action>>()> m_fn;
};
