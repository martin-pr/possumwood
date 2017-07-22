#include "attributes.h"

namespace anim {

Attributes::Attributes() {
}

Attributes::Attributes(const Attributes& a) : m_attributes(a.m_attributes) {
}

Attributes::Attributes(Attributes&& a) : m_attributes(std::move(a.m_attributes)) {
}

Attributes& Attributes::operator = (const Attributes& a) {
	m_attributes = a.m_attributes;

	return *this;
}

Attributes& Attributes::operator = (Attributes&& a) {
	m_attributes = std::move(a.m_attributes);

	return *this;
}

const Attribute& Attributes::operator[](const std::string& key) const {
	auto it = m_attributes.find(key);
	assert(it != m_attributes.end());

	return it->second;
}

Attribute& Attributes::operator[](const std::string& key) {
	return m_attributes[key];
}

Attributes::const_iterator Attributes::begin() const {
	return m_attributes.begin();
}

Attributes::const_iterator Attributes::end() const {
	return m_attributes.end();
}

Attributes::iterator Attributes::begin() {
	return m_attributes.begin();
}

Attributes::iterator Attributes::end() {
	return m_attributes.end();
}

std::ostream& operator << (std::ostream& out, const Attributes& attrs) {
	for(auto& p : attrs)
		out << p.first << " - " << p.second.toString() << std::endl;

	return out;
}

}
