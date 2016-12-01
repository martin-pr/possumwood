#include "metadata.h"

#include <cassert>

Metadata::Metadata(const std::string& nodeType) : m_type(nodeType) {
}

void Metadata::setCompute(std::function<void(Datablock&)> compute) {
	m_compute = compute;
}

size_t Metadata::attributeCount() const {
	return m_attrs.size();
}

Attr& Metadata::attr(unsigned index) {
	assert(index < m_attrs.size());
	return *m_attrs[index];
}

const Attr& Metadata::attr(unsigned index) const {
	assert(index < m_attrs.size());
	return *m_attrs[index];
}
