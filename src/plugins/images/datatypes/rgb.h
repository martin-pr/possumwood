#pragma once

#include <actions/traits.h>

#include <QColor>

namespace possumwood {

template<>
struct Traits<QColor> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0}};
	}
};

}

std::ostream& operator << (std::ostream& out, const QColor& col) {
	out << col.red() << " " << col.green() << " " << col.blue();

	return out;
}
