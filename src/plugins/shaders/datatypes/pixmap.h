#pragma once

#include <possumwood_sdk/traits.h>

#include <QPixmap>

namespace possumwood {

template<>
struct Traits<std::shared_ptr<const QPixmap>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 0}};
	}
};

}
