#include "labels.h"

#include <cassert>
#include <map>

namespace lightfields {

Labels::Labels(std::size_t size, unsigned max_label) : m_labels(size), m_maxLabel(max_label) {
}

std::size_t Labels::size() const {
	return m_labels.size();
}

unsigned& Labels::operator[](std::size_t index) {
	assert(index < m_labels.size());
	return m_labels[index];
}

const unsigned& Labels::operator[](std::size_t index) const {
	assert(index < m_labels.size());
	return m_labels[index];
}

void Labels::relabelGap() {
	std::map<unsigned, std::size_t> label_counts;
	for(auto& l : m_labels)
		if(l < m_maxLabel)
			label_counts[l]++;

	int gap_count = 0;
	if(label_counts.size() > 2) {
		auto it = label_counts.begin();
		auto it2 = it;
		++it2;

		while(it2 != label_counts.end()) {
			if(it2->first != it->first+1) {
				// relabel the gap
				for(auto& e : m_labels)
					if(e == it2->first)
						e = m_maxLabel;

				gap_count++;

			}
			++it;
			++it2;
		}
	}
}

}
