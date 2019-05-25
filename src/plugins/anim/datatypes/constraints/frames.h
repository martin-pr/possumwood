#pragma once

#include <vector>

#include "frame.h"

namespace anim {

class Constraints;

namespace constraints {

class Channel;

/// Container for constraint frame data. Meant to be used as read-only.
class Frames {
	public:
		const Frame& operator[](std::size_t index) const;
		Frame& operator[](std::size_t index);

		typedef std::vector<Frame>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		typedef std::vector<Frame>::iterator iterator;
		iterator begin();
		iterator end();

		bool empty() const;
		std::size_t size() const;

	protected:
		Frames(const Frames& f) = default;
		Frames& operator = (const Frames&) = default;

	private:
		Frames();

		void clear();

		std::vector<Frame> m_frames;

	friend class ::anim::Constraints;
	friend class ::anim::constraints::Channel;
};


} }
