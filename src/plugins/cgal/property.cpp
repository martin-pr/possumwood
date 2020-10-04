#include "property.h"

namespace possumwood {

PropertyBase::PropertyBase(const std::string& name, const std::type_index type) : m_name(name), m_type(type) {
}

PropertyBase::~PropertyBase() {
}

const std::string& PropertyBase::name() const {
	return m_name;
}

const std::type_index& PropertyBase::type() const {
	return m_type;
}

bool PropertyBase::operator==(const PropertyBase& p) const {
	return isEqual(p);
}

bool PropertyBase::operator!=(const PropertyBase& p) const {
	return !isEqual(p);
}
}  // namespace possumwood
