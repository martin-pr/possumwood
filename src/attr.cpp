#include "attr.h"

Attr::Attr(const std::string& name, unsigned offset, Category cat) : m_name(name), m_offset(offset), m_category(cat) {
}

Attr::~Attr() {
}

const std::string& Attr::name() const {
	return m_name;
}

const Attr::Category& Attr::category() const {
	return m_category;
}

const unsigned& Attr::offset() const {
	return m_offset;
}

bool Attr::isValid() const {
	return m_offset != unsigned(-1);
}
