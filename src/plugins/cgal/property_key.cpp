#include "property_key.h"

namespace possumwood {

PropertyKey::PropertyKey() : m_index(-1) {
}

bool PropertyKey::isDefault() const {
	return m_index < 0;
}

std::ostream& operator<<(std::ostream& out, const PropertyKey& prop) {
	out << prop.m_index;
	return out;
}

}  // namespace possumwood
