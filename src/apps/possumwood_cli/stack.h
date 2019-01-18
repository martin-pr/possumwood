#pragma once

#include "action.h"

class Stack {
	public:
		Stack() = default;

		void add(std::unique_ptr<Action>&& action);

		void step();
		bool isFinished() const;

	private:
		Stack(const Stack&) = delete;
		Stack& operator = (const Stack&) = delete;

		std::vector<std::unique_ptr<Action>> m_actions;
};
