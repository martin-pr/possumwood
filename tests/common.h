#pragma once

#include <iostream>
#include <typeinfo>

namespace std {

/// printing type_info, to allow its use in BOOST_*_EQUAL macro
inline std::ostream& operator << (std::ostream& out, const std::type_info& t) {
	out << t.name();

	return out;
}

}
