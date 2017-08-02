#include "unique_id.h"

#include <atomic>

namespace possumwood {

UniqueId::UniqueId() {
	static std::atomic<std::size_t> s_counter;

	m_id = s_counter++;
}

bool UniqueId::operator == (const UniqueId& id) const {
	return m_id == id.m_id;
}

bool UniqueId::operator != (const UniqueId& id) const {
	return m_id != id.m_id;
}

bool UniqueId::operator < (const UniqueId& id) const {
	return m_id < id.m_id;
}

}
