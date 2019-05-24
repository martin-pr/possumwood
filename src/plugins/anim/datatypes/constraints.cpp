#include "constraints.h"

#include "animation.h"

namespace anim {

Constraints::Constraints(const anim::Animation& a) : m_anim(new anim::Animation(a)) {
}

Constraints::Constraints(const Constraints& c) {
	for(auto& val : c) {
		std::pair<std::string, constraints::Channel> value = std::make_pair(std::string(val.first), constraints::Channel(this));

		std::map<std::string, constraints::Channel>::iterator it = m_channels.insert(value).first;
		it->second = val.second;
	}

	m_anim = c.m_anim;
}

const Constraints& Constraints::operator = (const Constraints& c) {
	m_channels.clear();

	for(auto& val : c) {
		std::map<std::string, constraints::Channel>::iterator it = m_channels.insert(std::make_pair(val.first, constraints::Channel(this))).first;
		it->second = val.second;
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

	std::map<std::string, constraints::Channel>::iterator cit = m_channels.insert(std::make_pair(jointName, constraints::Channel(this))).first;
	constraints::Channel& channel = cit->second;
	cit->second.clear();

	// extract the transforms in world space
	constraints::Frames& frames = channel.m_frames;
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
