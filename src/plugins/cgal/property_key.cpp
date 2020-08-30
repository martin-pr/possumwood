#include "property_key.h"

namespace possumwood {

PropertyKey::PropertyKey() : m_index(-1) {
}

bool PropertyKey::isDefault() const {
	return m_index < 0;
};

}  // namespace possumwood
