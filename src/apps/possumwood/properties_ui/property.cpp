#include "property.h"

namespace properties {

property_base::~property_base() {

}

unsigned property_base::flags() const {
	return m_flags;
}

void property_base::setFlags(unsigned flags) {
	m_flags = flags;
	onFlagsChanged(flags);
}

void property_base::onFlagsChanged(unsigned flags) {
	// do nothing in the base class
}

}
