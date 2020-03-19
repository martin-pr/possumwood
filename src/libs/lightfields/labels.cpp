#include "labels.h"

#include <cassert>
#include <map>

namespace lightfields {

Labels::Labels(std::size_t size, unsigned label_limit) : m_labels(size), m_counters(label_limit), m_limit(label_limit), m_maxLabel(0) {
	m_counters[0] = size;
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

void Labels::relabelGap() {
	auto it = m_counters.begin();
	auto it2 = it+1;

	std::vector<unsigned> toRelabel;

	while(it2 - m_counters.begin() <= m_maxLabel && it2 != m_counters.end()) {
		if(*it == 0 && *it2 > 0)
			toRelabel.push_back(it2 - m_counters.begin());

		++it;
		++it2;
	}

	if(!toRelabel.empty()) {
		for(auto& r : toRelabel)
			m_counters[r] = 0;

		for(auto& l : m_labels)
			for(auto& c : toRelabel)
				if(l == c)
					l = m_limit;
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
