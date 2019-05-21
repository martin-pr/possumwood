#pragma once

#include "datatypes/transform.h"

namespace anim {

class Channel;
class Constraints;

namespace constraints {

/// A single frame of constraint data.
/// Transformation represents the position of the constraint (i.e., contact point);
/// value is the normalized constraint "strength", with 1.0 being the threshold value.
class Frame {
	public:
		const anim::Transform& tr() const;
		float value() const;

	private:
		Frame(const anim::Transform& tr, float value);

		anim::Transform m_tr;
		float m_constraintValue;

	friend class ::anim::Channel;
	friend class ::anim::Constraints;
};

} }
