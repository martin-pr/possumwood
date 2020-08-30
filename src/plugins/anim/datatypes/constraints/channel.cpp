#include "channel.h"

#include <cassert>

namespace anim {
namespace constraints {

Channel::Channel(Constraints* parent) : m_parent(parent) {
}

void Channel::addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin) {
	assert(m_values.empty() || m_values.back().endFrame() < startFrame);

	m_values.push_back(Constraint(origin, startFrame, endFrame));
}

void Channel::clear() {
	m_values.clear();
	m_frames.clear();
}

Channel::const_iterator Channel::begin() const {
	return m_values.begin();
}

Channel::const_iterator Channel::end() const {
	return m_values.end();
}

std::size_t Channel::size() const {
	return m_values.size();
}

const Frames& Channel::frames() const {
	return m_frames;
}

bool Channel::operator==(const Channel& c) const {
	return m_values == c.m_values;
}

bool Channel::operator!=(const Channel& c) const {
	return m_values != c.m_values;
}

}  // namespace constraints
}  // namespace anim
