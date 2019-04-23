#include "animation.h"

namespace anim {

Animation::Animation(float fps) : m_fps(fps) {
}

void Animation::addFrame(const Skeleton& f) {
	if(!m_frames.empty())
		assert(f.isCompatibleWith(m_frames.front()));

	m_frames.push_back(f);

	m_frames.back().makeConsistentWith(m_frames.front());
}

void Animation::setFrame(const Skeleton& f, std::size_t index) {
	assert(index < m_frames.size());
	assert(f.isCompatibleWith(m_frames.front()));

	m_frames[index] = f;

	if(m_frames.size() > 1) {
		if(index != 0)
			m_frames[index].makeConsistentWith(m_frames.front());
		else
			m_frames[index].makeConsistentWith(m_frames.back());
	}
}

const Skeleton& Animation::frame(std::size_t index) const {
	assert(index < m_frames.size());

	return m_frames[index];
}

bool Animation::empty() const {
	return m_frames.empty();
}

std::size_t Animation::size() const {
	return m_frames.size();
}

float Animation::fps() const {
	return m_fps;
}

void Animation::setFps(float fps) {
	m_fps = fps;
}

Animation::const_iterator Animation::begin() const {
	return m_frames.begin();
}

Animation::const_iterator Animation::end() const {
	return m_frames.end();
}

const Skeleton& Animation::front() const {
	assert(!m_frames.empty());
	return m_frames.front();
}
const Skeleton& Animation::back() const {
	assert(!m_frames.empty());
	return m_frames.back();
}

}
