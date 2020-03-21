#pragma once

#include <queue>

#include "bitfield.h"

namespace lightfields {

class Labels;

/// a "unique" priority queue with excess per active item
class ActiveQueue {
	public:
		ActiveQueue(std::size_t size);

		struct Item {
			Item(std::size_t i, int e, unsigned l) : index(i), excess(e), label(l) {}

			std::size_t index;
			int excess;
			unsigned label;

			bool operator < (const Item& i) const;
		};

		bool empty() const;
		std::size_t size() const;

		bool checkEmpty() const;

		void push(const Item& i);
		Item pop();
		bool isActive(std::size_t index) const;

		void relabel(const Labels& l);
		int excess(std::size_t index) const;

	private:
		std::priority_queue<Item> m_queue;
		Bitfield m_active;

		std::vector<int> m_excess;
		std::vector<unsigned> m_labels;
};

}
