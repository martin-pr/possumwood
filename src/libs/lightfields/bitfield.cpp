#include "bitfield.h"

#include <atomic>
#include <cassert>

namespace lightfields {

/// An implementation of a simple parallel-safe bitfield
Bitfield::Accessor::operator bool() const {
	return m_parent->m_data[m_index] & (1 << m_offset);
}

Bitfield::Accessor& Bitfield::Accessor::operator = (bool val) {
	uint32_t new_value;
	uint32_t old_value = m_parent->m_data[m_index];
	do {
		if(val)
			new_value = old_value | (1 << m_offset);
		else
			new_value = old_value & (~(1 << m_offset));

	} while(!m_parent->m_data[m_index].compare_exchange_weak(old_value, new_value));

	assert((*static_cast<const Bitfield*>(m_parent))[m_index * 32 + m_offset] == val);

	return *this;
}

Bitfield::Accessor::Accessor(Bitfield* parent, std::size_t index, std::size_t offset) : m_parent(parent), m_index(index), m_offset(offset) {
}

//////

Bitfield::Bitfield(std::size_t size) : m_size(size), m_data(m_size / 32 + 1) {
}

std::size_t Bitfield::size() const {
	return m_size;
}

bool Bitfield::empty() const {
	return m_size == 0;
}

Bitfield::Accessor Bitfield::operator[](std::size_t index) {
	return Accessor(this, index / 32, index % 32);
}

bool Bitfield::operator[](std::size_t index) const {
	assert(index < m_size);
	return m_data[index / 32] & (1 << (index % 32));
}

}
