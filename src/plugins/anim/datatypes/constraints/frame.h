#pragma once

#include "datatypes/transform.h"

namespace anim {

class Constraints;

namespace constraints {

class Channel;

/// A single frame of constraint data.
/// Transformation represents the position of the constraint (i.e., contact point);
/// value is the normalized constraint "strength", with 1.0 being the threshold value.
class Frame {
	public:
		const anim::Transform& tr() const;

		float value() const;
		void setValue(const float& value);

	private:
		Frame(const anim::Transform& tr, float value);

		anim::Transform m_tr;
		float m_constraintValue;

	friend class Channel;
	friend class ::anim::Constraints;
};

} }
