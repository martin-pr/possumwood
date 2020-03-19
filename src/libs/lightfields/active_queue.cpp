#include "active_queue.h"

#include <cassert>

namespace lightfields {

ActiveQueue::ActiveQueue(std::size_t size) : m_active(size), m_excess(size) {
}

bool ActiveQueue::empty() const {
	return m_queue.empty();
}

std::size_t ActiveQueue::size() const {
	return m_queue.size();
}

void ActiveQueue::push(const Item& i) {
	assert(i.index < m_excess.size());
	assert(i.excess > 0);

	if(!m_active[i.index]) {
		m_active[i.index] = true;
		m_queue.push(i.index);
	}

	m_excess[i.index] += i.excess;

	assert(m_active[i.index]);
}

ActiveQueue::Item ActiveQueue::pop() {
	assert(!empty());

	Item result {m_queue.front(), m_excess[m_queue.front()]};

	m_active[m_queue.front()] = false;
	m_excess[m_queue.front()] = 0;

	m_queue.pop();

	return result;
}

bool ActiveQueue::isActive(std::size_t index) const {
	return m_active[index];
}

}
