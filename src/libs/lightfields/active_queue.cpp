#include "active_queue.h"

#include <cassert>

#include "labels.h"

namespace lightfields {

ActiveQueue::ActiveQueue(std::size_t size) : m_active(size), m_excess(size), m_labels(size) {
}

bool ActiveQueue::empty() const {
	return m_queue.empty();
}

bool ActiveQueue::checkEmpty() const {
	if(!m_queue.empty())
		return false;

	for(std::size_t a=0;a<m_active.size();++a)
		if(m_active[a])
			return false;

	for(auto& e : m_excess)
		if(e != 0)
			return false;

	return true;
}

std::size_t ActiveQueue::size() const {
	return m_queue.size();
}

void ActiveQueue::push(const Item& i) {
	assert(i.index < m_excess.size());
	assert(i.excess > 0);

	Item to_push = i;
	to_push.excess += m_excess[i.index];

	m_active[to_push.index] = true;
	m_queue.push(to_push);

	m_excess[to_push.index] = to_push.excess;
	m_labels[to_push.index] = to_push.label;

	assert(m_active[i.index]);
}

ActiveQueue::Item ActiveQueue::pop() {
	assert(!empty());

	// discard all elements that don't match
	while(!m_queue.empty() && (m_queue.top().excess != m_excess[m_queue.top().index] || m_queue.top().label != m_labels[m_queue.top().index]))
		m_queue.pop();

	const Item result = m_queue.top();
	assert(result.excess == m_excess[result.index]);
	m_queue.pop();

	m_active[result.index] = false;
	m_excess[result.index] = 0;

	// discard all elements that don't match
	while(!m_queue.empty() && (m_queue.top().excess != m_excess[m_queue.top().index] || m_queue.top().label != m_labels[m_queue.top().index]))
		m_queue.pop();

	return result;
}

bool ActiveQueue::isActive(std::size_t index) const {
	return m_active[index];
}

bool ActiveQueue::Item::operator < (const Item& i) const {
	// if(label != i.label)
		return label > i.label;

	// if(excess != i.excess)
	// 	return excess > i.excess;

	// return index < i.index;
}

void ActiveQueue::relabel(const Labels& l) {
	assert(l.size() == m_labels.size());

	std::priority_queue<Item> new_q;
	while(!m_queue.empty()) {
		Item i = m_queue.top();
		m_queue.pop();

		if(i.excess == m_excess[i.index] && i.label == m_labels[i.index]) {
			i.label = l[i.index];

			new_q.push(i);
		}
	}
	m_queue = new_q;

	for(std::size_t a=0;a<l.size();++a)
		m_labels[a] = l[a];
}

int ActiveQueue::excess(std::size_t index) const {
	assert(index < m_excess.size());
	return m_excess[index];
}

}
