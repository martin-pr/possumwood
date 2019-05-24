#pragma once

#include "frames.h"
#include "constraint.h"

namespace anim {

class Constraints;

namespace constraints {

class Channel {
	public:
		typedef std::vector<Constraint>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		std::size_t size() const;

		const constraints::Frames& frames() const;

		bool operator == (const Channel& c) const;
		bool operator != (const Channel& c) const;

	private:
		Channel(::anim::Constraints* parent);

		void clear();

		/// Adds a new constraint to the channel. All constraints should be non-overlapping.
		void addConstraint(std::size_t startFrame, std::size_t endFrame, const anim::Transform& origin);

		std::vector<Constraint> m_values;
		Constraints* m_parent;

		constraints::Frames m_frames;

	friend class ::anim::Constraints;
};

} }
