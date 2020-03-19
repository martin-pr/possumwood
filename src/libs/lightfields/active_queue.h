#pragma once

#include <queue>

#include "bitfield.h"

namespace lightfields {

/// a "unique" priority queue with excess per active item
class ActiveQueue {
	public:
		ActiveQueue(std::size_t size);

		struct Item {
			std::size_t index;
			int excess;
		};

		bool empty() const;
		std::size_t size() const;

		bool checkEmpty() const;

		void push(const Item& i);
		Item pop();
		bool isActive(std::size_t index) const;

	private:
		std::queue<std::size_t> m_queue;
		Bitfield m_active;

		std::vector<int> m_excess;
};

}
