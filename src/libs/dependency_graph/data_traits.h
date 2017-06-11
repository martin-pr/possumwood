#pragma once

namespace dependency_graph {

template<typename T>
struct DataTraits {
	static void assignValue(T& dest, const T& src) {
		dest = src;
	}

	static bool isEqual(const T& v1, const T& v2) {
		return v1 == v2;
	}

	static bool isNotEqual(const T& v1, const T& v2) {
		return v1 != v2;
	}
};

}
