#include "metric.h"

namespace anim { namespace metric {

Base::~Base() {
}

LocalAngle::~LocalAngle() {
}

float LocalAngle::eval(const Animation& a1, std::size_t f1, const Animation& a2, std::size_t f2) const {
	const Skeleton& s1 = a1.frame(f1);
	const Skeleton& s2 = a2.frame(f2);

	if(s1.size() != s2.size() || s1.size() == 0)
		return 0.0f;

	float result = 0.0f;
	for(unsigned bi = 1; bi < s1.size(); ++bi) {
		const auto& b1 = s1[bi];
		const auto& b2 = s2[bi];

		if((b1.tr().rotation ^ b2.tr().rotation) > 0.0f)
			result += (b1.tr().rotation * b2.tr().rotation.inverse()).angle();
		else
			result += (b1.tr().rotation * (-b2.tr().rotation).inverse()).angle();
	}

	return result;
}

} }
