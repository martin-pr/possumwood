#pragma once

#include <anim/datatypes/transform.h>

namespace anim { namespace constraints {

class Constraint {
	public:
		const anim::Transform& origin() const;
		std::size_t startFrame() const;
		std::size_t endFrame() const;

		// typedef boost::iterator_range<std::vector<Frame>::const_iterator> FrameRange;
		// typedef FrameRange::const_iterator const_iterator;
		// const_iterator begin() const;
		// const_iterator end() const;

		bool operator == (const Constraint& c) const;
		bool operator != (const Constraint& c) const;

	private:
		Constraint(const anim::Transform& origin, std::size_t start, std::size_t end);

		anim::Transform m_origin;
		std::size_t m_startFrame, m_endFrame;

	friend class Channel;
};

}}
