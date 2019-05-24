#include "frames.h"

#include <cassert>

namespace anim { namespace constraints {

Frames::Frames() {
}

void Frames::clear() {
	m_frames.clear();
}

const Frame& Frames::operator[](std::size_t index) const {
	assert(index < m_frames.size());
	return m_frames[index];
}

Frames::const_iterator Frames::begin() const {
	return m_frames.begin();
}

Frames::const_iterator Frames::end() const {
	return m_frames.end();
}

bool Frames::empty() const {
	return m_frames.empty();
}

std::size_t Frames::size() const {
	return m_frames.size();
}

} }
