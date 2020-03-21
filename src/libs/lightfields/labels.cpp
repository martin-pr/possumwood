#include "labels.h"

#include <cassert>
#include <map>
#include <iostream>

namespace lightfields {

Labels::Labels(std::size_t size, unsigned label_limit) : m_labels(size), m_counters(label_limit), m_limit(label_limit), m_maxLabel(0) {
	m_counters[0] = size;
}

void Labels::clear(unsigned default_val) {
	m_labels.assign(m_labels.size(), default_val);
	m_counters.assign(m_counters.size(), 0);

	if(default_val < m_counters.size())
		m_counters[default_val] = m_labels.size();
}

std::size_t Labels::size() const {
	return m_labels.size();
}

Labels::Proxy Labels::operator[](std::size_t index) {
	assert(index < m_labels.size());
	return Proxy(this, index);
}

const unsigned& Labels::operator[](std::size_t index) const {
	assert(index < m_labels.size());
	return m_labels[index];
}

std::vector<Labels::Gap> Labels::gaps() const {
	std::vector<Labels::Gap> result;

	Labels::Gap current {0, 0};

	auto it = m_counters.begin();
	auto it2 = it+1;

	while(it2 - m_counters.begin() <= m_maxLabel && it2 != m_counters.end()) {
		if(*it > 0 && *it2 == 0)
			current.min = it2 - m_counters.begin();

		if(*it == 0 && *it2 > 0) {
			current.max = it - m_counters.begin();

			if(current.min > 0)
				result.push_back(current);
		}

		++it;
		++it2;
	}

	return result;
}

void Labels::relabelGap() {
	std::vector<Labels::Gap> gs = gaps();

	if(!gs.empty()) {
		std::cout << "relabelling gaps ";
		for(auto& g : gs)
			std::cout << g.min << "/" << g.max << "  ";
		std::cout << std::endl;
	}


	if(!gs.empty()) {
		for(auto& r : gs)
			m_counters[r.max+1] = 0;

		for(auto& l : m_labels)
			for(auto& r : gs)
				if(l == r.max+1)
					l = m_labels.size() + 1;
	}
}

////////////

Labels::Proxy::Proxy(Labels* parent, std::size_t index) : m_parent(parent), m_index(index) {
}

Labels::Proxy::operator unsigned () const {
	return m_parent->m_labels[m_index];
}

Labels::Proxy& Labels::Proxy::operator = (unsigned val) {
	unsigned& current = m_parent->m_labels[m_index];
	if(current < m_parent->m_limit)
		m_parent->m_counters[current]--;

	current = val;

	if(val < m_parent->m_limit) {
		m_parent->m_maxLabel = std::max(m_parent->m_maxLabel, val);
		m_parent->m_counters[val]++;
	}

	return *this;
}

}
