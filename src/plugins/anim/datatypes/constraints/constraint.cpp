#include "constraint.h"

#include <cassert>

namespace anim {
namespace constraints {

const anim::Transform& Constraint::origin() const {
	return m_origin;
}

std::size_t Constraint::startFrame() const {
	return m_startFrame;
}

std::size_t Constraint::endFrame() const {
	return m_endFrame;
}

Constraint::Constraint(const anim::Transform& origin, std::size_t start, std::size_t end)
    : m_origin(origin), m_startFrame(start), m_endFrame(end) {
	assert(start <= end);
}

bool Constraint::operator==(const Constraint& c) const {
	return m_origin == c.m_origin && m_startFrame == c.m_startFrame && m_endFrame == c.m_endFrame;
}

bool Constraint::operator!=(const Constraint& c) const {
	return m_origin != c.m_origin || m_startFrame != c.m_startFrame || m_endFrame != c.m_endFrame;
}

}  // namespace constraints
}  // namespace anim
