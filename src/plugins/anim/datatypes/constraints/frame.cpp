#include "frame.h"

namespace anim { namespace constraints {

Frame::Frame(const anim::Transform& tr, float value) : m_tr(tr), m_constraintValue(value) {
}

const anim::Transform& Frame::tr() const {
	return m_tr;
}

float Frame::value() const {
	return m_constraintValue;
}

void Frame::setValue(const float& value) {
	m_constraintValue = value;
}

} }
