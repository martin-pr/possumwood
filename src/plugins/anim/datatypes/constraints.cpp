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

/////

Constraints::Channel::Channel(Constraints* parent) : m_parent(parent) {
}

void Constraints::Channel::addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin) {
	assert(m_values.empty() || m_values.back().endFrame() < startFrame);

	m_values.push_back(Constraint(origin, startFrame, endFrame));
}

void Constraints::Channel::clear() {
	m_values.clear();
	m_frames.clear();
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

const Constraints::Frames& Constraints::Channel::frames() const {
	return m_frames;
}

bool Constraints::Channel::operator == (const Channel& c) const {
	return m_values == c.m_values;
}

bool Constraints::Channel::operator != (const Channel& c) const {
	return m_values != c.m_values;
}

////

Constraints::Frames::Frames() {
}

void Constraints::Frames::clear() {
	m_frames.clear();
}

const constraints::Frame& Constraints::Frames::operator[](std::size_t index) const {
	assert(index < m_frames.size());
	return m_frames[index];
}

Constraints::Frames::const_iterator Constraints::Frames::begin() const {
	return m_frames.begin();
}

Constraints::Frames::const_iterator Constraints::Frames::end() const {
	return m_frames.end();
}

bool Constraints::Frames::empty() const {
	return m_frames.empty();
}

std::size_t Constraints::Frames::size() const {
	return m_frames.size();
}

////

Constraints::Constraints(const anim::Animation& a) : m_anim(new anim::Animation(a)) {
}

Constraints::Constraints(const Constraints& c) {
	for(auto& val : c) {
		std::map<std::string, Channel>::iterator it = m_channels.insert(std::make_pair(val.first, Channel(this))).first;
		it->second.m_values = val.second.m_values;

		it->second.m_frames = val.second.m_frames;
	}

	m_anim = c.m_anim;
}

const Constraints& Constraints::operator = (const Constraints& c) {
	m_channels.clear();

	for(auto& val : c) {
		std::map<std::string, Channel>::iterator it = m_channels.insert(std::make_pair(val.first, Channel(this))).first;
		it->second.m_values = val.second.m_values;

		it->second.m_frames = val.second.m_frames;
	}

	m_anim = c.m_anim;

	return *this;
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

Imath::V3f velocity(const std::vector<constraints::Frame>& source, std::size_t frame, float fps) {
	assert(frame < source.size());

	if(source.size() <= 1)
		return Imath::V3f(0, 0, 0);

	else if(frame == 0)
		return (source[1].tr().translation - source[0].tr().translation) * fps;

	else if(frame == source.size()-1)
		return (source[source.size()-1].tr().translation - source[source.size()-2].tr().translation) * fps;

	// frames in the middle - average velocity from previous and next frame differential
	return (source[frame].tr().translation - source[frame-1].tr().translation +
		source[frame+1].tr().translation - source[frame].tr().translation) / 2.0f * fps;
}

anim::Transform average(const std::vector<constraints::Frame>& tr, std::size_t start, std::size_t end) {
	assert(start <= end);
	assert(end < tr.size());

	Imath::V3f translation(0,0,0);
	Imath::Quatf rotation(0,0,0,0);
	for(std::size_t i=start; i<=end; ++i) {
		translation += tr[i].tr().translation;

		if((rotation ^ tr[i].tr().rotation) > 0)
			rotation += tr[i].tr().rotation;
		else
			rotation += -tr[i].tr().rotation;
	}

	return anim::Transform(rotation.normalized(), translation / (float)(end-start+1));
}

std::size_t findJointId(const std::string& name, const anim::Animation& anim) {
	assert(!anim.empty());

	std::size_t jointId = std::numeric_limits<std::size_t>::max();
	for(auto& j : anim.frame(0))
		if(j.name() == name)
			jointId = j.index();
	if(jointId == std::numeric_limits<std::size_t>::max())
		throw std::runtime_error("Joint '" + name + "' not found in the animation data!");

	return jointId;
}

}

void Constraints::addVelocityConstraint(const std::string& jointName, float velocityThreshold) {
	if(m_anim == nullptr || m_anim->empty())
		throw std::runtime_error("Cannot detect constraints - animation data is empty.");

	// find the joint ID
	const std::size_t jointId = findJointId(jointName, *m_anim);

	std::map<std::string, Channel>::iterator cit = m_channels.insert(std::make_pair(jointName, Channel(this))).first;
	Channel& channel = cit->second;
	cit->second.clear();

	// extract the transforms in world space
	Frames& frames = channel.m_frames;
	for(auto& fr : *m_anim)
		frames.m_frames.push_back(constraints::Frame(fr[jointId].world(), 0.0f));

	// simple velocity thresholding, with averaging of the world space transform to derive the position of the constraint
	std::size_t begin = std::numeric_limits<std::size_t>::max();
	std::size_t end = 0;

	for(std::size_t frameIndex=0; frameIndex < frames.size(); ++frameIndex) {
		const float currentVelocity = velocity(frames.m_frames, frameIndex, m_anim->fps()).length();
		frames.m_frames[frameIndex].m_constraintValue = currentVelocity / velocityThreshold;

		if(currentVelocity < velocityThreshold) {
			if(begin > end)
				begin = frameIndex;
			end = frameIndex;
		}

		else {
			if(begin <= end) {
				channel.addConstraint(begin, end, average(frames.m_frames, begin, end));
				begin = std::numeric_limits<std::size_t>::max();
			}
		}
	}

	if(begin <= end)
		channel.addConstraint(begin, end, average(frames.m_frames, begin, end));
}

std::ostream& operator <<(std::ostream& out, const Constraints& c) {
	out << "Constraints:" << std::endl;

	if(c.empty())
		out << "  (empty)" << std::endl;

	for(auto& i : c) {
		out << "  " << i.first << " -";
		for(auto& ci : i.second)
			out << " (" << ci.startFrame() << " - " << ci.endFrame() << ")";
		out << std::endl;
	}

	return out;
}

}
