#include "constraints.h"

#include "animation.h"

namespace anim {

const anim::Transform& Constraints::Constraint::origin() const {
	return m_origin;
}

std::size_t Constraints::Constraint::startFrame() const {
	return m_startFrame;
}

std::size_t Constraints::Constraint::endFrame() const {
	return m_endFrame;
}

Constraints::Constraint::Constraint(const anim::Transform& origin, std::size_t start, std::size_t end) : m_origin(origin), m_startFrame(start), m_endFrame(end) {
	assert(start <= end);
}

bool Constraints::Constraint::operator == (const Constraint& c) const {
	return m_origin == c.m_origin && m_startFrame == c.m_startFrame && m_endFrame == c.m_endFrame;
}

bool Constraints::Constraint::operator != (const Constraint& c) const {
	return m_origin != c.m_origin || m_startFrame != c.m_startFrame || m_endFrame != c.m_endFrame;
}

void Constraints::Channel::addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin) {
	assert(m_values.empty() || m_values.back().endFrame() < startFrame);

	m_values.push_back(Constraint(origin, startFrame, endFrame));
}

Constraints::Channel::const_iterator Constraints::Channel::begin() const {
	return m_values.begin();
}

Constraints::Channel::const_iterator Constraints::Channel::end() const {
	return m_values.end();
}

std::size_t Constraints::Channel::size() const {
	return m_values.size();
}

bool Constraints::Channel::operator == (const Channel& c) const {
	return m_values == c.m_values;
}

bool Constraints::Channel::operator != (const Channel& c) const {
	return m_values != c.m_values;
}

Constraints::Constraints(const anim::Animation& a) : m_anim(new anim::Animation(a)) {
}

Constraints::const_iterator Constraints::begin() const {
	return m_channels.begin();
}

Constraints::const_iterator Constraints::end() const {
	return m_channels.end();
}

bool Constraints::operator == (const Constraints& c) const {
	return m_channels == c.m_channels;
}

bool Constraints::operator != (const Constraints& c) const {
	return m_channels != c.m_channels;
}

bool Constraints::empty() const {
	return m_channels.empty();
}

std::size_t Constraints::size() const {
	return m_channels.size();
}

namespace {

Imath::V3f velocity(const std::vector<anim::Transform>& source, std::size_t frame, float fps) {
	assert(frame < source.size());

	if(source.size() <= 1)
		return Imath::V3f(0, 0, 0);

	else if(frame == 0)
		return (source[1].translation - source[0].translation) * fps;

	else if(frame == source.size()-1)
		return (source[source.size()-1].translation - source[source.size()-2].translation) * fps;

	// frames in the middle - average velocity from previous and next frame differential
	return (source[frame].translation - source[frame-1].translation +
		source[frame+1].translation - source[frame].translation) / 2.0f * fps;
}

anim::Transform average(const std::vector<anim::Transform>& tr, std::size_t start, std::size_t end) {
	assert(start <= end);
	assert(end < tr.size());

	Imath::V3f translation(0,0,0);
	Imath::Quatf rotation(0,0,0,0);
	for(std::size_t i=start; i<=end; ++i) {
		translation += tr[i].translation;

		if((rotation ^ tr[i].rotation) > 0)
			rotation += tr[i].rotation;
		else
			rotation += -tr[i].rotation;
	}

	return anim::Transform(rotation.normalized(), translation / (float)(end-start+1));
}

}

void Constraints::addVelocityConstraint(const std::string& jointName, float velocityThreshold) {
	if(m_anim == nullptr || m_anim->empty())
		throw std::runtime_error("Cannot detect constraints - animation data is empty.");

	// find the joint ID
	std::size_t jointId = std::numeric_limits<std::size_t>::max();
	for(auto& j : m_anim->frame(0))
		if(j.name() == jointName)
			jointId = j.index();
	if(jointId == std::numeric_limits<std::size_t>::max())
		throw std::runtime_error("Joint '" + jointName + "' not found in the animation data!");

	Channel& channel = m_channels[jointName];

	// extract the transforms in world space
	std::vector<anim::Transform> worldSpaceTr;
	for(auto& fr : *m_anim)
		worldSpaceTr.push_back(fr[jointId].world());

	// simple velocity thresholding, with averaging of the world space transform to derive the position of the constraint
	std::size_t begin = std::numeric_limits<std::size_t>::max();
	std::size_t end = 0;

	for(std::size_t frameIndex=0; frameIndex < worldSpaceTr.size(); ++frameIndex) {
		if(velocity(worldSpaceTr, frameIndex, m_anim->fps()).length() < velocityThreshold) {
			if(begin > end)
				begin = frameIndex;
			end = frameIndex;
		}

		else {
			if(begin <= end) {
				channel.addConstraint(begin, end, average(worldSpaceTr, begin, end));
				begin = std::numeric_limits<std::size_t>::max();
			}
		}
	}

	if(begin <= end)
		channel.addConstraint(begin, end, average(worldSpaceTr, begin, end));
}

std::ostream& operator <<(std::ostream& out, const Constraints& c) {
	out << "Constraints:" << std::endl;

	if(c.empty())
		out << "  (empty)" << std::endl;

	for(auto& i : c)
		out << "  " << i.first << " - " << i.second.size() << " records" << std::endl;

	return out;
}

}
