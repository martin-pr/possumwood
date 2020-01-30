#pragma once

#include <type_traits>

namespace dependency_graph {

/// For types that do NOT have equality operator - always assume two instances are NOT the same
template<typename T, typename ENABLE = void>
struct DataTraits {
	static bool isEqual(const T& v1, const T& v2) {
		return false;
	}

	static bool isNotEqual(const T& v1, const T& v2) {
		return true;
	}
};

/// For types that do have equality operator
template<typename T>
struct DataTraits<T,
	typename std::enable_if<
		std::is_convertible<
			decltype(std::declval<T>()==std::declval<T>()), bool
		>::value
	>::type
 > {
	static bool isEqual(const T& v1, const T& v2) {
		return v1 == v2;
	}

	static bool isNotEqual(const T& v1, const T& v2) {
		return v1 != v2;
	}
};

}
