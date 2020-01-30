#pragma once

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood { namespace maths {

class Interval {
	public:
		enum Type {
			kLinear = 0,
			kLog = 1
		};

		Interval();

		float value() const;
		void setValue(float val);

		operator float() const;
		Interval& operator = (float val);

		void setMin(float min);
		float min() const;

		void setMax(float max);
		float max() const;

		void setType(Type t);
		Type type() const;

		void setInProgressUpdates(bool val);
		bool inProgressUpdates() const;

	private:
		float m_min, m_max, m_value;
		Type m_type;
		bool m_inProgressUpdates;
};

std::ostream& operator << (std::ostream& out, const Interval& f);

}

template<>
struct Traits<maths::Interval> {
	static IO<maths::Interval> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 1}};
	}
};

}


