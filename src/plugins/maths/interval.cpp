#include "interval.h"

#include <algorithm>

namespace possumwood { namespace maths {

namespace {

const float s_minLog = 1e-6f;

}

Interval::Interval() : m_min(0.0f), m_max(1.0f), m_value(0.0f), m_type(kLinear), m_inProgressUpdates(true) {
}

float Interval::value() const {
	return m_value;
}

void Interval::setValue(float val) {
	m_value = val;

	m_value = std::max(m_min, m_value);
	m_value = std::min(m_max, m_value);
}

Interval::operator float() const {
	return value();
}

Interval& Interval::operator = (float val) {
	setValue(val);
	return *this;
}

void Interval::setMin(float minval) {
	if(m_type == kLog)
		minval = std::max(minval, s_minLog);

	m_min = minval;
	m_max = std::max(m_max, minval);

	m_value = std::max(m_min, m_value);
	m_value = std::min(m_max, m_value);
}

float Interval::min() const {
	return m_min;
}

void Interval::setMax(float maxval) {
	if(m_type == kLog)
		maxval = std::max(maxval, s_minLog);

	m_max = maxval;
	m_min = std::min(m_min, maxval);

	m_value = std::max(m_min, m_value);
	m_value = std::min(m_max, m_value);
}

float Interval::max() const {
	return m_max;
}

void Interval::setType(Type t) {
	if(t == kLog) {
		m_min = std::max(m_min, s_minLog);
		m_max = std::max(m_max, s_minLog);
	}

	m_type = t;
}

Interval::Type Interval::type() const {
	return m_type;
}

void Interval::setInProgressUpdates(bool val) {
	m_inProgressUpdates = val;
}

bool Interval::inProgressUpdates() const {
	return m_inProgressUpdates;
}

std::ostream& operator << (std::ostream& out, const Interval& f) {
	out << "min=" << f.min() << ", max=" << f.max() << ", value=" << f.value();
	return out;
}

}

//////////

namespace {

void toJson(::possumwood::io::json& json, const maths::Interval& value) {
	json["min"] = value.min();
	json["max"] = value.max();
	json["in_progress_updates"] = value.inProgressUpdates();

	if(value.type() == maths::Interval::kLinear)
		json["type"] = "linear";
	else if(value.type() == maths::Interval::kLog)
		json["type"] = "log";
	else
		assert(false);

	json["value"] = value.value();
}

void fromJson(const ::possumwood::io::json& json, maths::Interval& value) {
	value.setMin(json["min"].get<float>());
	value.setMax(json["max"].get<float>());
	value.setInProgressUpdates(json["in_progress_updates"].get<bool>());

	const std::string type = json["type"].get<std::string>();
	if(type == "linear")
		value.setType(maths::Interval::kLinear);
	else if(type == "log")
		value.setType(maths::Interval::kLog);
	else
		throw std::runtime_error("Unknown interval type " + type);

	value.setValue(json["value"].get<float>());
}

}

IO<maths::Interval> Traits<maths::Interval>::io(&toJson, &fromJson);

}
