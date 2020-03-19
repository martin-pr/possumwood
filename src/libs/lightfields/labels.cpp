#include "labels.h"

#include <cassert>

namespace lightfields {

Labels::Labels(std::size_t size) : m_labels(size) {
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

Labels::const_iterator Labels::begin() const {
	return m_labels.begin();
}

Labels::const_iterator Labels::end() const {
	return m_labels.end();
}

Labels::iterator Labels::begin() {
	return m_labels.begin();
}

Labels::iterator Labels::end() {
	return m_labels.end();
}

}
