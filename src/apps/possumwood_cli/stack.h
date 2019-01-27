#pragma once

#include "action.h"

class Stack {
	public:
		Stack() = default;

		void add(const Action& action);

		void step();
		bool isFinished() const;

	private:
		Stack(const Stack&) = delete;
		Stack& operator = (const Stack&) = delete;

		std::vector<Action> m_actions;
};
