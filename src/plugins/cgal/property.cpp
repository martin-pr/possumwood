#include "property.h"

namespace possumwood {

PropertyBase::PropertyBase(Properties* parent, std::size_t index)
    : m_parent(parent), m_index(index) {
}

PropertyBase::~PropertyBase() {
}

Properties& PropertyBase::parent() {
	return *m_parent;
}

const Properties& PropertyBase::parent() const {
	return *m_parent;
}

std::size_t PropertyBase::index() const {
	return m_index;
}

void PropertyBase::setIndex(std::size_t i) {
	m_index = i;
}

}
